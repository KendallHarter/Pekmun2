#include "gba.hpp"

#include "generated/font.hpp"
#include "generated/snake.hpp"

#include "fmt/core.h"

#include <algorithm>

volatile std::uint16_t* bg_screen_loc_at(gba::bg_opt::screen_base_block loc, int x, int y) noexcept
{
   return gba::bg_screen_loc(loc) + x + y * 32;
}

void write_at(gba::bg_opt::screen_base_block loc, const char* to_write, const int x, const int y) noexcept
{
   int write_x = x;
   int write_y = y;
   for (int i = 0; to_write[i] != '\0'; ++i) {
      if (to_write[i] == '\n') {
         write_x = x;
         write_y += 1;
      }
      else {
         *bg_screen_loc_at(loc, write_x, write_y) = to_write[i];
         write_x += 1;
      }
   }
}

inline constexpr char arrow = 0x05;

int main()
{
   const auto write_bg0 = [&](const char* c, int x, int y) { write_at(gba::bg_opt::screen_base_block::b62, c, x, y); };

   const auto write_bg0_char
      = [&](char c, int x, int y) { *bg_screen_loc_at(gba::bg_opt::screen_base_block::b62, x, y) = c; };

   gba::dma3_copy(std::begin(font_pal), std::end(font_pal), gba::bg_palette_addr(0));
   gba::dma3_copy(std::begin(font), std::end(font), gba::bg_char_loc(gba::bg_opt::char_base_block::b0));
   const auto start = gba::bg_screen_loc(gba::bg_opt::screen_base_block::b62);
   gba::dma3_fill(start, start + 32 * 32, ' ');
   gba::dma3_copy(std::begin(snake), std::end(snake), gba::base_obj_tile_addr(0));
   gba::dma3_copy(std::begin(snake_pal), std::end(snake_pal), gba::obj_palette_addr(0));

   *gba::bg_palette_addr(0) = gba::make_gba_color(0x00, 0x00, 0xA0);

   const auto snake_obj = gba::obj{0};
   {
      using namespace gba::obj_opt;
      for (int i = 0; i < 128; ++i) {
         gba::obj{i}.set_attr0(gba::obj_attr0_options{}.set(display::disable));
      }
      snake_obj.set_attr0(gba::obj_attr0_options{}
                             .set(rot_scale::enable)
                             .set(double_size::disable)
                             .set(mode::normal)
                             .set(mosaic::disable)
                             .set(shape::vertical));
      snake_obj.set_attr1(gba::obj_attr1_options{}.set(size::v16x32));
   }

   snake_obj.set_x(120);
   snake_obj.set_y(80);

   // Turn on screen
   {
      using namespace gba::lcd_opt;
      gba::lcd.set_options(gba::lcd_options{}
                              .set(bg_mode::mode_0)
                              .set(forced_blank::off)
                              .set(display_bg0::on)
                              .set(display_bg1::off)
                              .set(display_bg2::off)
                              .set(display_bg3::off)
                              .set(display_obj::on)
                              .set(display_window_0::off)
                              .set(display_window_1::off)
                              .set(display_window_obj::off)
                              .set(obj_char_mapping::one_dimensional));
   }

   {
      using namespace gba::bg_opt;
      gba::bg0.set_options(gba::bg_options{}
                              .set(priority::p0)
                              .set(char_base_block::b0)
                              .set(mosaic::disable)
                              .set(colors_palettes::c16_p16)
                              .set(screen_base_block::b62)
                              .set(display_area_overflow::transparent)
                              .set(screen_size::text_256x256));
   }

   gba::keypad_status keypad;
   int option = 0;
   std::array<int, 4> values;

   while (true) {
      while (gba::in_vblank()) {}
      while (!gba::in_vblank()) {}

      keypad.update();

      // Erase arrow
      write_bg0_char(' ', 0, option);
      if (keypad.down_repeat()) {
         option += 1;
      }
      else if (keypad.up_repeat()) {
         option -= 1;
      }
      option = std::clamp(option, 0, 3);

      if (keypad.left_repeat()) {
         const auto change = keypad.r_held() ? 100 : 1;
         auto& val = values[option];
         val -= change;
         if (val < 0) {
            val = 0;
         }
      }
      else if (keypad.right_repeat()) {
         const auto change = keypad.r_held() ? 100 : 1;
         auto& val = values[option];
         val += change;
         const auto limit = 0xFFFF;
         if (val > limit) {
            val = limit;
         }
      }

      if (keypad.a_pressed()) {
         values[option] ^= 0b1000'0000'0000'0000;
      }
      // Draw arrow
      write_bg0_char(arrow, 0, option);

      // Draw values
      for (auto i = 0; i < 4; ++i) {
         std::array<char, 20> buffer;
         std::fill(buffer.begin(), buffer.end(), '\0');
         fmt::format_to_n(buffer.data(), buffer.size() - 1, "{:0>16b}", values[i]);
         write_bg0(buffer.data(), 1, i);
      }

      // 1st Group - PA=07000006, PB=0700000E, PC=07000016, PD=0700001E
      std::array<volatile std::uint16_t*, 4> locs{
         (std::uint16_t*)0x07000006,
         (std::uint16_t*)0x0700000E,
         (std::uint16_t*)0x07000016,
         (std::uint16_t*)0x0700001E};
      for (int i = 0; i < 4; ++i) {
         *locs[i] = values[i];
      }
   }
}
