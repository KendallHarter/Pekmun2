#include "gba.hpp"

#include "generated/font.hpp"
#include "generated/health_bar.hpp"

#include "fmt/core.h"

#include <algorithm>
#include <array>
#include <iterator>
#include <tuple>

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
   // enable sound
   volatile std::uint16_t* sound_1_4_level = (std::uint16_t*)0x4000080;
   volatile std::uint16_t* sound_control_h = (std::uint16_t*)0x4000082;
   volatile std::uint16_t* sound_enable = (std::uint16_t*)0x4000084;

   // Sound A enable left/right and timer 0
   *sound_enable = 0b0000'0000'1000'0000;
   *sound_1_4_level = 0b1111'1111'0111'0111;
   *sound_control_h = 0b0000'1011'0000'1110;

   const auto write_bg0 = [&](const char* c, int x, int y) { write_at(gba::bg_opt::screen_base_block::b62, c, x, y); };

   const auto write_bg0_char
      = [&](char c, int x, int y) { *bg_screen_loc_at(gba::bg_opt::screen_base_block::b62, x, y) = c; };

   gba::dma3_copy(std::begin(font_pal), std::end(font_pal), gba::bg_palette_addr(0));
   gba::dma3_copy(std::begin(font), std::end(font), gba::bg_char_loc(gba::bg_opt::char_base_block::b0));
   const auto start = gba::bg_screen_loc(gba::bg_opt::screen_base_block::b62);
   gba::dma3_fill(start, start + 32 * 32, ' ');

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

   const auto options = std::to_array<const char*>(
      {"Sweep Shift",
       "Sweep Freq. Dir.",
       "Sweep Time",
       "Length",
       "Wave Pattern Duty",
       "Envelope Step Time",
       "Envelope Dir.",
       "Init. Vol. of Envelope",
       "Frequency",
       "Length Flag"});

   std::array<std::tuple<int, int, int, int>, options.size()> values{{// value, value offset, num bits, start bit pos
                                                                      std::tuple<int, int, int, int>{0, 0, 3, 0},
                                                                      std::tuple<int, int, int, int>{0, 0, 1, 3},
                                                                      std::tuple<int, int, int, int>{0, 0, 3, 4},
                                                                      std::tuple<int, int, int, int>{0, 1, 6, 0},
                                                                      std::tuple<int, int, int, int>{0, 1, 2, 6},
                                                                      std::tuple<int, int, int, int>{0, 1, 3, 8},
                                                                      std::tuple<int, int, int, int>{0, 1, 1, 11},
                                                                      std::tuple<int, int, int, int>{0, 1, 3, 12},
                                                                      std::tuple<int, int, int, int>{0, 2, 11, 0},
                                                                      std::tuple<int, int, int, int>{0, 2, 1, 14}}};

   for (int i = 0; i < std::ssize(options); ++i) {
      write_bg0(options[i], 1, 2 * i);
   }

   int option = 0;

   while (true) {
      while (gba::in_vblank()) {}
      while (!gba::in_vblank()) {}

      keypad.update();

      // Erase arrow
      write_bg0_char(' ', 0, option * 2);
      if (keypad.down_repeat()) {
         option += 1;
      }
      else if (keypad.up_repeat()) {
         option -= 1;
      }
      if (keypad.left_repeat()) {
         const auto change = keypad.r_held() ? 100 : 1;
         auto& val = std::get<0>(values[option]);
         val -= change;
         if (val < 0) {
            val = 0;
         }
      }
      else if (keypad.right_repeat()) {
         const auto change = keypad.r_held() ? 100 : 1;
         auto& val = std::get<0>(values[option]);
         const auto num_bits = std::get<2>(values[option]);
         val += change;
         const auto limit = (1 << num_bits) - 1;
         if (val > limit) {
            val = limit;
         }
      }
      option = std::clamp(option, 0, static_cast<int>(values.size() - 1));
      // Draw arrow
      write_bg0_char(arrow, 0, option * 2);

      // Draw values
      int y = 0;
      for (const auto& [val, offset, num_bits, start_loc] : values) {
         std::array<char, 16> buffer;
         std::fill(buffer.begin(), buffer.end(), '\0');
         fmt::format_to_n(buffer.data(), num_bits + 1, "{:0>{}b}", val, num_bits);
         write_bg0(buffer.data(), 2, 1 + y * 2);
         y += 1;
      }

      // Play sound!
      if (keypad.a_pressed()) {
         std::array<std::uint16_t, 3> sound_values{0, 0, 0};
         for (const auto& [val, offset, num_bits, start_loc] : values) {
            // We don't need to AND the values away because they always start at 0
            sound_values[offset] |= (val << start_loc);
         }
         // Write the reset sound bit
         sound_values[2] |= 0b1000'0000'0000'0000;
         for (int i = 0; i < std::ssize(sound_values); ++i) {
            volatile std::uint16_t* sound_ptr = (std::uint16_t*)(0x4000060 + 2 * i);
            *sound_ptr = sound_values[i];
         }
      }
   }
}
