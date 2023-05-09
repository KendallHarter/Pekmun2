import json
import sys
import os
import itertools

root_dir = os.path.dirname(os.path.abspath(__file__))

if len(sys.argv) != 4:
   sys.exit(f'Usage: {sys.argv[0]} map_name output_file name')

map_name = sys.argv[1]
output_file = sys.argv[2]
name = sys.argv[3]

def flatten(lst):
   return list(itertools.chain.from_iterable(lst))

def to_cpp_array(a_list: list[int | bool]) -> str:
   return str(a_list).replace("[", "{").replace("]", "};").replace("True", "true").replace("False", "false")

with open(os.path.join(root_dir, f'../assets/maps/{map_name}/tiles.json')) as f:
   tiles = json.load(f)

with open(os.path.join(root_dir, f'../assets/maps/{map_name}/info.json')) as f:
   info = json.load(f)

low_priority_tiles = [x - 1 for x in tiles['layers'][0]['data']]
high_priority_tiles = [x - 1 for x in tiles['layers'][1]['data']]
heights = flatten(info['heights'])
walkable = flatten(info['walkable'])
sprite_priority = flatten(info['sprite_priority'])
tile_priority = flatten(info['tile_priority'])

with open(output_file, 'w') as f:
   header_guard = name.upper()
   f.write(
      f'#ifndef {header_guard}_MAP_DATA\n'
      f'#define {header_guard}_MAP_DATA\n'
      f'#include "map_data.hpp"\n'
      f'#include <array>\n'
      f'#include <cstdint>\n'
      # TODO: The 2400 is hardcoded because we need std::array
      #       There's probably a better way to do this
      f'inline constexpr std::array<std::uint16_t, 2400> {name}_high_priority_tiles{to_cpp_array(high_priority_tiles)}\n'
      f'inline constexpr std::array<std::uint16_t, 2400> {name}_low_priority_tiles{to_cpp_array(low_priority_tiles)}\n'
      f'inline constexpr std::uint8_t {name}_walkable_map[]{to_cpp_array(walkable)}\n'
      f'inline constexpr std::uint8_t {name}_height_map[]{to_cpp_array(heights)}\n'
      f'inline constexpr bool {name}_sprite_high_priority[]{to_cpp_array(sprite_priority)}\n'
      f'inline constexpr bool {name}_tile_high_priority[]{to_cpp_array(tile_priority)}\n'
      f'inline constexpr map_data {name}{{'
         f'{info["width"]}, {info["height"]}, {info["y_offset"]}, '
         f'{name}_high_priority_tiles, {name}_low_priority_tiles, {name}_walkable_map, {name}_height_map, '
         f'{name}_sprite_high_priority, {name}_tile_high_priority'
         f'}};\n'
      f'#endif\n'
   )
