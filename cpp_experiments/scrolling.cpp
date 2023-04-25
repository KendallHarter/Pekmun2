#include "gba.hpp"

#include "generated/test_tileset.hpp"

#include <utility>

constexpr std::uint16_t tiles[] = {
   0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0,
   0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0,
   0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0,
   0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0,
   0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0,
   0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0,
   0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  7,  23, 24, 8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0,
   0, 0,  0,  0,  0,  0,  0,  1,  2,  22, 26, 13, 10, 3,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0,
   0, 0,  0,  0,  1,  2,  22, 23, 24, 25, 11, 23, 9,  10, 3,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0,
   0, 0,  0,  5,  6,  13, 10, 11, 12, 19, 19, 22, 23, 9,  10, 3,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0,
   0, 0,  5,  19, 22, 23, 9,  10, 11, 12, 19, 19, 19, 23, 24, 6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0,
   0, 14, 15, 19, 19, 22, 23, 9,  10, 11, 12, 19, 6,  5,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0,
   0, 0,  14, 15, 19, 19, 22, 23, 24, 8,  14, 21, 20, 17, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0,
   0, 0,  0,  14, 15, 19, 6,  5,  6,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0,
   0, 0,  0,  0,  14, 21, 20, 17, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0,
   0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0,
   0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0,
   0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0,
   0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0,
   0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0,
   0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0,
   0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0,
   0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0,
   0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0,
   0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  0, 0, 0, 0, 0,
   0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  7,  23, 24, 8,  0,  0, 0, 0, 0, 0,
   0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  22, 26, 13, 10, 3,  4,  0, 0, 0, 0, 0,
   0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  22, 23, 24, 25, 11, 23, 9,  10, 3,  4, 0, 0, 0, 0,
   0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  5,  6,  13, 10, 11, 12, 19, 19, 22, 23, 9,  10, 3, 4, 0, 0, 0,
   0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  5,  19, 22, 23, 9,  10, 11, 12, 19, 19, 19, 23, 24, 6, 0, 0, 0, 0,
   0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  14, 15, 19, 19, 22, 23, 9,  10, 11, 12, 19, 6,  5,  6,  0, 0, 0, 0, 0,
   0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  14, 15, 19, 19, 22, 23, 24, 8,  14, 21, 20, 17, 0,  0, 0, 0, 0, 0,
   0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  14, 15, 19, 6,  5,  6,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0,
   0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  14, 21, 20, 17, 0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0,
   0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0,
   0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0,
   0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0,
   0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0,
   0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0};

constexpr int map_width = 30;
constexpr int map_height = 40;

int main()
{
   constexpr auto blank_tile = 17;
   using namespace gba::lcd_opt;
   volatile std::uint16_t* bg_palettes = (std::uint16_t*)0x5000000;
   volatile std::uint32_t* tiles_data = (std::uint32_t*)0x6000000;
   gba::dma3_copy(std::begin(test_tileset), std::end(test_tileset), (std::uint32_t*)tiles_data);
   gba::dma3_copy(std::begin(test_tileset_pal), std::end(test_tileset_pal), (std::uint16_t*)bg_palettes);
   *(volatile std::uint16_t*)(0x4000008) = 0b0001'1110'0000'0011;
   *(volatile std::uint16_t*)(0x400000A) = 0b0001'1111'0000'0010;
   const auto bg1_loc = [&](int x, int y) {
      return ((volatile std::uint16_t*)((std::uint8_t*)tiles_data + 0x800 * 31 + x * 2 + y * 0x40));
   };
   // gba::dma3_copy(std::begin(tiles), std::end(tiles), (std::uint16_t*)tiles_data + 0x800 * 31);
   for (int y = 0; y < 20; ++y) {
      for (int x = 0; x < 30; ++x) {
         const auto tile = tiles[x + y * map_width];
         if (tile == 0) {
            *bg1_loc(x, y) = blank_tile;
         }
         else {
            *bg1_loc(x, y) = tiles[x + y * map_width] - 1;
         }
      }
   }
   gba::lcd.set_options(gba::lcd_options{}
                           .set(bg_mode{0})
                           .set(forced_blank::off)
                           .set(display_bg1::on)
                           .set(display_obj::off)
                           .set(display_window_0::off)
                           .set(display_window_1::off)
                           .set(display_window_obj::off)
                           .set(obj_char_mapping::one_dimensional));
   int camera_x = 0;
   int camera_y = 0;
   volatile std::uint16_t* bg1_x_scroll_loc = (std::uint16_t*)0x4000014;
   volatile std::uint16_t* bg1_y_scroll_loc = (std::uint16_t*)0x4000016;
   gba::keypad_status keypad;
   while (true) {
      // wait for vblank to end
      while ((*(volatile std::uint16_t*)(0x4000004) & 1)) {}
      // wait for vblank to start
      while (!(*(volatile std::uint16_t*)(0x4000004) & 1)) {}
      keypad.update();
      if (keypad.start_pressed()) {
         camera_x = 0;
         camera_y = 0;
      }
      const auto change_amount = keypad.r_held() ? 5 : 1;
      if (keypad.left_held()) {
         camera_x -= change_amount;
      }
      else if (keypad.right_held()) {
         camera_x += change_amount;
      }
      if (keypad.up_held()) {
         camera_y -= change_amount;
      }
      else if (keypad.down_held()) {
         camera_y += change_amount;
      }
      // The scrolling doesn't work properly when these are less than zero due
      // to overflow math not co-operating; to prevent this, simply have everything
      // loaded to an offset great enough that we never need to scroll past 0
      if (camera_x < 0) {
         camera_x = 0;
      }
      if (camera_y < 0) {
         camera_y = 0;
      }
      *bg1_x_scroll_loc = camera_x;
      *bg1_y_scroll_loc = camera_y;
      const auto offset_x = camera_x / 8;
      const auto offset_y = camera_y / 8;
      for (int y = 0; y < 21; ++y) {
         const auto tile_y = y + offset_y;
         for (int x = 0; x < 31; ++x) {
            const auto tile_x = x + offset_x;
            *bg1_loc(tile_x % (256 / 8), tile_y % (256 / 8)) = [&]() {
               if (tile_x >= map_width || tile_y >= map_height || tile_x < 0 || tile_y < 0) {
                  return blank_tile;
               }
               else if (tiles[tile_x + tile_y * map_width] == 0) {
                  return blank_tile;
               }
               else {
                  return tiles[tile_x + tile_y * map_width] - 1;
               };
            }();
         }
      }
   }
}
