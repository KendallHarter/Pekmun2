# Converts from a height/walkable map to a complete map

import json

# temporary input
HEIGHT_INPUT = [
   [3, 1, 1, 5, 7],
   [1, 1, 2, 4, 7],
   [1, 1, 2, 3, 7],
   [1, 1, 1, 2, 7],
   [1, 1, 1, 1, 7]
]

WALKABLE_INPUT = [
   [1, 1, 1, 1, 1],
   [1, 0, 1, 1, 1],
   [1, 0, 1, 1, 0],
   [1, 0, 1, 1, 1],
   [1, 1, 1, 1, 1]
]

INPUT_WIDTH = len(HEIGHT_INPUT[0])
INPUT_HEIGHT = len(HEIGHT_INPUT)

def clamp(n, smallest, largest):
   return max(smallest, min(n, largest))


# This priority is for the tiles themselves, the sprite priority map
# is somewhat different
# TODO: Determine if the above is actually true
# A tile needs to be on a higher priority layer if:
#     A tile of lower height is to the upper-right [x + 1]
#     A tile of higher priority and height is to the upper-right [x + 1]
#     A tile of lower height is to the upper-left [y - 1]
def tile_is_high_priority(x, y):
   if x < INPUT_WIDTH - 1:
      if HEIGHT_INPUT[y][x + 1] < HEIGHT_INPUT[y][x] or HEIGHT_INPUT[y][x + 1] > HEIGHT_INPUT[y][x] and tile_is_high_priority(x + 1, y):
         return True
   if y > 0 and HEIGHT_INPUT[y - 1][x] < HEIGHT_INPUT[y][x]:
      return True
   return False


def main():
   BLANK_TILE = 18
   MAP_WIDTH = 30
   MAP_HEIGHT = 20
   low_priority = [BLANK_TILE] * MAP_HEIGHT * MAP_WIDTH
   high_priority = [BLANK_TILE] * MAP_HEIGHT * MAP_WIDTH

   # Find the lowest height the y-offset is known
   # The lowest x-offset is always 0
   min_y = float('inf')
   for y in range(INPUT_HEIGHT):
      for x in range(INPUT_WIDTH):
         tile_y = y - x - HEIGHT_INPUT[y][x]
         min_y = min(min_y, tile_y)

   y_offset = -min_y
   print(f'y offset: {y_offset}')
   for y in range(INPUT_HEIGHT):
      for x in range(INPUT_WIDTH):
         if WALKABLE_INPUT[y][x]:
            adjust_layer = low_priority
            if tile_is_high_priority(x, y):
               adjust_layer = high_priority
            tile_x = x * 2 + y * 2
            tile_y = y_offset + y - x - HEIGHT_INPUT[y][x]
            # Draw the tile
            for i in range(4):
               # If there's already something there, write a tile that
               # includes both the upper-left and lower-right
               if adjust_layer[tile_x + tile_y * MAP_WIDTH + i] == BLANK_TILE:
                  adjust_layer[tile_x + tile_y * MAP_WIDTH + i] = i + 1
               else:
                  adjust_layer[tile_x + tile_y * MAP_WIDTH + i] = i + 9
            for i in range(4):
               adjust_layer[tile_x + (tile_y + 1) * MAP_WIDTH + i] = i + 22

   # Emulate the Tiled file format
   output = {
      'compressionlevel': -1,
      'height': MAP_HEIGHT,
      'infinite': False,
      'nextlayerid': 3,
      'nextobjectid': 1,
      'orientation': 'orthogonal',
      'renderorder': 'right-down',
      'tiledversion': '1.10.1',
      'tileheight': 8,
      'tilewidth': 8,
      'type': 'map',
      'version': '1.10',
      'width': MAP_WIDTH,
      'tilesets': [{
         'columns': 28,
         'firstgid': 1,
         'image': 'test_tileset.png',
         'imageheight': 8,
         'imagewidth': 224,
         'margin': 0,
         'name': 'test',
         'spacing': 0,
         'tilecount': 28,
         'tileheight': 8,
         'tilewidth': 8,
         'transparentcolor': '#2b44ff'
      }],
      'layers': [{
         'data': low_priority,
         'height': MAP_HEIGHT,
         'id': 1,
         'name': 'low priority',
         'opacity': 1,
         'type': 'tilelayer',
         'visible': True,
         'width': MAP_WIDTH,
         'x': 0,
         'y': 0
      },
      {
         'data': high_priority,
         'height': MAP_HEIGHT,
         'id': 2,
         'name': 'high priority',
         'opacity': 1,
         'type': 'tilelayer',
         'visible': True,
         'width': MAP_WIDTH,
         'x': 0,
         'y': 0
      }]
   }

   # print(to_cpp_array(low_priority))
   with open('../assets/test_output.json', 'w') as f:
      f.write(json.dumps(output))


def to_cpp_array(a_list: list[int]) -> str:
   return str(a_list).replace("[", "{").replace("]", "};")


if __name__ == "__main__":
   main()
