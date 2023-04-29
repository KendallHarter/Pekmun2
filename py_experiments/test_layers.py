import pygame
import sys
import json

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

my_font = pygame.font.SysFont('Consolas', 12)

with open('../assets/test_map2.json') as f:
   map_info = json.load(f)

map_width = map_info['width']
map_height = map_info['height']

def draw_tile_layer(layer_number):
   for y in range(map_height):
      for x in range(map_width):
         loc = x + y * map_width
         tile = map_info['layers'][layer_number]['data'][loc]
         if tile != 0:
            screen.blit(tileset, (x * 8, y * 8), (((tile - 1) * 8, 0), (8, 8)))

# I don't know why but the Y offset needs to be somewhat adjusted at times
y_adj = -2
y_offset = 11

map_layer_data = [
   [True,  False, False, False, False],
   [True,  True,  True,  False, False],
   [True,  True,  True,  False, False],
   [True,  False, False, False, False],
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

def clamp(n, smallest, largest):
   return max(smallest, min(n, largest))

def main():
   snake_x = 0
   snake_y = 0
   cursor_x = 0
   cursor_y = 0
   selected = False
   move_layer_0 = []
   move_layer_1 = []

   def draw_snake():
      screen.blit(snake, (8 + snake_x * 16 + snake_y * 16, y_adj + y_offset * 6 + snake_y * 8 - 8 * snake_x - 8 * map_height_data[snake_y][snake_x]))

   def draw_cursor():
      screen.blit(cursor, (cursor_x * 16 + cursor_y * 16, y_adj + y_offset * 6 + 7 + cursor_y * 8 - 8 * cursor_x - 8 * map_height_data[cursor_y][cursor_x]))

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

      cursor_x = clamp(cursor_x, 0, data_width - 1)
      cursor_y = clamp(cursor_y, 0, data_height - 1)

      if keys[pygame.K_z]:
         if not selected and cursor_x == snake_x and cursor_y == snake_y:
            for y in range(data_height):
               for x in range(data_width):
                  if map_walkable[y][x]:
                     if map_layer_data[y][x] == 0:
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

      snake_priority = map_layer_data[snake_y][snake_x]
      cursor_priority = map_layer_data[cursor_y][cursor_x]

      screen.fill((0, 0xff, 0))
      draw_tile_layer(0)
      for x, y in move_layer_0:
         screen.blit(move_indic, (x * 16 + y * 16, y_adj + y_offset * 6 + 24 + 8 * y - 8 * x - 8 * map_height_data[y][x]))
      if cursor_priority == 0:
         draw_cursor()
      if snake_priority == 0:
         draw_snake()

      draw_tile_layer(1)
      for x, y in move_layer_1:
         screen.blit(move_indic, (x * 16 + y * 16, y_adj + y_offset * 6 + 24 + 8 * y - 8 * x - 8 * map_height_data[y][x]))
      if cursor_priority == 1:
         draw_cursor()
      if snake_priority == 1:
         draw_snake()

      screen.blit(my_font.render(f'{map_layer_data[cursor_y][cursor_x]}', False, (0, 0, 0)), (0, 0))
      screen.blit(my_font.render(f'{cursor_x}, {cursor_y}', False, (0, 0, 0)), (0, 12))

      pygame.display.flip()
      pygame.time.wait(50)


if __name__ == '__main__':
   main()
