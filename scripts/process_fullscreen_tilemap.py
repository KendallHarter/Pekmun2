# Processes a full screen tilemap (makes sure it is exactly 30x20 tiles)

import json
import sys

if len(sys.argv) != 4:
   sys.exit(f'Usage: {sys.argv[0]} input_file output_file variable_name')

def to_cpp_array(a_list: list[int]) -> str:
   return str(a_list).replace("[", "{").replace("]", "};")

with open(sys.argv[1]) as f:
   data = json.load(f)

tiles = data['layers'][0]['data']
width = data['layers'][0]['width']
height = data['layers'][0]['height']

assert width == 30
assert height == 20

adj_tiles = [x - 1 for x in tiles]

with open(sys.argv[2], 'w') as f:
   header_guard = sys.argv[3].upper()
   name = sys.argv[3]
   f.write(
      f'#ifndef {header_guard}_TILEMAP_DATA\n'
      f'#define {header_guard}_TILEMAP_DATA\n'
      f'#include <cstdint>\n'
      f'inline constexpr std::uint16_t {name}[]{to_cpp_array(adj_tiles)}\n'
      f'#endif\n'
   )
