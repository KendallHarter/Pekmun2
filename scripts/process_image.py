# Takes in an image from the command line and outputs its representation
# as a header

import cv2
import sys

if len(sys.argv) != 4:
   sys.exit(f'Usage: {sys.argv[0]} input_file output_file variable_name')

def to_cpp_array(a_list: list[int]) -> str:
   return str(a_list).replace("[", "{").replace("]", "};")

def make_gba_color(r, g, b):
   # Converts from from 0-255 range to GBA range
   def conv(val):
      # Apparently only 15-31 are really visible
      # So use 14 as 0 and go from there
      # Mess around with this eventually for better colors
      low_val = 0
      return int(low_val + round((31 - low_val) * val / 255))

   # Either the documentation is wrong (doubtful), or there's some weird endian issue
   # This seems to work, though
   # Swapping the bytes around doesn't seem to fix it so ehhhh
   return (conv(r) << 10) + (conv(g) << 5) + (conv(b) << 0)

def process_image(image) -> (list[int], list[int]):
   image_vals = []
   image_pal = []
   pal_map = {}

   # Process in blocks of 8x8
   assert len(image) % 8 == 0
   for base_y in range(len(image) // 8):
      assert len(image[0]) % 8 == 0
      for base_x in range(len(image[0]) // 8):
         for y in range(8):
            add_value = False
            prev_value = 0
            for x in range(8):
               r, g, b = image[base_y * 8 + y][base_x * 8 + x]
               if (r, g, b) not in pal_map:
                  pal_num = len(image_pal)
                  pal_map[(r, g, b)] = pal_num
                  image_pal.append(make_gba_color(r, g,  b))
               if add_value:
                  image_vals.append((pal_map[(r, g, b)] << 4) + prev_value)
                  add_value = False
               else:
                  add_value = True
                  prev_value = pal_map[(r, g, b)]

   bit32_vals = []
   for a, b, c, d in zip(*([iter(image_vals)] * 4)):
      bit32_vals.append(int(d << 24 | c << 16 | b << 8 | a))

   if len(image_pal) > 16:
      sys.exit(f'Image {sys.argv[1]} has too many colors ({len(image_pal)}/16)')

   return bit32_vals, image_pal


with open(sys.argv[2], 'w') as f:
   vals, pal = process_image(cv2.imread(sys.argv[1]))
   header_guard = sys.argv[3].upper()
   name = sys.argv[3]
   f.write(
      f'#ifndef {header_guard}_IMAGE_DATA\n'
      f'#define {header_guard}_IMAGE_DATA\n'
      f'#include <cstdint>\n'
      f'inline constexpr std::uint32_t {name}[]{to_cpp_array(vals)}\n'
      f'inline constexpr std::uint16_t {name}_pal[]{to_cpp_array(pal)}\n'
      f'#endif\n'
   )
