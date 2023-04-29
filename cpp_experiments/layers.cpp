#include "gba.hpp"

#include "generated/move_indicator.hpp"
#include "generated/snake.hpp"
#include "generated/test_tileset.hpp"

#include <algorithm>
#include <iterator>

constexpr std::uint16_t tiles_0[] = {
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 0,  1,  2,  3,  17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 6,  22, 23, 7,  17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 4,  5,  4,  5,  17, 17, 17, 17, 17,
   17, 0,  1,  2,  3,  17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 6,  22, 23, 7,  17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 5,  4,  17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17};

constexpr std::uint16_t tiles_1[] = {
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
constexpr int map_height = 20;

// clang-format off
constexpr int map_priority[] = {
   0, 1, 1,
   0, 1, 1,
   1, 1, 1,
   1, 0, 0,
};

constexpr int map_height_data[] = {
   2, 0, 1,
   0, 0, 0,
   0, 0, 0,
   0, 0, 1,
};

constexpr int map_walkable[] = {
   1, 1, 1,
   1, 0, 1,
   1, 0, 1,
   1, 0, 1
};
// clang-format on

// These names are bad but map_width/height are already taken
constexpr int info_width = 3;
constexpr int info_height = 4;

void set_obj_tile_info(int obj_no, int palette_number, int tile_no)
{
   volatile std::uint16_t* addr = (std::uint16_t*)(0x7000000 + 0x08 * obj_no);
   *(addr + 2) = (palette_number << 12) | tile_no;
}

void set_obj_x_y(int obj_no, int x, int y)
{
   volatile std::uint16_t* addr = (std::uint16_t*)(0x7000000 + 0x08 * obj_no);
   auto val1 = *addr;
   val1 &= 0xFF00;
   val1 |= y;
   *addr = val1;
   auto val = *(addr + 1);
   val &= 0b1111'1110'0000'0000;
   val |= x;
   *(addr + 1) = val;
}

enum class obj_size {
   s8_8,
   s16_16,
   s32_32,
   s64_64
};

void set_obj_size(int obj_no, obj_size size, bool hflip)
{
   volatile std::uint16_t* addr = (std::uint16_t*)(0x7000000 + 0x08 * obj_no);
   auto val = *(addr + 1);
   val &= 0b0010'1111'1111'1111;
   val |= (static_cast<int>(size) << 14) | (hflip << 12);
   *(addr + 1) = val;
}

enum class style {
   disable,
   enable
};

void set_obj_display(int obj_no, style s)
{
   volatile std::uint16_t* addr = (std::uint16_t*)(0x7000000 + 0x08 * obj_no);
   switch (s) {
   case style::disable: *addr = 0b0000'0010'0000'0000; break;
   case style::enable: *addr = 0b0000'0000'0000'0000; break;
   }
}

void set_obj_priority(int obj_no, int priority)
{
   volatile std::uint16_t* addr = (std::uint16_t*)(0x7000000 + 0x08 * obj_no);
   auto val = *(addr + 2);
   val &= 0b0111'1001'1111'1111;
   val |= (priority << 10);
   *(addr + 2) = val;
}

int main()
{
   constexpr auto blank_tile = 17;
   using namespace gba::lcd_opt;
   volatile std::uint16_t* bg_palettes = (std::uint16_t*)0x5000000;
   volatile std::uint16_t* obj_palettes = (std::uint16_t*)0x5000200;
   volatile std::uint32_t* tiles_data = (std::uint32_t*)0x6000000;
   volatile std::uint32_t* obj_tile_data = (std::uint32_t*)0x6010000;
   volatile std::uint16_t* bg0_base = (std::uint16_t*)0x6008000;
   volatile std::uint16_t* bg1_base = (std::uint16_t*)0x6008800;
   volatile std::uint16_t* bg2_base = (std::uint16_t*)0x600F000;
   volatile std::uint16_t* bg3_base = (std::uint16_t*)((std::uint8_t*)tiles_data + 0x800 * 31);

   const auto end_tiles = gba::dma3_copy(std::begin(test_tileset), std::end(test_tileset), tiles_data);
   gba::dma3_copy(std::begin(move_indicator), std::end(move_indicator), end_tiles);
   gba::dma3_copy(std::begin(test_tileset_pal), std::end(test_tileset_pal), bg_palettes);
   gba::dma3_copy(std::begin(move_indicator_pal), std::end(move_indicator_pal), bg_palettes + 16);
   gba::dma3_fill(bg0_base, bg0_base + 256 / 8 * 256 / 8, blank_tile);
   gba::dma3_fill(bg1_base, bg1_base + 256 / 8 * 256 / 8, blank_tile);
   gba::dma3_fill(bg2_base, bg2_base + 256 / 8 * 256 / 8, blank_tile);
   gba::dma3_fill(bg3_base, bg3_base + 256 / 8 * 256 / 8, blank_tile);
   gba::dma3_copy(std::begin(snake), std::end(snake), obj_tile_data);
   gba::dma3_copy(std::begin(snake_pal), std::end(snake_pal), obj_palettes);

   // Set-up bg0 and bg1 properties
   *(volatile std::uint16_t*)(0x4000008) = 0b0001'0000'0000'0000;
   *(volatile std::uint16_t*)(0x400000A) = 0b0001'0001'0000'0001;
   // Set-up bg2 and bg3 properties
   *(volatile std::uint16_t*)(0x400000C) = 0b0001'1110'0000'0000;
   *(volatile std::uint16_t*)(0x400000E) = 0b0001'1111'0000'0001;

   // Disable all sprites to start
   for (int i = 0; i < 128; ++i) {
      set_obj_display(i, style::disable);
   }

   // Enable the first two sprites and set them to 16x16
   for (int i = 0; i < 2; ++i) {
      set_obj_display(i, style::enable);
      set_obj_size(i, obj_size::s16_16, false);
   }

   // Lower half of the snake
   set_obj_tile_info(1, 0, 4);

   const auto bg2_loc
      = [&](int x, int y) { return ((volatile std::uint16_t*)((std::uint8_t*)bg2_base + x * 2 + y * 0x40)); };
   const auto bg3_loc = [&](int x, int y) {
      return ((volatile std::uint16_t*)((std::uint8_t*)tiles_data + 0x800 * 31 + x * 2 + y * 0x40));
   };

   for (auto y = 0; y < 20; ++y) {
      for (auto x = 0; x < 30; ++x) {
         *bg2_loc(x, y) = tiles_0[x + y * map_width];
         *bg3_loc(x, y) = tiles_1[x + y * map_width];
      }
   }

   gba::lcd.set_options(gba::lcd_options{}
                           .set(bg_mode{0})
                           .set(forced_blank::off)
                           .set(display_bg0::on)
                           .set(display_bg1::on)
                           .set(display_bg2::on)
                           .set(display_bg3::on)
                           .set(display_obj::on)
                           .set(display_window_0::off)
                           .set(display_window_1::off)
                           .set(display_window_obj::off)
                           .set(obj_char_mapping::one_dimensional));

   int snake_x = 0;
   int snake_y = 0;
   gba::keypad_status keypad;
   std::array<std::uint16_t, 256 / 8 * 256 / 8> bg0_buffer{};
   std::array<std::uint16_t, 256 / 8 * 256 / 8> bg1_buffer{};
   gba::dma3_fill(bg0_buffer.begin(), bg0_buffer.end(), blank_tile);
   gba::dma3_fill(bg1_buffer.begin(), bg1_buffer.end(), blank_tile);
   bool move_showing = false;
   while (true) {
      // wait for vblank to end
      while ((*(volatile std::uint16_t*)(0x4000004) & 1)) {}
      // wait for vblank to start
      while (!(*(volatile std::uint16_t*)(0x4000004) & 1)) {}

      keypad.update();
      if (keypad.left_pressed()) {
         snake_x -= 1;
      }
      else if (keypad.right_pressed()) {
         snake_x += 1;
      }
      if (keypad.up_pressed()) {
         snake_y -= 1;
      }
      else if (keypad.down_pressed()) {
         snake_y += 1;
      }
      if (keypad.a_pressed()) {
         if (move_showing) {
            // clear out the move location
            gba::dma3_fill(bg0_buffer.begin(), bg0_buffer.end(), blank_tile);
            gba::dma3_fill(bg1_buffer.begin(), bg1_buffer.end(), blank_tile);
            gba::dma3_fill(bg0_base, bg0_base + 256 / 8 * 256 / 8, blank_tile);
            gba::dma3_fill(bg1_base, bg1_base + 256 / 8 * 256 / 8, blank_tile);
            move_showing = false;
         }
         else {
            constexpr auto start_indic = std::size(test_tileset) / 8;
            for (int y = 0; y < info_height; ++y) {
               for (int x = 0; x < info_width; ++x) {
                  // only show movement on walkable tiles
                  if (map_walkable[x + y * info_width]) {
                     const auto tile_x = (96 + x * 16 + y * 16) / 8;
                     const auto tile_y = (64 + 8 * y - 8 * x - 8 * map_height_data[x + y * info_width]) / 8;
                     constexpr auto palette_1 = 1 << 12;
                     auto& buffer = map_priority[x + y * info_width] == 0 ? bg0_buffer : bg1_buffer;
                     // TODO: This works, but feels really messy
                     //       There's probably a better way to do it
                     // write the tile information
                     if (buffer[tile_x + tile_y * 256 / 8] == blank_tile) {
                        buffer[tile_x + 0 + tile_y * 256 / 8] = start_indic | palette_1;
                        buffer[tile_x + 1 + tile_y * 256 / 8] = (start_indic + 1) | palette_1;
                     }
                     else {
                        buffer[tile_x + 0 + tile_y * 256 / 8] = (start_indic + 8) | palette_1;
                        buffer[tile_x + 1 + tile_y * 256 / 8] = (start_indic + 9) | palette_1;
                     }
                     buffer[tile_x + 2 + tile_y * 256 / 8] = (start_indic + 2) | palette_1;
                     buffer[tile_x + 3 + tile_y * 256 / 8] = (start_indic + 3) | palette_1;
                     if (buffer[tile_x + (tile_y + 1) * 256 / 8] == blank_tile) {
                        buffer[tile_x + 0 + (tile_y + 1) * 256 / 8] = (start_indic + 4) | palette_1;
                        buffer[tile_x + 1 + (tile_y + 1) * 256 / 8] = (start_indic + 5) | palette_1;
                     }
                     else {
                        buffer[tile_x + 0 + (tile_y + 1) * 256 / 8] = (start_indic + 10) | palette_1;
                        buffer[tile_x + 1 + (tile_y + 1) * 256 / 8] = (start_indic + 11) | palette_1;
                     }
                     buffer[tile_x + 2 + (tile_y + 1) * 256 / 8] = (start_indic + 6) | palette_1;
                     buffer[tile_x + 3 + (tile_y + 1) * 256 / 8] = (start_indic + 7) | palette_1;
                  }
               }
            }
            // copy the buffers over
            gba::dma3_copy(std::begin(bg0_buffer), std::end(bg0_buffer), bg0_base);
            gba::dma3_copy(std::begin(bg1_buffer), std::end(bg1_buffer), bg1_base);
            move_showing = true;
         }
      }

      snake_x = std::clamp(snake_x, 0, info_width - 1);
      snake_y = std::clamp(snake_y, 0, info_height - 1);

      // Set Snake attributes
      const auto base_x = 104 + snake_x * 16 + snake_y * 16;
      const auto base_y = 42 + snake_y * 8 - 8 * snake_x - 8 * map_height_data[snake_x + info_width * snake_y];
      set_obj_x_y(0, base_x, base_y);
      set_obj_x_y(1, base_x, base_y + 16);
      const auto priority = map_priority[snake_x + info_width * snake_y];
      set_obj_priority(0, priority);
      set_obj_priority(1, priority);
   }
}
