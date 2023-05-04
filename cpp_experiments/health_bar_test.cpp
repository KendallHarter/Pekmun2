#include "gba.hpp"

#include "generated/font.hpp"
#include "generated/health_bar.hpp"

#include "fmt/core.h"

#include <algorithm>
#include <iterator>

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

int main()
{
   gba::dma3_copy(std::begin(font_pal), std::end(font_pal), gba::bg_palette_addr(0));
   gba::dma3_copy(std::begin(font), std::end(font), gba::bg_char_loc(gba::bg_opt::char_base_block::b0));
   // gba::dma3_copy(std::begin(health_bar), std::end(health_bar), gba::base_obj_tile_addr(0));
   gba::dma3_copy(std::begin(health_bar_pal), std::end(health_bar_pal), gba::obj_palette_addr(0));

   const auto start_bg0 = gba::bg_screen_loc(gba::bg_opt::screen_base_block::b62);
   gba::dma3_fill(start_bg0, start_bg0 + 256 / 8 * 256 / 8, ' ');

   // Set background to light blue
   *gba::bg_palette_addr(0) = make_gba_color(0, 0xBA, 0xFF);

   for (int i = 0; i < 128; ++i) {
      gba::obj{i}.set_attr0(gba::obj_attr0_options{}.set(gba::obj_opt::display::disable));
   }

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

   int hp = 100;
   int max_hp = 100;

   gba::keypad_status keypad;

   // Setup health-bar
   using namespace gba::obj_opt;
   gba::obj{0}.set_y_and_attr0(
      8, gba::obj_attr0_options{}.set(shape::horizontal).set(display::enable).set(colors_pal::c16_p16));
   gba::obj{0}.set_attr1(gba::obj_attr1_options{}.set(size::h16x8));

   while (true) {
      while (gba::in_vblank()) {}
      while (!gba::in_vblank()) {}

      keypad.update();

      if (keypad.left_held()) {
         hp -= 1;
      }
      else if (keypad.right_held()) {
         hp += 1;
      }
      if (keypad.up_held()) {
         max_hp += 1;
      }
      else if (keypad.down_held()) {
         max_hp -= 1;
      }

      hp = std::clamp(hp, 0, max_hp);
      max_hp = std::max(1, max_hp);

      // Set HP bar sprites
      const auto hp_bar_val = 16 * hp / max_hp;
      const auto left_half = std::min(7, hp_bar_val);
      const auto right_half = std::clamp(hp_bar_val, 8, 15);
      std::copy(&health_bar[left_half * 8], &health_bar[left_half * 8] + 8, gba::base_obj_tile_addr(0));
      std::copy(&health_bar[right_half * 8], &health_bar[right_half * 8] + 8, gba::base_obj_tile_addr(0) + 8);

      const auto write_bg0
         = [&](const char* c, int x, int y) { write_at(gba::bg_opt::screen_base_block::b62, c, x, y); };
      char buffer[32];
      for (auto& buf : buffer) {
         buf = ' ';
      }
      buffer[31] = '\0';
      fmt::format_to_n(buffer, std::ssize(buffer) - 1, "{}/{}", hp, max_hp);
      write_bg0(buffer, 0, 0);
      for (auto& buf : buffer) {
         buf = ' ';
      }
      buffer[31] = '\0';
      fmt::format_to_n(buffer, std::ssize(buffer) - 1, "{}\n{}", left_half, right_half);
      write_bg0(buffer, 0, 2);
   }
}
