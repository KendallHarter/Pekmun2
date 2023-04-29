import pygame
import sys
import json
import os

def clamp(n, smallest, largest):
   return max(smallest, min(n, largest))

def main():
   # Move to the directory the script is in so it can be run anywhere
   os.chdir(os.path.dirname(os.path.abspath(__file__)))

   pygame.init()
   SCREEN_WIDTH = 240
   SCREEN_HEIGHT = 160
   screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT), pygame.SCALED)

   tileset = pygame.image.load('../assets/test_tileset.png')
   tileset.set_colorkey(tileset.get_at((0, 0)))

   snake = pygame.image.load('../assets/snake.png')
   snake.set_colorkey(snake.get_at((0, 0)))

   cursor = pygame.image.load('../assets/cursor.png')
   cursor.set_colorkey(cursor.get_at((0, 0)))

   move_indic = pygame.image.load('../assets/py_move_indicator.png')
   move_indic.set_colorkey(move_indic.get_at((0, 0)))

   test_square = pygame.image.load('../assets/py_test_sprite.png')

   my_font = pygame.font.SysFont('Consolas', 12)

   y_adj = -24
   if len(sys.argv) == 1:
      with open('../assets/test_map2.json') as f:
         map_info = json.load(f)

      y_offset = 11

      sprite_priority = [
         [True,  False, False, False, False],
         [True,  True,  True,  False, False],
         [True,  True,  True,  False, False],
         [True,  True, False, False, False],
         [False, False, False, False, False]
      ]

      tile_priority = [
         [True,  False, False, False, False],
         [True,  True,  True,  False, False],
         [True,  True,  True,  False, False],
         [True,  True, False, False, False],
         [False, False, False, False, False]
      ]

      map_height_data = [
         [3, 1, 1, 5, 7],
         [1, 1, 2, 4, 7],
         [1, 1, 2, 3, 7],
         [1, 1, 1, 2, 7],
         [1, 1, 1, 1, 7]
      ]

      map_walkable = [
         [1, 1, 1, 1, 1],
         [1, 0, 1, 1, 1],
         [1, 0, 1, 1, 0],
         [1, 0, 1, 1, 1],
         [1, 1, 1, 1, 1]
      ]

      data_height = len(map_height_data)
      data_width = len(map_height_data[0])
   else:
      with open(f'../assets/maps/{sys.argv[1]}/tiles.json') as f:
         map_info = json.load(f)

      with open(f'../assets/maps/{sys.argv[1]}/info.json') as f:
         info = json.load(f)

      sprite_priority = info['sprite_priority']
      tile_priority = info['tile_priority']
      y_offset = info['y_offset']
      data_width = info['width']
      data_height = info['height']

      with open(f'../assets/raw_maps/{sys.argv[1]}.json') as f:
         info = json.load(f)

      map_height_data = info['heights']
      map_walkable = info['walkable']

   map_width = map_info['width']
   map_height = map_info['height']

   snake_x = 0
   snake_y = 0
   cursor_x = 0
   cursor_y = 0
   selected = False
   move_layer_0 = []
   move_layer_1 = []
   sprite_layer_0 = []
   sprite_layer_1 = []
   camera_x = 0
   camera_y = 0

   def draw_tile_layer(layer_number):
      for y in range(map_height):
         for x in range(map_width):
            loc = x + y * map_width
            tile = map_info['layers'][layer_number]['data'][loc]
            if tile != 0:
               screen.blit(tileset, (x * 8 - camera_x, y * 8 - camera_y), (((tile - 1) * 8, 0), (8, 8)))

   def draw_snake():
      screen.blit(snake, (8 + snake_x * 16 + snake_y * 16 - camera_x, y_adj + y_offset * 8 + snake_y * 8 - 8 * snake_x - 8 * map_height_data[snake_y][snake_x] - camera_y))

   def draw_cursor():
      screen.blit(cursor, (cursor_x * 16 + cursor_y * 16 - camera_x, y_adj + y_offset * 8 + 7 + cursor_y * 8 - 8 * cursor_x - 8 * map_height_data[cursor_y][cursor_x] - camera_y))

   def draw_test_square(x, y):
      screen.blit(test_square, (8 + x * 16 + y * 16 - camera_x, y_adj + y_offset * 8 + y * 8 - 8 * x - 8 * map_height_data[y][x] - camera_y))

   while True:
      for event in pygame.event.get():
         if event.type == pygame.QUIT:
            sys.exit()

      keys = pygame.key.get_pressed()
      if keys[pygame.K_LEFT]:
         cursor_x -= 1
      elif keys[pygame.K_RIGHT]:
         cursor_x += 1
      if keys[pygame.K_UP]:
         cursor_y -= 1
      elif keys[pygame.K_DOWN]:
         cursor_y += 1

      # if keys[pygame.K_w]:
      #    camera_y -= 4
      # elif keys[pygame.K_s]:
      #    camera_y += 4
      # if keys[pygame.K_a]:
      #    camera_x -= 4
      # elif keys[pygame.K_d]:
      #    camera_x += 4

      cursor_x = clamp(cursor_x, 0, data_width - 1)
      cursor_y = clamp(cursor_y, 0, data_height - 1)

      camera_x = 32 + -SCREEN_WIDTH / 2 + cursor_x * 16 + cursor_y * 16
      camera_y = 32 + -SCREEN_HEIGHT / 2 + y_adj + y_offset * 8 + cursor_y * 8 - 8 * cursor_x - 8 * map_height_data[cursor_y][cursor_x]

      if keys[pygame.K_z]:
         if not selected and cursor_x == snake_x and cursor_y == snake_y:
            for y in range(data_height):
               for x in range(data_width):
                  if map_walkable[y][x]:
                     if tile_priority[y][x] == 0:
                        move_layer_0.append((x, y))
                     else:
                        move_layer_1.append((x, y))
            selected = True
         elif selected:
            if (cursor_x, cursor_y) in move_layer_0 or (cursor_x, cursor_y) in move_layer_1:
               snake_x = cursor_x
               snake_y = cursor_y
               move_layer_0 = []
               move_layer_1 = []
               selected = False

      if keys[pygame.K_x]:
         if sprite_priority[cursor_y][cursor_x] == 0:
            # If there's a sprite of higher priority above we need to put this in higher priority instead
            add_to = sprite_layer_0
            if (cursor_x + 1, cursor_y - 1) in sprite_layer_1:
               add_to = sprite_layer_1
            if (cursor_x, cursor_y) not in add_to:
               add_to.append((cursor_x, cursor_y))
               add_to.sort(key=lambda x: x[1])
         else:
            # If there's a sprite of lower priority in the square below we need to raise its priority
            try:
               index = sprite_layer_0.index((cursor_x - 1, cursor_y + 1))
               del sprite_layer_0[index]
               sprite_layer_1.append((cursor_x - 1, cursor_y + 1))
            except ValueError:
               pass
            if (cursor_x, cursor_y) not in sprite_layer_1:
               sprite_layer_1.append((cursor_x, cursor_y))
               sprite_layer_1.sort(key=lambda x: x[1])

      snake_priority = sprite_priority[snake_y][snake_x]
      cursor_priority = sprite_priority[cursor_y][cursor_x]

      screen.fill((0, 0xff, 0))
      draw_tile_layer(0)
      for x, y in move_layer_0:
         screen.blit(move_indic, (x * 16 + y * 16 - camera_x, y_adj + y_offset * 8 + 24 + 8 * y - 8 * x - 8 * map_height_data[y][x] - camera_y))
      if cursor_priority == 0:
         draw_cursor()
      for x, y in sprite_layer_0:
         draw_test_square(x, y)
      if snake_priority == 0:
         draw_snake()

      draw_tile_layer(1)
      for x, y in move_layer_1:
         screen.blit(move_indic, (x * 16 + y * 16 - camera_x, y_adj + y_offset * 8 + 24 + 8 * y - 8 * x - 8 * map_height_data[y][x] - camera_y))
      if cursor_priority == 1:
         draw_cursor()
      for x, y in sprite_layer_1:
         draw_test_square(x, y)
      if snake_priority == 1:
         draw_snake()

      screen.blit(my_font.render(f'{sprite_priority[cursor_y][cursor_x]}', False, (0, 0, 0)), (0, 0))
      screen.blit(my_font.render(f'{cursor_x}, {cursor_y}', False, (0, 0, 0)), (0, 12))
      screen.blit(my_font.render(f'camera: {camera_x}, {camera_y}', False, (0, 0, 0)), (0, 24))

      pygame.display.flip()
      pygame.time.wait(50)


if __name__ == '__main__':
   main()
