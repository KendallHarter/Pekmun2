#include "gba.hpp"

#include "generated/file_select.hpp"
#include "generated/font.hpp"
#include "generated/naming_screen.hpp"
#include "generated/pal1.hpp"
#include "generated/stats_screen.hpp"
#include "generated/test_tileset.hpp"
#include "generated/title.hpp"
#include "generated/win_screen.hpp"

#include "classes.hpp"
#include "data.hpp"
#include "map_data.hpp"
#include "save_data.hpp"
#include "static_vector.hpp"

#include "fmt/core.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <limits>
#include <span>
#include <tuple>
#include <utility>

// This isn't an enum struct because we want them to be chars
namespace font_chars {

inline constexpr char box_ul = 0x01;
inline constexpr char box_top = 0x02;
inline constexpr char box_ur = 0x03;
inline constexpr char box_left_vertical = 0x04;
inline constexpr char arrow = 0x05;
inline constexpr char hollow_arrow = 0x06;
inline constexpr char down_arrow = 0x07;
inline constexpr char up_arrow = 0x08;
inline constexpr char box_bl = 0x11;
inline constexpr char box_bottom = 0x12;
inline constexpr char box_br = 0x13;
inline constexpr char box_right_vertical = 0x14;

} // namespace font_chars

enum struct title_selection {
   new_game,
   continue_
};

gba::keypad_status keypad;
global_save_data global_data;
[[gnu::section(".ewram")]] file_save_data save_data;
[[gnu::section(".ewram")]] file_save_data backup_data;

void wait_vblank_and_update(bool allow_soft_reset = true) noexcept
{
   while (gba::in_vblank()) {}
   while (!gba::in_vblank()) {}

   save_data.frame_count += 1;
   keypad.update();

   if (allow_soft_reset && keypad.soft_reset_buttons_held()) {
      gba::soft_reset();
   }
}

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
namespace tile_locs {

inline constexpr auto start_tileset = std::size(font) / 8;

} // namespace tile_locs

// Sets up common palettes and tiles used for most places
void common_tile_and_palette_setup()
{
   gba::bg0.set_scroll(0, 0);
   // Load tiles in
   const auto start_tileset
      = gba::dma3_copy(std::begin(font), std::end(font), gba::bg_char_loc(gba::bg_opt::char_base_block::b0));
   gba::dma3_copy(std::begin(test_tileset), std::end(test_tileset), start_tileset);
   // Load palettes in
   gba::dma3_copy(std::begin(font_pal), std::end(font_pal), gba::bg_palette_addr(0));
   gba::dma3_copy(std::begin(test_tileset_pal), std::end(test_tileset_pal), gba::bg_palette_addr(1));
   gba::dma3_copy(std::begin(font_pal), std::end(font_pal), gba::obj_palette_addr(0));
   gba::dma3_copy(std::begin(pal1::palette), std::end(pal1::palette), gba::obj_palette_addr(1));
}

void victory_screen() noexcept
{
   disable_all_sprites();
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
   gba::dma3_copy(std::begin(win_screen), std::end(win_screen), gba::bg_screen_loc(gba::bg_opt::screen_base_block::b0));
   while (true) {
      wait_vblank_and_update();

      if (keypad.a_pressed()) {
         break;
      }
   }
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
         cur_obj.set_tile_and_attr2(512 + text[i], gba::obj_attr2_options{}.set(priority::p0).set(palette_num::p0));
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
      wait_vblank_and_update(false);

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

int select_file_data(const char* prompt_finish, bool file_must_exist) noexcept
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
         const auto frame_str = frames_to_time(data.frame_count);
         char buffer[14];
         buffer[13] = '\0';
         fmt::format_to_n(buffer, std::size(buffer) - 1, "{: >13}", frame_str.data());
         write_bg0(buffer, base_loc.first / 8 - 2, base_loc.second / 8 + 3);
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
      wait_vblank_and_update();

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
         if (!file_must_exist || file_exists(arrow_loc)) {
            return arrow_loc;
         }
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
       {'{', '}', '~', '<', '=', '>', ':', ';', '@', '?', ' ', ' ', ' ', ' '}}};
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
   write_bg0("_", 1, 3);
   while (true) {
      wait_vblank_and_update();

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
      if (arrow_x < 0) {
         arrow_x = max_arrow_x;
      }
      else if (arrow_x > max_arrow_x) {
         arrow_x = 0;
      }
      if (arrow_y < 0) {
         arrow_y = max_arrow_y;
      }
      else if (arrow_y > max_arrow_y) {
         arrow_y = 0;
      }

      arrow_obj.set_loc(8 + arrow_x * 16, 6 * 8 + arrow_y * 16);

      if (keypad.a_pressed() && write_loc < max_length) {
         to_ret[write_loc] = character_mapping[arrow_y][arrow_x];
         write_loc += 1;
         write_bg0_n(to_ret.data(), write_loc, 1, 3);
         if (write_loc < max_length) {
            write_bg0("_", write_loc + 1, 3);
         }
      }

      if (keypad.b_pressed() && write_loc > 0) {
         to_ret[write_loc] = '\0';
         write_loc -= 1;
         // Rewrite spaces to clear the previous letters (16 spaces)
         write_bg0("                ", 1, 3);
         write_bg0_n(to_ret.data(), write_loc, 1, 3);
         write_bg0("_", write_loc + 1, 3);
      }

      if (keypad.start_pressed()) {
         return to_ret;
      }
   }
}

void draw_box(gba::bg_opt::screen_base_block loc, int x, int y, int width, int height) noexcept
{
   const auto draw_at = [loc](char c, int x_, int y_) { *bg_screen_loc_at(loc, x_, y_) = c; };
   // draw the top and bottom
   draw_at(font_chars::box_ul, x, y);
   draw_at(font_chars::box_bl, x, y + height - 1);
   for (int i = 0; i < width - 2; ++i) {
      draw_at(font_chars::box_top, x + 1 + i, y);
      draw_at(font_chars::box_bottom, x + 1 + i, y + height - 1);
   }
   draw_at(font_chars::box_ur, x + width - 1, y);
   draw_at(font_chars::box_br, x + width - 1, y + height - 1);
   // draw the sides
   for (int i = 0; i < height - 2; ++i) {
      draw_at(font_chars::box_left_vertical, x, y + 1 + i);
      draw_at(font_chars::box_right_vertical, x + width - 1, y + 1 + i);
   }
   // fill in the middle
   for (int y2 = 1; y2 < height - 1; ++y2) {
      for (int x2 = 1; x2 < width - 1; ++x2) {
         draw_at(' ', x + x2, y + y2);
      }
   }
}

void draw_menu(std::span<const char* const> options, int x, int y, int blank_arrow_loc = -1) noexcept
{
   const int max_len = [&]() {
      int max_so_far = 0;
      for (const auto& opt : options) {
         max_so_far = std::max(max_so_far, static_cast<int>(std::strlen(opt)));
      }
      return max_so_far;
   }();

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
   {
      using namespace gba::lcd_opt;
      gba::lcd.set_options(
         gba::preserve,
         gba::lcd_options{}
            .set(bg_mode::mode_0)
            .set(forced_blank::off)
            .set(display_bg0::on)
            // .set(display_bg1::off)
            // .set(display_bg2::off)
            // .set(display_bg3::off)
            // .set(display_obj::off)
            .set(display_window_0::off)
            .set(display_window_1::off)
            .set(display_window_obj::off)
            .set(obj_char_mapping::one_dimensional));
   }

   draw_box(gba::bg_opt::screen_base_block::b62, x, y, max_len + 3, options.size() + 2);
   for (int i = 0; i != std::ssize(options); ++i) {
      write_at(gba::bg_opt::screen_base_block::b62, options[i], x + 2, y + 1 + i);
   }

   if (blank_arrow_loc != -1) {
      *bg_screen_loc_at(gba::bg_opt::screen_base_block::b62, x + 1, y + 1 + blank_arrow_loc) = font_chars::hollow_arrow;
   }
}

int menu(
   std::span<const char* const> options,
   int x,
   int y,
   int default_choice,
   bool allow_cancel,
   bool draw_the_menu = true,
   void (*on_refresh)(int) = nullptr) noexcept
{
   if (draw_the_menu) {
      draw_menu(options, x, y);
   }

   const auto draw_at
      = [](char c, int x_, int y_) { *bg_screen_loc_at(gba::bg_opt::screen_base_block::b62, x_, y_) = c; };
   draw_at(font_chars::arrow, x + 1, y + 1 + default_choice);

   const int num_choices = options.size();
   int choice = default_choice;
   while (true) {
      wait_vblank_and_update();

      if (keypad.up_repeat()) {
         draw_at(' ', x + 1, y + 1 + choice);
         choice -= 1;
      }
      else if (keypad.down_repeat()) {
         draw_at(' ', x + 1, y + 1 + choice);
         choice += 1;
      }

      if (choice < 0) {
         choice = num_choices - 1;
      }
      else if (choice >= num_choices) {
         choice = 0;
      }

      draw_at(font_chars::arrow, x + 1, y + 1 + choice);

      if (keypad.a_pressed()) {
         return choice;
      }
      if (keypad.b_pressed() && allow_cancel) {
         return -1;
      }

      if (on_refresh != nullptr) {
         on_refresh(choice);
      }
   }
}

int load_game() noexcept
{
   const auto loc = select_file_data("to load from.", true);
   if (loc != -1) {
      global_data.last_file_select = loc;
      write_global_save_data(global_data);
      save_data = sram_read<file_save_data>(file_save_loc(loc));
   }
   return loc;
}

static_vector<const char*, max_characters> get_character_names() noexcept
{
   static_vector<const char*, max_characters> to_ret;
   for (const auto& char_ : save_data.characters) {
      if (!char_.exists) {
         break;
      }
      to_ret.push_back(char_.name.data());
   }
   return to_ret;
}

int display_stats(std::span<character> char_list, int index, bool allow_equipping) noexcept
{
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

   const auto write_bg0 = [&](const char* c, int x, int y) { write_at(gba::bg_opt::screen_base_block::b62, c, x, y); };
   // TODO: Can probably make this generic as "convert number" or something
   const auto disp_core_stat = [&](std::int32_t stat, int x, int y) {
      char buffer[10];
      buffer[9] = '\0';
      fmt::format_to_n(buffer, std::size(buffer) - 1, "{: >9}", stat);
      write_bg0(buffer, x, y);
   };
   const auto disp_mv_jmp = [&](std::uint8_t stat, int x, int y) {
      char buffer[4];
      buffer[3] = '\0';
      fmt::format_to_n(buffer, std::size(buffer) - 1, "{: >3}", stat);
      write_bg0(buffer, x, y);
   };
   const auto disp_hp_mp = [&](std::uint64_t stat, std::uint64_t max_stat, int x, int y) {
      char buffer[38];
      std::fill(std::begin(buffer), std::end(buffer), '\0');
      // If we have enough digits in the max stat split onto two lines if if'd be too large with full stat
      if (stat > 99'999'999) {
         fmt::format_to_n(buffer, std::size(buffer) - 1, "{: <17}\n /{: <15}", stat, max_stat);
      }
      else {
         char buffer2[18];
         std::fill(std::begin(buffer2), std::end(buffer2), ' ');
         buffer2[17] = '\0';
         fmt::format_to_n(buffer2, std::size(buffer2) - 1, "{}/{}", stat, max_stat);
         fmt::format_to_n(buffer, std::size(buffer) - 1, "{: <17}\n{: <17}", buffer2, "");
      }
      write_bg0(buffer, x, y);
   };

   const auto display_stats = [&](const character& char_) {
      const auto start_fill = gba::bg_screen_loc(gba::bg_opt::screen_base_block::b62);
      gba::dma3_fill(start_fill, start_fill + 32 * 32, ' ');
      gba::copy_tilemap(stats_screen, gba::bg_opt::screen_base_block::b62);
      write_bg0(char_.name.data(), 1, 1);
      write_bg0(class_data[char_.class_].name, 1, 3);
      disp_core_stat(char_.attack, 1, 8);
      disp_core_stat(char_.defense, 1, 10);
      disp_core_stat(char_.m_attack, 1, 12);
      disp_core_stat(char_.m_defense, 1, 14);
      disp_core_stat(char_.hit, 1, 16);
      disp_core_stat(char_.speed, 1, 18);
      disp_core_stat(char_.level, 16, 2);
      disp_core_stat(char_.remaining_exp(), 16, 4);
      disp_mv_jmp(char_.bases.move, 26, 8);
      disp_mv_jmp(char_.bases.jump, 26, 10);
      disp_hp_mp(char_.hp, char_.max_hp, 12, 14);
      disp_hp_mp(char_.mp, char_.max_mp, 12, 17);

      for (int i = 0; i != static_cast<int>(char_.equips.size()); ++i) {
         const auto& equip = char_.equips[i];
         if (equip.exists) {
            ;
         }
         else {
            write_bg0("(Nothing)", 13, 7 + i);
         }
      }

      // Display the sprite
      {
         using namespace gba::obj_opt;
         const auto char_obj = gba::obj{0};
         char_obj.set_attr0(
            gba::obj_attr0_options{}.set(display::enable).set(mode::normal).set(mosaic::disable).set(shape::vertical));
         char_obj.set_attr1(gba::obj_attr1_options{}.set(size::h32x16).set(vflip::disable).set(hflip::disable));
         char_obj.set_tile_and_attr2(
            0, gba::obj_attr2_options{}.set(palette_num{class_data[char_.class_].palette}).set(priority::p0));
         char_obj.set_x(27 * 8);
         char_obj.set_y(1 * 8);

         gba::dma3_copy(
            std::begin(class_data[char_.class_].sprite),
            std::end(class_data[char_.class_].sprite),
            gba::base_obj_tile_addr(0));
      }
   };

   display_stats(char_list[index]);
   if (allow_equipping) {
      int choice = 0;
      while (true) {
         constexpr std::array<const char*, 4> dummy_menu_opts{"", "", "", ""};
         choice = menu(dummy_menu_opts, 11, 6, choice, true, false);
         // TODO: Actual equipping
         if (choice == -1) {
            return -1;
         }
      }
   }
   else {
      while (true) {
         wait_vblank_and_update();

         if (keypad.b_pressed()) {
            return index;
         }
         // if (keypad.left_pressed() || keypad.l_pressed()) {
         //    index -= 1;
         // }
         // else if (keypad.right_pressed() || keypad.r_pressed()) {
         //    index += 1;
         // }

         // if (index < 0) {
         //    index = char_list.size() - 1;
         // }
         // else if (index >= static_cast<int>(char_list.size())) {
         //    index = 0;
         // }

         // display_stats(char_list[index]);
      }
   }
}

// Returns true if the map is beaten, false otherwise
bool do_map(const full_map_info& map_info) noexcept
{
   disable_all_sprites();

   // load all the enemies
   constexpr auto start_tile_offset = 24;
   static_vector<combatant, max_enemies> enemies;
   static_vector<character, max_enemies> enemy_stats;
   static_vector<combatant, max_player_units_on_map> player_units;
   for (int i = 0; i < std::ssize(map_info.base_enemies); ++i) {
      const auto& enemy = map_info.base_enemies[i];
      if (enemy.level > 0) {
         enemies.push_back({});
         enemy_stats.push_back({});
         auto& new_enemy = enemies.back();
         auto& new_stats = enemy_stats.back();
         new_stats.level = enemy.level;
         new_enemy.x = enemy.x;
         new_enemy.y = enemy.y;
         new_enemy.stats = &new_stats;
         new_stats.class_ = enemy.class_;
         new_stats.bases = class_data[enemy.class_].stats;
         new_stats.calc_stats(true);
         new_stats.fully_heal();
         const auto tile_offset = start_tile_offset + i * 8;
         new_enemy.tile_no = tile_offset;
         const auto& tile_data = class_data[enemy.class_].sprite;
         gba::dma3_copy(std::begin(tile_data), std::end(tile_data), gba::base_obj_tile_addr(0) + tile_offset * 8);
      }
   }

   constexpr auto screen_width = 240;
   constexpr auto screen_height = 160;

   constexpr auto bg0_screen_block = gba::bg_opt::screen_base_block::b62;
   constexpr auto bg1_screen_block = gba::bg_opt::screen_base_block::b60;
   constexpr auto bg2_screen_block = gba::bg_opt::screen_base_block::b58;
   constexpr auto bg3_screen_block = gba::bg_opt::screen_base_block::b56;

   const auto bg0_tiles = gba::bg_screen_loc(bg0_screen_block);
   const auto bg1_tiles = gba::bg_screen_loc(bg1_screen_block);
   const auto bg2_tiles = gba::bg_screen_loc(bg2_screen_block);
   const auto bg3_tiles = gba::bg_screen_loc(bg3_screen_block);

   constexpr auto blank_tile = tile_locs::start_tileset + 17;

   const auto tile_at = [&](const std::uint16_t* layer_data, unsigned x, unsigned y) {
      if (x >= tilemap_width || y >= tilemap_height) {
         return gba::make_tile(blank_tile, 1);
      }
      else {
         return layer_data[x + y * 60];
      }
   };

   int camera_x = 0;
   int camera_y = 0;

   const auto set_camera = [&](int x, int y) {
      camera_x = -screen_width / 2 + x * 16 + y * 16;
      camera_y = -screen_height / 2 + y * 8 - x * 8 - map_info.map->height_at(x, y) * 8 + map_info.map->y_offset * 8;
   };

   set_camera(map_info.base_x, map_info.base_y);

   const auto redraw_layer = [&](const std::uint16_t* layer_data, gba::bg_opt::screen_base_block loc) {
      // If the camera is negative we need to subtract 7 to make sure we're updating
      // the tile that's showing at the edge
      const auto offset_x = camera_x < 0 ? (camera_x - 7) / 8 : camera_x / 8;
      const auto offset_y = camera_y < 0 ? (camera_y - 7) / 8 : camera_y / 8;
      for (int y = 0; y != 21; ++y) {
         const unsigned tile_y = y + offset_y;
         for (int x = 0; x != 31; ++x) {
            const unsigned tile_x = x + offset_x;
            *bg_screen_loc_at(loc, tile_x % 32, tile_y % 32) = tile_at(layer_data, tile_x, tile_y);
         }
      }
   };

   const auto scroll_layer
      = [&](const std::uint16_t* layer_data, gba::bg_opt::screen_base_block loc, int delta_x, int delta_y) {
           const auto offset_x = camera_x < 0 ? (camera_x - 7) / 8 : camera_x / 8;
           const auto offset_y = camera_y < 0 ? (camera_y - 7) / 8 : camera_y / 8;
           // TODO: Is there a more compact way to represent this?
           if (delta_x < 0) {
              const auto x_scroll_amount = std::min(std::abs(delta_x - 7) / 8, 31);
              for (int x = 0; x != x_scroll_amount; ++x) {
                 const unsigned tile_x = x + offset_x;
                 for (int y = 0; y != 21; ++y) {
                    const unsigned tile_y = y + offset_y;
                    *bg_screen_loc_at(loc, tile_x % 32, tile_y % 32) = tile_at(layer_data, tile_x, tile_y);
                 }
              }
           }
           else if (delta_x > 0) {
              const auto x_scroll_amount = std::min((delta_x + 7) / 8, 31);
              for (int x = 31 - x_scroll_amount; x != 31; ++x) {
                 const unsigned tile_x = x + offset_x;
                 for (int y = 0; y != 21; ++y) {
                    const unsigned tile_y = y + offset_y;
                    *bg_screen_loc_at(loc, tile_x % 32, tile_y % 32) = tile_at(layer_data, tile_x, tile_y);
                 }
              }
           }
           if (delta_y < 0) {
              const auto y_scroll_amount = std::min(std::abs(delta_y - 7) / 8, 21);
              for (int y = 0; y != y_scroll_amount; ++y) {
                 const unsigned tile_y = y + offset_y;
                 for (int x = 0; x != 31; ++x) {
                    const unsigned tile_x = x + offset_x;
                    *bg_screen_loc_at(loc, tile_x % 32, tile_y % 32) = tile_at(layer_data, tile_x, tile_y);
                 }
              }
           }
           else if (delta_y > 0) {
              const auto y_scroll_amount = std::min((delta_y + 7) / 8, 21);
              for (int y = 21 - y_scroll_amount; y != 21; ++y) {
                 const unsigned tile_y = y + offset_y;
                 for (int x = 0; x != 31; ++x) {
                    const unsigned tile_x = x + offset_x;
                    *bg_screen_loc_at(loc, tile_x % 32, tile_y % 32) = tile_at(layer_data, tile_x, tile_y);
                 }
              }
           }
        };

   combatant cursor;
   character cursor_char;
   cursor.x = map_info.base_x;
   cursor.y = map_info.base_y;
   cursor.tile_no = 0;
   cursor.stats = &cursor_char;
   cursor_char.class_ = cursor_class;

   const auto display_sprite = [&](int x, int y, int obj_num, int x_adj, int y_adj) {
      const auto disp_x = x_adj - camera_x + x * 16 + y * 16;
      const auto disp_y
         = y_adj + -camera_y + y * 8 - x * 8 - map_info.map->height_at(x, y) * 8 + map_info.map->y_offset * 8;
      if (disp_x < -16 || disp_x >= screen_width || disp_y < -32 || disp_y >= screen_height) {
         gba::obj{obj_num}.set_attr0(gba::obj_attr0_options{}.set(gba::obj_opt::display::disable));
      }
      else {
         using namespace gba::obj_opt;
         gba::obj{obj_num}.set_attr0(gba::obj_attr0_options{}.set(display::enable));
         gba::obj{obj_num}.set_loc(disp_x, disp_y);
         gba::obj{obj_num}.set_attr2(gba::obj_attr2_options{}.set(
            map_info.map->sprite_is_high_priority_at(x, y) ? gba::obj_opt::priority{2} : gba::obj_opt::priority{3}));
      }
   };

   const auto init_screen = [&]() {
      {
         using namespace gba::bg_opt;
         gba::bg0.set_options(gba::bg_options{}
                                 .set(priority::p2)
                                 .set(char_base_block::b0)
                                 .set(mosaic::disable)
                                 .set(colors_palettes::c16_p16)
                                 .set(bg0_screen_block)
                                 .set(display_area_overflow::transparent)
                                 .set(screen_size::text_256x256));

         gba::bg1.set_options(gba::bg_options{}
                                 .set(priority::p3)
                                 .set(char_base_block::b0)
                                 .set(mosaic::disable)
                                 .set(colors_palettes::c16_p16)
                                 .set(bg1_screen_block)
                                 .set(display_area_overflow::transparent)
                                 .set(screen_size::text_256x256));

         gba::bg2.set_options(gba::bg_options{}
                                 .set(priority::p2)
                                 .set(char_base_block::b0)
                                 .set(mosaic::disable)
                                 .set(colors_palettes::c16_p16)
                                 .set(bg2_screen_block)
                                 .set(display_area_overflow::transparent)
                                 .set(screen_size::text_256x256));

         gba::bg3.set_options(gba::bg_options{}
                                 .set(priority::p3)
                                 .set(char_base_block::b0)
                                 .set(mosaic::disable)
                                 .set(colors_palettes::c16_p16)
                                 .set(bg3_screen_block)
                                 .set(display_area_overflow::transparent)
                                 .set(screen_size::text_256x256));
      }

      gba::dma3_fill(bg0_tiles, bg0_tiles + 32 * 32, gba::make_tile(blank_tile, 1));
      gba::dma3_fill(bg1_tiles, bg1_tiles + 32 * 32, gba::make_tile(blank_tile, 1));
      gba::dma3_fill(bg2_tiles, bg2_tiles + 32 * 32, gba::make_tile(blank_tile, 1));
      gba::dma3_fill(bg3_tiles, bg3_tiles + 32 * 32, gba::make_tile(blank_tile, 1));

      redraw_layer(map_info.map->high_priority_tiles.data(), bg2_screen_block);
      redraw_layer(map_info.map->low_priority_tiles.data(), bg3_screen_block);

      gba::bg0.set_scroll(camera_x, camera_y);
      gba::bg1.set_scroll(camera_x, camera_y);
      gba::bg2.set_scroll(camera_x, camera_y);
      gba::bg3.set_scroll(camera_x, camera_y);

      {
         using namespace gba::lcd_opt;
         gba::lcd.set_options(gba::lcd_options{}
                                 .set(bg_mode::mode_0)
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
      }

      // Set-up cursor sprite
      const auto start_base
         = gba::dma3_copy(std::begin(pal1::cursor), std::end(pal1::cursor), gba::base_obj_tile_addr(0));

      // Set-up base sprite
      gba::dma3_copy(std::begin(pal1::base), std::end(pal1::base), start_base);
      const auto base_obj = gba::obj{127};

      {
         const auto base_tile = (start_base - gba::base_obj_tile_addr(0)) / 8;
         using namespace gba::obj_opt;
         base_obj.set_attr0(gba::obj_attr0_options{}
                               .set(display::enable)
                               .set(mode::normal)
                               .set(mosaic::disable)
                               .set(shape::horizontal));
         base_obj.set_attr1(gba::obj_attr1_options{}.set(size::h32x16).set(vflip::disable).set(hflip::disable));
         base_obj.set_tile_and_attr2(base_tile, gba::obj_attr2_options{}.set(palette_num{1}).set(priority::p0));
         display_sprite(map_info.base_x, map_info.base_y, 127, 0, 0);
      }
   };

   init_screen();
   std::array<bool, 8> used_tiles;
   std::fill(used_tiles.begin(), used_tiles.end(), false);
   while (true) {
      const auto update_screen = [&]() {
         static_vector<const combatant*, max_enemies + max_player_units_on_map + 1> combatant_pointers;
         combatant_pointers.push_back(&cursor);
         for (const auto& enemy : enemies) {
            combatant_pointers.push_back(&enemy);
         }
         for (const auto& unit : player_units) {
            combatant_pointers.push_back(&unit);
         }

         const auto i8 = [](auto val) { return static_cast<std::int8_t>(val); };
         cursor.x = std::clamp(cursor.x, i8(0), i8(map_info.map->width - 1));
         cursor.y = std::clamp(cursor.y, i8(0), i8(map_info.map->height - 1));

         display_sprite(map_info.base_x, map_info.base_y, 127, 0, -1);

         const auto old_cx = camera_x;
         const auto old_cy = camera_y;

         set_camera(cursor.x, cursor.y);

         // sort combatants by y location to assign sprites to them
         std::sort(combatant_pointers.begin(), combatant_pointers.end(), [](const auto& lhs, const auto& rhs) {
            if (lhs->y == rhs->y) {
               if (lhs->x == rhs->x) {
                  // this only happens for the cursor, which we always want to be lower priority
                  return lhs->stats->class_ > rhs->stats->class_;
               }
               return lhs->x < rhs->x;
            }
            return lhs->y > rhs->y;
         });

         // Assign objects to combatants (3 per combatant, the two extra are for HP bar and damage numbers)
         for (int i = 0; i < std::ssize(combatant_pointers); ++i) {
            const auto combatant_obj = gba::obj{3 * i};
            const auto& cur_combatant = *combatant_pointers[i];
            using namespace gba::obj_opt;
            const auto palette_no = class_data[cur_combatant.stats->class_].palette;
            combatant_obj.set_tile_and_attr2(
               cur_combatant.tile_no, gba::obj_attr2_options{}.set(palette_num{palette_no}));
            if (cur_combatant.stats->class_ == cursor_class) {
               combatant_obj.set_attr0(gba::obj_attr0_options{}
                                          .set(display::enable)
                                          .set(mode::normal)
                                          .set(mosaic::disable)
                                          .set(shape::square));
               combatant_obj.set_attr1(
                  gba::obj_attr1_options{}.set(size::s32x32).set(vflip::disable).set(hflip::disable));
               display_sprite(cur_combatant.x, cur_combatant.y, 3 * i, 0, -17);
            }
            else {
               combatant_obj.set_attr0(gba::obj_attr0_options{}
                                          .set(display::enable)
                                          .set(mode::normal)
                                          .set(mosaic::disable)
                                          .set(shape::vertical));
               combatant_obj.set_attr1(
                  gba::obj_attr1_options{}.set(size::v16x32).set(vflip::disable).set(hflip::disable));
               display_sprite(cur_combatant.x, cur_combatant.y, 3 * i, 8, -24);
            }
         }

         gba::bg0.set_scroll(camera_x, camera_y);
         gba::bg1.set_scroll(camera_x, camera_y);
         gba::bg2.set_scroll(camera_x, camera_y);
         gba::bg3.set_scroll(camera_x, camera_y);

         const auto delta_x = camera_x - old_cx;
         const auto delta_y = camera_y - old_cy;
         scroll_layer(map_info.map->high_priority_tiles.data(), bg2_screen_block, delta_x, delta_y);
         scroll_layer(map_info.map->low_priority_tiles.data(), bg3_screen_block, delta_x, delta_y);
      };

      const auto matches_cursor_loc = [&](const auto& obj) { return cursor.x == obj.x && cursor.y == obj.y; };

      wait_vblank_and_update();

      if (keypad.start_pressed()) {
         return true;
      }

      if (keypad.left_repeat()) {
         cursor.x -= 1;
      }
      else if (keypad.right_repeat()) {
         cursor.x += 1;
      }
      if (keypad.up_repeat()) {
         cursor.y -= 1;
      }
      else if (keypad.down_repeat()) {
         cursor.y += 1;
      }

      if (keypad.b_pressed()) {
         const auto player_iter = std::find_if(player_units.begin(), player_units.end(), matches_cursor_loc);
         if (player_iter != player_units.end()) {
            auto& unit = *player_iter;
            if (
               unit.start_x == map_info.base_x && unit.start_y == map_info.base_y && unit.start_x == unit.x
               && unit.start_y == unit.y) {
               // Stuff them back into the base
               used_tiles[unit.index] = false;
               unit.stats->deployed = false;
               player_units.erase(player_iter);
               // Disable all sprites (the relevant ones will be enabled later)
               disable_all_sprites();
            }
            else if (unit.moved && !unit.acted) {
               unit.x = unit.start_x;
               unit.y = unit.start_y;
            }
         }
      }

      if (keypad.l_pressed()) {
         const auto enemy_iter = std::find_if(enemies.begin(), enemies.end(), matches_cursor_loc);
         const auto player_iter = std::find_if(player_units.begin(), player_units.end(), matches_cursor_loc);
         if (enemy_iter != enemies.end()) {
            gba::bg0.set_scroll(0, 0);
            const auto index = std::distance(enemies.begin(), enemy_iter);
            display_stats(enemy_stats, index, false);
            init_screen();
            gba::dma3_fill(bg0_tiles, bg0_tiles + 32 * 32, gba::make_tile(blank_tile, 1));
         }
         else if (player_iter != player_units.end()) {
            gba::bg0.set_scroll(0, 0);
            const auto index = std::distance(player_units.begin(), player_iter);
            display_stats(save_data.characters, index, false);
            init_screen();
            gba::dma3_fill(bg0_tiles, bg0_tiles + 32 * 32, gba::make_tile(blank_tile, 1));
         }
      }

      if (keypad.a_pressed()) {
         const auto player_unit_menu = [&](combatant& unit) {
            constexpr std::array<const char*, 2> options{"Move", "Attack"};
            gba::bg0.set_scroll(0, 0);
            const auto choice = menu(options, 0, 0, 0, true);
            gba::dma3_fill(bg0_tiles, bg0_tiles + 32 * 32, gba::make_tile(blank_tile, 1));
            if (choice != -1) {
               ;
            }
         };
         // const auto enemy_iter = std::find_if(enemies.begin(), enemies.end(), matches_cursor_loc);
         const auto player_iter = std::find_if(player_units.begin(), player_units.end(), matches_cursor_loc);
         // if (enemy_iter != enemies.end()) {
         //    ;
         // }
         if (player_iter != player_units.end()) {
            player_unit_menu(*player_iter);
         }
         else if (cursor.x == map_info.base_x && cursor.y == map_info.base_y) {
            static_vector<const char*, max_characters> char_names;
            static_vector<character*, max_characters> char_mapping;
            for (auto& char_ : save_data.characters) {
               if (char_.exists && !char_.deployed) {
                  char_names.push_back(char_.name.data());
                  char_mapping.push_back(&char_);
               }
            }
            if (char_names.size() > 0) {
               if (player_units.size() != max_player_units_on_map) {
                  gba::bg0.set_scroll(0, 0);
                  const auto choice = menu(char_names, 0, 0, 0, true);
                  if (choice != -1) {
                     player_units.push_back({});
                     auto& new_unit = player_units.back();
                     new_unit.x = map_info.base_x;
                     new_unit.y = map_info.base_y;
                     new_unit.start_x = map_info.base_x;
                     new_unit.start_y = map_info.base_y;
                     new_unit.stats = char_mapping[choice];
                     char_mapping[choice]->deployed = true;
                     const auto index_loc = std::find(used_tiles.begin(), used_tiles.end(), false);
                     *index_loc = true;
                     const auto index = std::distance(used_tiles.begin(), index_loc);
                     new_unit.index = index;
                     const auto& tile_data = class_data[char_mapping[choice]->class_].sprite;
                     const auto tile_no = start_tile_offset + max_enemies * 8 + index * 8;
                     const auto write_loc = gba::base_obj_tile_addr(0) + tile_no * 8;
                     new_unit.tile_no = tile_no;
                     gba::dma3_copy(std::begin(tile_data), std::end(tile_data), write_loc);
                     update_screen();
                     gba::dma3_fill(bg0_tiles, bg0_tiles + 32 * 32, gba::make_tile(blank_tile, 1));
                     player_unit_menu(new_unit);
                  }
                  gba::dma3_fill(bg0_tiles, bg0_tiles + 32 * 32, gba::make_tile(blank_tile, 1));
               }
            }
         }
      }

      update_screen();
   }
}

void main_game_loop() noexcept
{
   disable_all_sprites();
   gba::bg0.set_scroll(0, 0);
   int opt = 0;
   while (true) {
      constexpr const char* main_options[]{"Map", "Shop", "Equip", "Re-order", "New char", "Load", "Save"};
      const auto start_screen = gba::bg_screen_loc(gba::bg_opt::screen_base_block::b62);
      gba::dma3_fill(start_screen, start_screen + 32 * 32, ' ');
      opt = menu(std::span{main_options}, 0, 0, opt, false);
      switch (opt) {
      // Map
      case 0: {
         auto ch_choice = 0;
         while (ch_choice != -1) {
            // We may not end up using all 7 chapters but just in case have them ready
            constexpr const char* chapters[]{"Ch.1", "Ch.2", "Ch.3", "Ch.4", "Ch.5", "Ch.6", "Ch.7"};
            const auto get_chapter_choices = [&]() {
               return std::span{std::begin(chapters), std::begin(chapters) + 1 + save_data.chapter};
            };
            gba::dma3_fill(start_screen, start_screen + 32 * 32, ' ');
            draw_menu(main_options, 0, 0, opt);
            ch_choice = menu(get_chapter_choices(), 11, 0, ch_choice, true);
            if (ch_choice != -1) {
               auto map_choice = 0;
               while (map_choice != -1) {
                  gba::dma3_fill(start_screen, start_screen + 32 * 32, ' ');
                  draw_menu(main_options, 0, 0, opt);
                  draw_menu(get_chapter_choices(), 11, 0, ch_choice);
                  map_choice = menu(get_map_names(ch_choice, save_data), 0, 9, map_choice, true);
                  if (map_choice != -1) {
                     backup_data = save_data;
                     if (do_map(get_map_data_and_enemies(ch_choice, map_choice))) {
                        if (ch_choice == save_data.chapter && map_choice == save_data.chapter_progress) {
                           save_data.chapter_progress += 1;
                           if (save_data.chapter_progress == 9) {
                              if (save_data.chapter != num_chapters - 1) {
                                 save_data.chapter += 1;
                                 save_data.chapter_progress = 0;
                                 draw_menu(get_chapter_choices(), 11, 0, ch_choice);
                              }
                              else {
                                 // Last map completed!
                                 if (!save_data.game_completed) {
                                    save_data.game_completed = true;
                                    victory_screen();
                                    gba::dma3_fill(start_screen, start_screen + 32 * 32, ' ');
                                    common_tile_and_palette_setup();
                                    draw_menu(main_options, 0, 0, opt);
                                    draw_menu(get_chapter_choices(), 11, 0, ch_choice);
                                 }
                                 save_data.chapter_progress = 8;
                              }
                           }
                           else {
                              map_choice += 1;
                           }
                        }
                     }
                     else {
                        // Load the old data
                        save_data = backup_data;
                     }
                     gba::bg0.set_scroll(0, 0);
                  }
               }
            }
         }
      } break;
      // Shop
      case 1: break;
      // Equip
      case 2: {
         auto choice = 0;
         while (choice != -1) {
            gba::dma3_fill(start_screen, start_screen + 32 * 32, ' ');
            draw_menu(std::span{main_options}, 0, 0, opt);
            choice = menu(get_character_names(), 11, 0, choice, true);
            if (choice != -1) {
               display_stats(save_data.characters, choice, true);
               disable_all_sprites();
            }
         }
      } break;
      // Re-order
      case 3: {
         const auto names = get_character_names();
         int longest_name = 0;
         for (const auto& name : names) {
            longest_name = std::max(longest_name, static_cast<int>(std::strlen(name)));
         }
         int choice1 = 0;
         while (true) {
            gba::dma3_fill(start_screen, start_screen + 32 * 32, ' ');
            choice1 = menu(names, 0, 0, choice1, true);
            if (choice1 == -1) {
               break;
            }
            draw_menu(names, 0, 0, choice1);
            const auto choice2 = menu(names, std::min(14, longest_name + 3), 0, choice1, true);
            if (choice2 != -1) {
               std::swap(save_data.characters[choice1], save_data.characters[choice2]);
               choice1 = choice2;
            }
         }
      } break;
      // New char
      case 4: break;
      // Load
      case 5:
         load_game();
         disable_all_sprites();
         break;
      // Save
      case 6: {
         const auto loc = select_file_data("to save to.", false);
         if (loc != -1) {
            // Find the snake and copy the level to the file data
            const auto snake_it
               = std::find_if(save_data.characters.begin(), save_data.characters.end(), [](const auto& c) {
                    return c.class_ == class_ids::snake;
                 });
            save_data.file_level = snake_it->level;
            global_data.last_file_select = loc;
            write_global_save_data(global_data);
            sram_write(save_data, file_save_loc(loc));
         }
         disable_all_sprites();
      } break;
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
         if (load_game() != -1) {
            main_game_loop();
         }
      }
      else if (selection == title_selection::new_game) {
         // Set-up initial stats/characters
         save_data.frame_count = 0;
         auto& snake = save_data.characters[0];
         snake.bases = class_data[class_ids::snake].stats;
         snake.level = 1;
         snake.calc_stats(false);
         snake.fully_heal();
         snake.class_ = class_ids::snake;
         snake.name = do_naming_screen("Name a snake!", 13);
         snake.exists = true;
         for (int i = 1; i != 3; ++i) {
            auto& minion = save_data.characters[i];
            minion.bases = class_data[class_ids::snake_minion].stats;
            minion.level = 1;
            minion.calc_stats(false);
            minion.fully_heal();
            minion.class_ = class_ids::snake_minion;
            minion.exists = true;
         }
         save_data.characters[1].name = {'M', 'i', 'n', 'i', 'o', 'n', '1', '\0'};
         save_data.characters[2].name = {'M', 'i', 'n', 'i', 'o', 'n', '2', '\0'};
         save_data.file_name = snake.name;
         save_data.file_level = snake.level;
         main_game_loop();
      }
   }
}
