#include "common_funcs.hpp"

#include "generated/stats_screen.hpp"

#include "classes.hpp"
#include "constants.hpp"
#include "gba.hpp"

#include <iterator>
#include <limits>

const gba::keypad_status& wait_vblank_and_update(file_save_data& save_data, bool allow_soft_reset) noexcept
{
   static gba::keypad_status keypad;
   while (gba::in_vblank()) {}
   while (!gba::in_vblank()) {}

   save_data.frame_count += 1;
   keypad.update();

   if (allow_soft_reset && keypad.soft_reset_buttons_held()) {
      gba::soft_reset();
   }
   return keypad;
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

void draw_menu(std::span<const char* const> options, int x, int y, int blank_arrow_loc) noexcept
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
   file_save_data& save_data,
   std::span<const char* const> options,
   int x,
   int y,
   int default_choice,
   bool allow_cancel,
   bool draw_the_menu,
   void (*on_refresh)(int)) noexcept
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
      const auto& keypad = wait_vblank_and_update(save_data);

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

int display_stats(file_save_data& save_data, std::span<character> char_list, int index, bool allow_equipping) noexcept
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

   const auto display_char_stats = [&](const character& char_) {
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

   display_char_stats(char_list[index]);
   if (allow_equipping) {
      int choice = 0;
      while (true) {
         constexpr std::array<const char*, 4> dummy_menu_opts{"", "", "", ""};
         choice = menu(save_data, dummy_menu_opts, 11, 6, choice, true, false);
         // TODO: Actual equipping
         if (choice == -1) {
            return -1;
         }
      }
   }
   else {
      while (true) {
         const auto& keypad = wait_vblank_and_update(save_data);

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

         // display_char_stats(char_list[index]);
      }
   }
}
