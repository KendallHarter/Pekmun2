#include "gba.hpp"

#include "generated/file_select.hpp"
#include "generated/font.hpp"
#include "generated/naming_screen.hpp"
#include "generated/test_tileset.hpp"
#include "generated/title.hpp"

#include "data.hpp"
#include "save_data.hpp"

#include "fmt/core.h"

#include <algorithm>
#include <array>
#include <limits>
#include <utility>

namespace font_chars {
inline constexpr char arrow = 0x05;
}

enum struct title_selection {
   new_game,
   continue_
};

gba::keypad_status keypad;

void disable_all_sprites() noexcept
{
   for (int i = 0; i < 128; ++i) {
      using namespace gba::obj_opt;
      gba::obj{i}.set_attr0(gba::obj_attr0_options{}.set(display::disable).set(rot_scale::disable));
   }
}

// Not in gba.hpp because it only works for certain setups
volatile std::uint16_t* bg_screen_loc_at(gba::bg_opt::screen_base_block loc, int x, int y) noexcept
{
   return gba::bg_screen_loc(loc) + x + y * 32;
}

void write_at_n(
   gba::bg_opt::screen_base_block loc, const char* to_write, const int n, const int x, const int y) noexcept
{
   int write_x = x;
   int write_y = y;
   for (int i = 0; to_write[i] != '\0' && i < n; ++i) {
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

void write_at(gba::bg_opt::screen_base_block loc, const char* to_write, const int x, const int y) noexcept
{
   write_at_n(loc, to_write, std::numeric_limits<int>::max(), x, y);
}

// Common tile locations
// TODO: There's probably a better/more flexible way to do this than hard coding stuff like this
inline constexpr auto start_tileset = std::size(font) / 8;

// Sets up common palettes and tiles used for most places
void common_tile_and_palette_setup()
{
   // Load tiles in
   const auto start_tileset
      = gba::dma3_copy(std::begin(font), std::end(font), gba::bg_char_loc(gba::bg_opt::char_base_block::b0));
   gba::dma3_copy(std::begin(test_tileset), std::end(test_tileset), start_tileset);
   // Load palettes in
   gba::dma3_copy(std::begin(font_pal), std::end(font_pal), gba::bg_palette_addr(0));
   gba::dma3_copy(std::begin(test_tileset_pal), std::end(test_tileset_pal), gba::bg_palette_addr(1));
   gba::dma3_copy(std::begin(font_pal), std::end(font_pal), gba::obj_palette_addr(0));
}

title_selection title_screen() noexcept
{
   gba::lcd.set_options(gba::lcd_options{}.set(gba::lcd_opt::forced_blank::on));

   gba::dma3_copy(std::begin(font), std::end(font), gba::base_obj_tile_addr(3));
   gba::dma3_copy(std::begin(font_pal), std::end(font_pal), gba::obj_palette_addr(0));
   gba::dma3_copy(std::begin(title), std::end(title), gba::bg_screen_loc(gba::bg_opt::screen_base_block::b0));
   gba::bg3.set_options(gba::bg_options{}.set(gba::bg_opt::priority::p3));

   // Disable all sprites to be safe
   disable_all_sprites();

   // Show New Game/Continue options
   const auto disp_sprite_text = [](const char* text, const int x, const int y, const int base_obj_no) {
      int i = 0;
      for (; text[i] != '\0'; ++i) {
         using namespace gba::obj_opt;
         const auto cur_obj = gba::obj{base_obj_no + i};
         cur_obj.set_y_and_attr0(
            y,
            gba::obj_attr0_options{}
               .set(display::enable)
               .set(shape::square)
               .set(colors_pal::c16_p16)
               .set(mosaic::disable)
               .set(mode::normal)
               .set(rot_scale::disable));
         cur_obj.set_x_and_attr1(
            x + i * 8, gba::obj_attr1_options{}.set(size::s8x8).set(vflip::disable).set(hflip::disable));
         // BG3 tiles start at 512
         cur_obj.set_tile(512 + text[i]);
         // cur_obj.set_tile_and_attr2(512 + text[i], gba::obj_attr2_options{}.set(priority::p0).set(palette_num::p0));
      }
      return i + base_obj_no;
   };
   const auto continue_obj = disp_sprite_text("New Game", 160, 16, 0);
   const auto arrow_obj = disp_sprite_text("Continue", 160, 24, continue_obj);

   // Set the arrow to continue instead of new game if any files exist
   int arrow_loc = 0;
   for (int i = 0; i < 4; ++i) {
      if (file_exists(i)) {
         arrow_loc = 1;
         break;
      }
   }
   // Arrow
   {
      using namespace gba::obj_opt;
      gba::obj{arrow_obj}.set_y_and_attr0(
         16 + arrow_loc * 8,
         gba::obj_attr0_options{}
            .set(display::enable)
            .set(shape::square)
            .set(colors_pal::c16_p16)
            .set(mosaic::disable)
            .set(mode::normal)
            .set(rot_scale::disable));
      gba::obj{arrow_obj}.set_x_and_attr1(
         152, gba::obj_attr1_options{}.set(size::s8x8).set(vflip::disable).set(hflip::disable));
      gba::obj{arrow_obj}.set_tile(512 + font_chars::arrow);
   }
   {
      using namespace gba::lcd_opt;
      gba::lcd.set_options(gba::lcd_options{}
                              .set(bg_mode::mode_3)
                              .set(forced_blank::off)
                              .set(display_bg0::off)
                              .set(display_bg1::off)
                              .set(display_bg2::on)
                              .set(display_bg3::off)
                              .set(display_obj::on)
                              .set(display_window_0::off)
                              .set(display_window_1::off)
                              .set(display_window_obj::off)
                              .set(obj_char_mapping::one_dimensional));
   }

   while (true) {
      while (gba::in_vblank()) {}
      while (!gba::in_vblank()) {}

      keypad.update();

      if (keypad.up_pressed()) {
         arrow_loc -= 1;
      }
      else if (keypad.down_pressed()) {
         arrow_loc += 1;
      }

      // Some precaution to prevent auto-selection while soft-reset buttons are held
      if (keypad.a_pressed() && !keypad.b_held() && !keypad.select_held() && !keypad.start_held()) {
         return arrow_loc == 0 ? title_selection::new_game : title_selection::continue_;
      }

      arrow_loc = std::clamp(arrow_loc, 0, 1);

      gba::obj{arrow_obj}.set_y(16 + arrow_loc * 8);
   }
}

int select_file_data(const char* prompt_finish) noexcept
{
   constexpr std::array<std::pair<std::uint8_t, std::uint8_t>, 4> arrow_pos{
      {{3 * 8, 5 * 8}, {18 * 8, 5 * 8}, {3 * 8, 13 * 8}, {18 * 8, 13 * 8}}};

   disable_all_sprites();

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
      gba::copy_tilemap(file_select, screen_base_block::b62);
   }

   const auto write_bg0 = [&](const char* c, int x, int y) { write_at(gba::bg_opt::screen_base_block::b62, c, x, y); };
   // Finish load prompt
   write_bg0(prompt_finish, 15, 1);

   // Set up arrow
   const auto arrow_offset = 8 * font_chars::arrow;
   const auto arrow_obj = gba::obj{0};
   const auto global_data = get_global_save_data();
   int arrow_loc = global_data.last_file_select;
   arrow_loc = std::clamp(arrow_loc, 0, 3);
   gba::dma3_copy(std::begin(font) + arrow_offset, std::end(font) + arrow_offset + 8, gba::base_obj_tile_addr(0));

   {
      using namespace gba::obj_opt;
      arrow_obj.set_attr0(gba::obj_attr0_options{}
                             .set(display::enable)
                             .set(mode::normal)
                             .set(mosaic::disable)
                             .set(shape::square)
                             .set(colors_pal::c16_p16));
      arrow_obj.set_attr1(gba::obj_attr1_options{}.set(size::s8x8).set(vflip::disable).set(hflip::disable));
      arrow_obj.set_tile_and_attr2(0, gba::obj_attr2_options{}.set(palette_num::p0).set(priority::p0));
      arrow_obj.set_loc(arrow_pos[arrow_loc].first, arrow_pos[arrow_loc].second);
   }

   // Set up data if it exists
   for (int i = 0; i < 4; ++i) {
      const auto base_loc = arrow_pos[i];
      if (file_exists(i)) {
         const auto data = get_file_load_data(i);
         write_bg0(data.file_name.data(), base_loc.first / 8 - 2, base_loc.second / 8 + 2);
         write_bg0(frames_to_time(data.frame_count).data(), base_loc.first / 8 - 2, base_loc.second / 8 + 3);
         char buffer[14];
         fmt::format_to_n(buffer, std::size(buffer) - 1, "Lv.{: <10}", data.file_level);
         write_bg0(buffer, base_loc.first / 8 - 2, base_loc.second / 8 + 5);
      }
      else {
         write_bg0("(No data)", base_loc.first / 8, base_loc.second / 8 + 3);
      }
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

   while (true) {
      while (gba::in_vblank()) {}
      while (!gba::in_vblank()) {}

      keypad.update();

      if (keypad.soft_reset_buttons_held()) {
         gba::soft_reset();
      }

      if (keypad.up_pressed()) {
         arrow_loc -= 2;
      }
      else if (keypad.down_pressed()) {
         arrow_loc += 2;
      }
      if (keypad.left_pressed()) {
         arrow_loc -= 1;
      }
      else if (keypad.right_pressed()) {
         arrow_loc += 1;
      }

      arrow_loc = std::clamp(arrow_loc, 0, 3);

      if (keypad.a_pressed()) {
         return arrow_loc;
      }
      else if (keypad.b_pressed()) {
         return -1;
      }

      arrow_obj.set_loc(arrow_pos[arrow_loc].first, arrow_pos[arrow_loc].second);
   }
}

// The longest possible name is only 16 characters
std::array<char, 16> do_naming_screen(const char* prompt, int max_length) noexcept
{
   std::array<char, 16> to_ret;
   std::fill(to_ret.begin(), to_ret.end(), '\0');
   constexpr std::array<std::array<char, 14>, 7> character_mapping = {
      {{'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N'},
       {'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '*', '+'},
       {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n'},
       {'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', ',', '-'},
       {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.', '/', '\\', '|'},
       {'!', '"', '#', '$', '%', '&', '\'', '(', ')', '^', '_', '`', '[', ']'},
       {'{', '}', '~', '<', '=', '>', ':', ';', '@', ' ', ' ', ' ', ' ', ' '}}};
   disable_all_sprites();

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
      gba::copy_tilemap(naming_screen, screen_base_block::b62);
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

   const auto arrow_offset = 8 * font_chars::arrow;
   gba::dma3_copy(std::begin(font) + arrow_offset, std::end(font) + arrow_offset + 8, gba::base_obj_tile_addr(0));

   const auto write_bg0 = [&](const char* c, int x, int y) { write_at(gba::bg_opt::screen_base_block::b62, c, x, y); };
   const auto write_bg0_n
      = [&](const char* c, int n, int x, int y) { write_at_n(gba::bg_opt::screen_base_block::b62, c, n, x, y); };

   write_bg0(prompt, 1, 1);

   const auto arrow_obj = gba::obj{0};
   {
      using namespace gba::obj_opt;
      arrow_obj.set_attr0(gba::obj_attr0_options{}
                             .set(display::enable)
                             .set(mode::normal)
                             .set(mosaic::disable)
                             .set(shape::square)
                             .set(colors_pal::c16_p16));
      arrow_obj.set_attr1(gba::obj_attr1_options{}.set(size::s8x8).set(vflip::disable).set(hflip::disable));
      arrow_obj.set_tile_and_attr2(0, gba::obj_attr2_options{}.set(palette_num::p0).set(priority::p0));
      arrow_obj.set_loc(8, 6 * 8);
   }

   int arrow_x = 0;
   int arrow_y = 0;
   int write_loc = 0;
   while (true) {
      while (gba::in_vblank()) {}
      while (!gba::in_vblank()) {}

      keypad.update();

      if (keypad.soft_reset_buttons_held()) {
         gba::soft_reset();
      }

      if (keypad.left_repeat()) {
         arrow_x -= 1;
      }
      else if (keypad.right_repeat()) {
         arrow_x += 1;
      }
      if (keypad.up_repeat()) {
         arrow_y -= 1;
      }
      else if (keypad.down_repeat()) {
         arrow_y += 1;
      }

      constexpr int max_arrow_x = character_mapping[0].size() - 1;
      constexpr int max_arrow_y = character_mapping.size() - 1;
      arrow_x = std::clamp(arrow_x, 0, max_arrow_x);
      arrow_y = std::clamp(arrow_y, 0, max_arrow_y);

      arrow_obj.set_loc(8 + arrow_x * 16, 6 * 8 + arrow_y * 16);

      if (keypad.a_pressed() && write_loc < max_length) {
         to_ret[write_loc] = character_mapping[arrow_y][arrow_x];
         write_loc += 1;
         write_bg0_n(to_ret.data(), write_loc, 1, 3);
      }

      if (keypad.b_pressed() && write_loc > 0) {
         to_ret[write_loc] = '\0';
         write_loc -= 1;
         // Rewrite spaces to clear the previous letters (16 spaces)
         write_bg0("                ", 1, 3);
         write_bg0_n(to_ret.data(), write_loc, 1, 3);
      }

      if (keypad.start_pressed()) {
         return to_ret;
      }
   }
}

int main()
{
   gba::set_fast_mode();
   while (true) {
      const auto selection = title_screen();
      common_tile_and_palette_setup();
      if (selection == title_selection::continue_) {
         const auto loc = select_file_data("to load from.");
         if (loc != -1) {
            global_save_data data;
            data.last_file_select = loc;
            write_global_save_data(data);
            file_save_data file_data;
            file_data.exists_text = file_exists_text;
            const char file_name[] = "hello it me";
            std::copy(std::begin(file_name), std::end(file_name), file_data.file_name.data());
            file_data.file_level = 10;
            file_data.frame_count = 0x7FFF'FFFF;
            sram_write(file_data, file_save_loc(loc));
         }
      }
      else if (selection == title_selection::new_game) {
         do_naming_screen("Name a snake!", 13);
      }
   }
}
