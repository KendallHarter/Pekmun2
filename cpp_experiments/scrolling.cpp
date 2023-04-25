#include "gba.hpp"

#include "generated/font.hpp"
#include "generated/test_tileset.hpp"

#include <charconv>
#include <utility>

constexpr std::uint16_t tiles[] = {
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 0,  1,  2,  3,  17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 6,  22, 23, 7,  17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 0,  1,  21, 25, 12, 9,  2,  3,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 0,  1,  21, 22, 23, 24, 10, 22, 8,
   9,  2,  3,  17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 4,  5,  12, 9,  10, 11, 18, 18,
   21, 22, 8,  9,  2,  3,  17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 4,  18, 21, 22, 8,  9,  10,
   11, 18, 18, 18, 22, 23, 5,  17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 13, 14, 18, 18, 21, 22,
   8,  9,  10, 11, 18, 5,  4,  5,  17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 13, 14, 18,
   18, 21, 22, 23, 7,  13, 20, 19, 16, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   13, 14, 18, 5,  4,  5,  17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 13, 20, 19, 16, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 0,  1,  2,  3,  17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 6,  22, 23, 7,  17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 0,  1,  21, 25, 12, 9,  2,  3,  17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 0,  1,  21, 22, 23, 24, 10, 22, 8,  9,  2,  3,  17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 4,  5,  12, 9,  10, 11, 18, 18, 21, 22, 8,  9,  2,  3,  17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 4,  18, 21, 22, 8,  9,  10, 11, 18, 18, 18, 22, 23, 5,  17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 13, 14, 18, 18, 21, 22, 8,  9,  10, 11, 18, 5,  4,  5,  17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 13, 14, 18, 18, 21, 22, 23, 7,  13, 20, 19, 16,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 13, 14, 18, 5,  4,  5,  17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 13, 20, 19, 16, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17};

constexpr int map_width = 30;
constexpr int map_height = 40;

int main()
{
   constexpr auto blank_tile = 17;
   using namespace gba::lcd_opt;
   volatile std::uint16_t* bg_palettes = (std::uint16_t*)0x5000000;
   volatile std::uint32_t* tiles_data = (std::uint32_t*)0x6000000;
   volatile std::uint16_t* bg0_base = (std::uint16_t*)0x600F000;
   volatile std::uint16_t* bg1_base = (std::uint16_t*)((std::uint8_t*)tiles_data + 0x800 * 31);
   const auto text_write_loc = gba::dma3_copy(std::begin(test_tileset), std::end(test_tileset), tiles_data);
   gba::dma3_copy(std::begin(test_tileset_pal), std::end(test_tileset_pal), bg_palettes);
   gba::dma3_copy(std::begin(font), std::end(font), text_write_loc);
   gba::dma3_fill(bg0_base, bg0_base + 256 / 8 * 256 / 8, blank_tile);
   gba::dma3_fill(bg1_base, bg1_base + 256 / 8 * 256 / 8, blank_tile);
   *(volatile std::uint16_t*)(0x4000008) = 0b0001'1110'0000'0011;
   *(volatile std::uint16_t*)(0x400000A) = 0b0001'1111'0000'0010;
   const auto bg0_loc
      = [&](int x, int y) { return ((volatile std::uint16_t*)((std::uint8_t*)bg0_base + x * 2 + y * 0x40)); };
   const auto bg1_loc = [&](int x, int y) {
      return ((volatile std::uint16_t*)((std::uint8_t*)tiles_data + 0x800 * 31 + x * 2 + y * 0x40));
   };
   const auto write_it = [&](const char* c, int x, int y) {
      for (int i = 0; c[i] != '\0'; ++i) {
         *bg0_loc(x + i, y) = c[i] + 26;
      }
   };
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
                           .set(display_bg0::on)
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
   write_it("hello", 0, 0);
   // initialize the map
   const auto init_map = [&]() {
      for (int y = 0; y != 20; ++y) {
         const auto tile_y = y;
         for (int x = 0; x != 30; ++x) {
            const auto tile_x = x;
            *bg1_loc(tile_x % (256 / 8), tile_y % (256 / 8)) = [&]() -> int {
               if (tile_x >= map_width || tile_y >= map_height || tile_x < 0 || tile_y < 0) {
                  return blank_tile;
               }
               else {
                  return tiles[tile_x + tile_y * map_width];
               };
            }();
         }
      }
   };
   init_map();
   while (true) {
      // wait for vblank to end
      while ((*(volatile std::uint16_t*)(0x4000004) & 1)) {}
      // wait for vblank to start
      while (!(*(volatile std::uint16_t*)(0x4000004) & 1)) {}
      keypad.update();
      if (keypad.start_pressed()) {
         init_map();
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
      const auto start_line = *(volatile std::uint16_t*)(0x4000006);
      const auto offset_x = camera_x / 8;
      const auto offset_y = camera_y / 8;
      // Run 4 times to see speed when doing this with 4 layers
      for (int i = 0; i < 4; ++i) {
         // Only the edges need to be updated (when scrolling <= 8px/frame)
         for (auto y : {0, 20}) {
            const auto tile_y = y + offset_y;
            // TODO: This can be done with DMA (a bit complicated, but faster)
            for (auto x = 0; x < 31; ++x) {
               const auto tile_x = x + offset_x;
               *bg1_loc(tile_x % (256 / 8), tile_y % (256 / 8)) = [&]() -> int {
                  if (tile_x >= map_width || tile_y >= map_height || tile_x < 0 || tile_y < 0) {
                     return blank_tile;
                  }
                  else {
                     return tiles[tile_x + tile_y * map_width];
                  };
               }();
            }
         }
         for (auto x : {0, 30}) {
            const auto tile_x = x + offset_x;
            for (auto y = 0; y < 21; ++y) {
               const auto tile_y = y + offset_y;
               *bg1_loc(tile_x % (256 / 8), tile_y % (256 / 8)) = [&]() -> int {
                  if (tile_x >= map_width || tile_y >= map_height || tile_x < 0 || tile_y < 0) {
                     return blank_tile;
                  }
                  else {
                     return tiles[tile_x + tile_y * map_width];
                  };
               }();
            }
         }
      }
      const auto end_line = *(volatile std::uint16_t*)(0x4000006);
      char tester[10] = {'\0'};
      std::to_chars(std::begin(tester), std::end(tester), end_line - start_line);
      write_it(tester, 0, 1);
   }
}
