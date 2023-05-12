import cv2
import sys
import glob
import os

if len(sys.argv) != 3:
   sys.exit(f'{sys.argv[0]} input_directory output_file')

def to_cpp_array(a_list: list[int]) -> str:
   return str(a_list).replace("[", "{").replace("]", "};")

def make_gba_color(r, g, b):
   def conv(val):
      low_val = 0
      return int(low_val + round((31 - low_val) * val / 255))

   return (conv(r) << 10) + (conv(g) << 5) + (conv(b) << 0)

input_dir = sys.argv[1]

pal_path = f'{input_dir}/palette.png'
pal_raw = cv2.imread(pal_path)
assert len(pal_raw) == 1
assert len(pal_raw[0]) == 16

pal = []
for row in pal_raw:
   for i, (r, g, b) in enumerate(row):
      pal.append((r, g, b))

images = {}

for image in glob.glob(f'{input_dir}/*.png'):
   if image == pal_path:
      continue
   file_name = os.path.basename(image)
   images[file_name] = []
   image_data = cv2.imread(image)
   raw_vals = []
   assert len(image_data) % 8 == 0
   for base_y in range(len(image_data) // 8):
      assert len(row) % 8 == 0
      for base_x in range(len(image_data[0]) // 8):
         for y in range(8):
            add_value = False
            prev_value = 0
            for x in range(8):
               r, g, b = image_data[base_y * 8 + y][base_x * 8 + x]
               try:
                  index = pal.index((r, g, b))
                  if add_value:
                     raw_vals.append((index << 4) | prev_value)
                     add_value = False
                  else:
                     add_value = True
                     prev_value = index
               except:
                  sys.exit(f'Color {(r, g, b)} at {(base_x * 8 + x, base_y * 8 + y)} in {image} not found in palette')

   # Compact into 32-bit integers
   for a, b, c, d in zip(*([iter(raw_vals)] * 4)):
      images[file_name].append(int(d << 24 | c << 16 | b << 8 | a))

with open(sys.argv[2], 'w') as f:
   base_name = os.path.basename(input_dir)
   header_guard = base_name.upper()
   gba_pal = [make_gba_color(r, g, b) for r, g, b in pal]
   f.write(
      f'#ifndef {header_guard}_PALETTE_DATA\n'
      f'#define {header_guard}_PALETTE_DATA\n'
      f'#include <cstdint>\n'
      f'namespace {base_name} {{\n'
      f'inline constexpr std::uint16_t palette[]{to_cpp_array(gba_pal)}\n'
   )
   for raw_name, data in images.items():
      name = os.path.splitext(raw_name)[0]
      f.write(f'inline constexpr std::uint32_t {name}[]{to_cpp_array(data)}\n')
   f.write(
      f'}}\n'
      f'#endif\n'
   )
