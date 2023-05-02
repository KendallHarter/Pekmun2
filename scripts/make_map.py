# Converts from a height/walkable map to a complete map

import json
import sys
import os

# TODO: Is there a good way to have this be a configuration somewhere rather
#       than a hard coded thing in two places?
# Max input width; this is so map data can always be padded to the same length
# for quick multiplication
MAX_INPUT_WIDTH = 16

class Map:
   def __init__(self, input_file):
      with open(os.path.join('../assets/raw_maps/', input_file)) as f:
         info = json.load(f)

      self.HEIGHT_INPUT = info['heights']
      self.WALKABLE_INPUT = info['walkable']
      self.INPUT_WIDTH = len(self.HEIGHT_INPUT[0])
      self.INPUT_HEIGHT = len(self.HEIGHT_INPUT)

      assert self.INPUT_WIDTH <= MAX_INPUT_WIDTH
      assert len(self.HEIGHT_INPUT) == len(self.WALKABLE_INPUT)
      assert len(self.HEIGHT_INPUT[0]) == len(self.WALKABLE_INPUT[0])

   # This priority is for the tiles themselves, the sprite priority map
   # is somewhat different
   # A tile needs to be on a higher priority layer if:
   #     A tile of lower height is to the upper-right [x + 1]
   #     A tile of higher priority and height is to the upper-right [x + 1] (maybe?)
   #     A tile of lower height is to the upper-left [y - 1]
   def tile_is_high_priority(self, x, y):
      if x < 0 or x >= self.INPUT_WIDTH or y < 0 or y >= self.INPUT_HEIGHT:
         return False
      if x < self.INPUT_WIDTH - 1:
         if self.HEIGHT_INPUT[y][x + 1] < self.HEIGHT_INPUT[y][x]:
            return True
         # elif HEIGHT_INPUT[y][x + 1] > HEIGHT_INPUT[y][x] and tile_is_high_priority(x + 1, y):
         #    return True
      if y > 0 and self.HEIGHT_INPUT[y - 1][x] < self.HEIGHT_INPUT[y][x]:
         return True
      return False


   # A tile is high sprite priority if it is tile high priority or:
   #     A tile to the upper-left is tile high priority [y - 1]
   #     A tile to the upper-right is tile high priority [x + 1]
   #     A tile above one is tile high priority [x + 1, y - 1]
   #     A tile one to the upper-left and two to the upper-right is tile high priority [x + 2, y - 1]
   #     A tile two to the upper-left and one to the upper-right is tile high priority [x + 1, y - 2]
   def tile_is_sprite_priority(self, x, y):
      if self.tile_is_high_priority(x, y):
         return True
      elif self.tile_is_high_priority(x, y - 1):
         return True
      elif self.tile_is_high_priority(x + 1, y):
         return True
      elif self.tile_is_high_priority(x + 2, y - 1):
         return True
      elif self.tile_is_high_priority(x + 1, y - 1):
         return True
      elif self.tile_is_high_priority(x + 1, y - 2):
         return True
      return False

def main():
   if len(sys.argv) != 2:
      sys.exit(f'Usage: {sys.argv[0]} input_file*\n\t*Relative to ../assets/raw_maps')

   # Move to the directory the script is in so it can be run anywhere
   os.chdir(os.path.dirname(os.path.abspath(__file__)))

   input_file = sys.argv[1]
   output_dir = os.path.splitext(os.path.basename(sys.argv[1]))[0]

   the_map = Map(input_file)

   BLANK_TILE = 18
   # TODO: Same as above; these are also hard-coded limits set in two different places
   MAP_WIDTH = 60
   MAP_HEIGHT = 40
   low_priority = [BLANK_TILE] * MAP_HEIGHT * MAP_WIDTH
   high_priority = [BLANK_TILE] * MAP_HEIGHT * MAP_WIDTH

   # Find the lowest height so the y-offset is known
   # The lowest x-offset is always 0
   min_y = float('inf')
   max_y = float('-inf')
   for y in range(the_map.INPUT_HEIGHT):
      for x in range(the_map.INPUT_WIDTH):
         tile_y = y - x - the_map.HEIGHT_INPUT[y][x]
         min_y = min(min_y, tile_y)
         max_y = max(max_y, tile_y)

   y_offset = -min_y
   print(f'size: {the_map.INPUT_WIDTH}x{the_map.INPUT_HEIGHT}')
   print(f'y offset: {y_offset}')
   max_x = (the_map.INPUT_WIDTH - 1) * 2 + (the_map.INPUT_HEIGHT - 1) * 2
   if max_x >= MAP_WIDTH:
      sys.exit(f'Max x {max_x} exceeds map size')
   elif max_y >= MAP_HEIGHT:
      sys.exit(f'Max y {max_y} exceeds map size')
   for y in range(the_map.INPUT_HEIGHT):
      for x in range(the_map.INPUT_WIDTH):
         if the_map.WALKABLE_INPUT[y][x]:
            adjust_layer = low_priority
            if the_map.tile_is_high_priority(x, y):
               adjust_layer = high_priority
            tile_x = x * 2 + y * 2
            tile_y = y_offset + y - x - the_map.HEIGHT_INPUT[y][x]
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
            # write the columns (only on the low priority layer)
            if adjust_layer is low_priority:
               for i in range(the_map.HEIGHT_INPUT[y][x] - 1):
                  if adjust_layer[tile_x + (tile_y + i + 2) * MAP_WIDTH] == BLANK_TILE:
                     adjust_layer[tile_x + (tile_y + i + 2) * MAP_WIDTH] = 19
                     adjust_layer[tile_x + (tile_y + i + 2) * MAP_WIDTH + 1] = 19
                     adjust_layer[tile_x + (tile_y + i + 2) * MAP_WIDTH + 2] = 19
                     adjust_layer[tile_x + (tile_y + i + 2) * MAP_WIDTH + 3] = 19
            # Write the bottom on corner tiles
            y_off = the_map.HEIGHT_INPUT[y][x] - 1 + 2
            if x == 0:
               if low_priority[tile_x + (tile_y + y_off) * MAP_WIDTH] == BLANK_TILE:
                  low_priority[tile_x + (tile_y + y_off) * MAP_WIDTH] = 14
                  low_priority[tile_x + (tile_y + y_off) * MAP_WIDTH + 1] = 15
            if y == the_map.INPUT_HEIGHT - 1:
               if low_priority[tile_x + (tile_y + y_off) * MAP_WIDTH + 2] == BLANK_TILE:
                  low_priority[tile_x + (tile_y + y_off) * MAP_WIDTH + 2] = 16
                  low_priority[tile_x + (tile_y + y_off) * MAP_WIDTH + 3] = 17

   # Emulate the Tiled file format
   # It discards extra fields when edited so a separate file will be needed for the
   # height / priority maps
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
         'image': '../../test_tileset.png',
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

   def try_get(lst, x, y):
      try:
         return lst[y][x]
      except IndexError:
         return 0

   sprite_priority = [[the_map.tile_is_sprite_priority(x, y) for x in range(MAX_INPUT_WIDTH)] for y in range(the_map.INPUT_HEIGHT)]
   tile_priority = [[the_map.tile_is_high_priority(x, y) for x in range(MAX_INPUT_WIDTH)] for y in range(the_map.INPUT_HEIGHT)]
   adjusted_heights = [[try_get(the_map.HEIGHT_INPUT, x, y) for x in range(MAX_INPUT_WIDTH)] for y in range(the_map.INPUT_HEIGHT)]
   adjusted_walkable = [[try_get(the_map.WALKABLE_INPUT, x, y) for x in range(MAX_INPUT_WIDTH)] for y in range(the_map.INPUT_HEIGHT)]

   output_dir = os.path.join('../assets/maps/', output_dir)
   os.makedirs(output_dir, exist_ok=True)
   with open(os.path.join(output_dir, 'tiles.json'), 'w') as f:
      f.write(json.dumps(output))
   with open(os.path.join(output_dir, 'info.json'), 'w') as f:
      f.write(json.dumps({
         'sprite_priority': sprite_priority,
         'tile_priority': tile_priority,
         'height': the_map.INPUT_HEIGHT,
         'width': the_map.INPUT_WIDTH,
         'y_offset': y_offset,
         'heights': adjusted_heights,
         'walkable': adjusted_walkable
      }))

if __name__ == "__main__":
   main()
