#include "gba.hpp"

#include "generated/file_select.hpp"
#include "generated/font.hpp"
#include "generated/test_tileset.hpp"
#include "generated/title.hpp"

#include "data.hpp"
#include "save_data.hpp"

#include <algorithm>
#include <array>
#include <utility>

namespace font_chars {
inline constexpr char arrow = 0x05;
}

enum struct title_selection {
   new_game,
   continue_
};

gba::keypad_status keypad;

// Not in gba.hpp because it only works for certain setups
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
   for (int i = 0; i < 128; ++i) {
      using namespace gba::obj_opt;
      gba::obj{i}.set_attr0(gba::obj_attr0_options{}.set(display::disable).set(rot_scale::disable));
   }

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

   // Arrow
   {
      using namespace gba::obj_opt;
      gba::obj{arrow_obj}.set_y_and_attr0(
         16,
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

   int arrow_loc = 0;
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

   gba::lcd.set_options(gba::lcd_options{}.set(gba::lcd_opt::forced_blank::on));

   for (int i = 0; i < 128; ++i) {
      gba::obj{i}.set_attr0(gba::obj_attr0_options{}.set(gba::obj_opt::display::disable));
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
         // TODO: File existing
         write_bg0("It exists!", base_loc.first / 8, base_loc.second / 8 + 3);
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
         }
      }
   }
}
