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

with open('../assets/test_map.json') as f:
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

map_layer_data = [
   [1, 0, 0],
   [1, 0, 0],
   [0, 0, 0],
   [0, 1, 1]
]

map_height_data = [
   [2, 0, 1],
   [0, 0, 0],
   [0, 0, 0],
   [0, 0, 1]
]

map_walkable = [
   [1, 1, 1],
   [1, 0, 1],
   [1, 0, 1],
   [1, 0, 1]
]

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
      screen.blit(snake, (104 + snake_x * 16 + snake_y * 16, 42 + snake_y * 8 - 8 * snake_x - 8 * map_height_data[snake_y][snake_x]))

   def draw_cursor():
      screen.blit(cursor, (96 + cursor_x * 16 + cursor_y * 16, 47 + cursor_y * 8 - 8 * cursor_x - 8 * map_height_data[cursor_y][cursor_x]))

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

      if keys[pygame.K_z]:
         if not selected and cursor_x == snake_x and cursor_y == snake_y:
            for y in range(4):
               for x in range(3):
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

      cursor_x = clamp(cursor_x, 0, 2)
      cursor_y = clamp(cursor_y, 0, 3)

      snake_priority = map_layer_data[snake_y][snake_x]
      cursor_priority = map_layer_data[cursor_y][cursor_x]

      screen.fill((0, 0xff, 0))
      draw_tile_layer(0)
      for x, y in move_layer_0:
         screen.blit(move_indic, (96 + x * 16 + y * 16, 64 + 8 * y - 8 * x - 8 * map_height_data[y][x]))
      if cursor_priority == 0:
         draw_cursor()
      if snake_priority == 0:
         draw_snake()

      draw_tile_layer(1)
      for x, y in move_layer_1:
         screen.blit(move_indic, (96 + x * 16 + y * 16, 64 + 8 * y - 8 * x - 8 * map_height_data[y][x]))
      if cursor_priority == 1:
         draw_cursor()
      if snake_priority == 1:
         draw_snake()

      pygame.display.flip()
      pygame.time.wait(50)


if __name__ == '__main__':
   main()
