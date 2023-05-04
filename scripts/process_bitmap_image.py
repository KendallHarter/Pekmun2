import cv2
import sys

if len(sys.argv) != 4:
   sys.exit(f'Usage: {sys.argv[0]} input_file output_file variable_name')

def to_cpp_array(a_list: list[int]) -> str:
   return str(a_list).replace("[", "{").replace("]", "};")

def make_gba_color(r, g, b):
   def conv(val):
      low_val = 0
      return int(low_val + round((31 - low_val) * val / 255))

   return (conv(r) << 10) + (conv(g) << 5) + (conv(b) << 0)

values = []
for row in cv2.imread(sys.argv[1]):
   for r, g, b in row:
      values.append(make_gba_color(r, g, b))

with open(sys.argv[2], 'w') as f:
   header_guard = sys.argv[3].upper()
   name = sys.argv[3]
   f.write(
      f'#ifndef {header_guard}_IMAGE_DATA\n'
      f'#define {header_guard}_IMAGE_DATA\n'
      f'#include <cstdint>\n'
      f'inline constexpr std::uint16_t {name}[]{to_cpp_array(values)}\n'
      f'#endif\n'
   )
