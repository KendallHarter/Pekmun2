#include "gba.hpp"

#include "generated/font.hpp"
#include "generated/title.hpp"

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

title_selection title_screen()
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

      if (keypad.a_pressed()) {
         break;
      }

      arrow_loc = std::clamp(arrow_loc, 0, 1);

      gba::obj{arrow_obj}.set_y(16 + arrow_loc * 8);
   }

   return arrow_loc == 0 ? title_selection::new_game : title_selection::continue_;
}

int main()
{
   gba::set_fast_mode();
   title_screen();
}
