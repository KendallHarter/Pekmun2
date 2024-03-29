#include "gba.hpp"

#include "generated/font.hpp"
#include "generated/snake.hpp"

#include "fmt/core.h"

#include <algorithm>
#include <cmath>
#include <numbers>

std::uint16_t float_to_fixed(float f)
{
   const auto max_fraction = 0b0'0000000'11111111;
   const auto max_integer = 0b0'1111111'00000000 >> 8;
   const bool is_negative = f < 0.0;
   float integral;
   const auto fraction = std::abs(std::modf(f, &integral));
   const auto frac_part = std::clamp(static_cast<int>(fraction * max_fraction), 0, max_fraction);
   const auto int_part = std::clamp(static_cast<int>(std::abs(integral)), 0, max_integer);
   const auto ret_val = (int_part << 8) | frac_part;
   return is_negative ? ~ret_val : ret_val;
}

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

   const auto snake_obj = gba::obj{0};
   const auto snake_obj2 = gba::obj{1};
   {
      using namespace gba::obj_opt;
      for (int i = 0; i < 128; ++i) {
         gba::obj{i}.set_attr0(gba::obj_attr0_options{}.set(display::disable));
      }
      for (int i = 0; i < 2; ++i) {
         gba::obj{i}.set_attr0(gba::obj_attr0_options{}
                                  .set(rot_scale::enable)
                                  .set(double_size::disable)
                                  .set(mode::normal)
                                  .set(mosaic::disable)
                                  .set(shape::vertical));
         gba::obj{i}.set_attr1(gba::obj_attr1_options{}.set(size::v16x32));
      }
   }

   snake_obj.set_x(120);
   snake_obj.set_y(80);
   snake_obj2.set_x(200);
   snake_obj2.set_y(80);
   snake_obj2.set_attr1(gba::obj_attr1_options{}.set(gba::obj_opt::rot_scale_param::p1));

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
   int angle = 0;

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
         values[option] ^= 0b1111'1111'1111'1111;
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
      // 2nd Group - PA=07000026, PB=0700002E, PC=07000036, PD=0700003E
      std::array<volatile std::uint16_t*, 4> locs2{
         (std::uint16_t*)0x07000026,
         (std::uint16_t*)0x0700002E,
         (std::uint16_t*)0x07000036,
         (std::uint16_t*)0x0700003E};
      angle += 1;
      angle %= 360;
      //   pa = x_scale * cos(angle)
      //   pb = y_scale * sin(angle)
      //   pc = x_scale * -sin(angle)
      //   pd = y_scale * cos(angle)
      const auto deg_to_rad = [](float f) { return f * std::numbers::pi / 180.0; };

      std::array<double, 4> test_values{
         std::cos(deg_to_rad(angle)),
         std::sin(deg_to_rad(angle)),
         -std::sin(deg_to_rad(angle)),
         std::cos(deg_to_rad(angle)),
      };
      for (auto i = 0; i < 4; ++i) {
         *locs2[i] = float_to_fixed(test_values[i]);
         std::array<char, 20> buffer;
         std::fill(buffer.begin(), buffer.end(), '\0');
         fmt::format_to_n(buffer.data(), buffer.size() - 1, "{}", test_values[i]);
         write_bg0(buffer.data(), 1, 5 + i);
      }
   }
}
