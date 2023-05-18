#include "generated/bg_pal2.hpp"
#include "generated/bg_pal3.hpp"
#include "generated/file_select.hpp"
#include "generated/font.hpp"
#include "generated/naming_screen.hpp"
#include "generated/obj_pal1.hpp"
#include "generated/obj_pal2.hpp"
#include "generated/stats_screen.hpp"
#include "generated/test_tileset.hpp"
#include "generated/title.hpp"
#include "generated/win_screen.hpp"

#include "battle.hpp"
#include "classes.hpp"
#include "common_funcs.hpp"
#include "constants.hpp"
#include "data.hpp"
#include "fmt/core.h"
#include "gba.hpp"
#include "map_data.hpp"
#include "pathfinding.hpp"
#include "save_data.hpp"
#include "static_vector.hpp"

#include <algorithm>
#include <array>
#include <cstring>
#include <limits>
#include <span>
#include <tuple>
#include <utility>

enum struct title_selection {
   new_game,
   continue_
};

global_save_data global_data;
[[gnu::section(".ewram")]] file_save_data save_data;
[[gnu::section(".ewram")]] file_save_data backup_data;

// Sets up common palettes and tiles used for most places
void common_tile_and_palette_setup()
{
   gba::bg0.set_scroll(0, 0);
   // Load tiles in
   const auto start_tileset
      = gba::dma3_copy(std::begin(font), std::end(font), gba::bg_char_loc(gba::bg_opt::char_base_block::b0));
   const auto start_move_indic = gba::dma3_copy(std::begin(test_tileset), std::end(test_tileset), start_tileset);
   gba::dma3_copy(std::begin(bg_pal2::move_indicator), std::end(bg_pal2::move_indicator), start_move_indic);
   // Load palettes in
   gba::dma3_copy(std::begin(font_pal), std::end(font_pal), gba::bg_palette_addr(0));
   gba::dma3_copy(std::begin(test_tileset_pal), std::end(test_tileset_pal), gba::bg_palette_addr(1));
   gba::dma3_copy(std::begin(bg_pal2::palette), std::end(bg_pal2::palette), gba::bg_palette_addr(2));
   gba::dma3_copy(std::begin(bg_pal3::palette), std::end(bg_pal3::palette), gba::bg_palette_addr(3));
   gba::dma3_copy(std::begin(font_pal), std::end(font_pal), gba::obj_palette_addr(0));
   gba::dma3_copy(std::begin(obj_pal1::palette), std::end(obj_pal1::palette), gba::obj_palette_addr(1));
   gba::dma3_copy(std::begin(obj_pal2::palette), std::end(obj_pal2::palette), gba::obj_palette_addr(2));
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
      const auto& keypad = wait_vblank_and_update(save_data);

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
      const auto& keypad = wait_vblank_and_update(save_data, false);

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
      const auto& keypad = wait_vblank_and_update(save_data);

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
      const auto& keypad = wait_vblank_and_update(save_data);

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

void main_game_loop() noexcept
{
   disable_all_sprites();
   gba::bg0.set_scroll(0, 0);
   int opt = 0;
   int enemy_strength = 0;
   while (true) {
      constexpr const char* main_options[]{"Map", "Shop", "Equip", "Re-order", "New char", "Load", "Save"};
      const auto start_screen = gba::bg_screen_loc(gba::bg_opt::screen_base_block::b62);
      gba::dma3_fill(start_screen, start_screen + 32 * 32, ' ');
      opt = menu(save_data, std::span{main_options}, 0, 0, opt, false);
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
            const auto adjust_enemy_strength = [&](int, const gba::keypad_status& keypad) {
               if (keypad.left_repeat()) {
                  enemy_strength -= 1;
               }
               else if (keypad.right_repeat()) {
                  enemy_strength += 1;
               }
               if (keypad.l_pressed()) {
                  enemy_strength -= 10;
               }
               else if (keypad.r_pressed()) {
                  enemy_strength += 10;
               }
               enemy_strength = std::clamp(enemy_strength, 0, 999);
               draw_box(gba::bg_opt::screen_base_block::b62, 11, 3, 16, 4);
               char buffer[50];
               std::fill(std::begin(buffer), std::end(buffer), '\0');
               fmt::format_to_n(buffer, std::size(buffer) - 1, "Enemy Strength\n{: >14}", enemy_strength + 1);
               write_at(gba::bg_opt::screen_base_block::b62, buffer, 11 + 1, 4);
            };
            gba::dma3_fill(start_screen, start_screen + 32 * 32, ' ');
            draw_menu(main_options, 0, 0, opt);
            ch_choice = menu(save_data, get_chapter_choices(), 11, 0, ch_choice, true);
            if (ch_choice != -1) {
               auto map_choice = 0;
               while (map_choice != -1) {
                  gba::dma3_fill(start_screen, start_screen + 32 * 32, ' ');
                  draw_menu(main_options, 0, 0, opt);
                  draw_menu(get_chapter_choices(), 11, 0, ch_choice);
                  map_choice = menu(
                     save_data,
                     get_map_names(ch_choice, save_data),
                     0,
                     9,
                     map_choice,
                     true,
                     true,
                     adjust_enemy_strength);
                  if (map_choice != -1) {
                     backup_data = save_data;
                     if (do_battle(save_data, get_map_data_and_enemies(ch_choice, map_choice), enemy_strength)) {
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
            choice = menu(save_data, get_character_names(), 11, 0, choice, true);
            if (choice != -1) {
               display_stats(save_data, save_data.characters, choice, true);
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
            choice1 = menu(save_data, names, 0, 0, choice1, true);
            if (choice1 == -1) {
               break;
            }
            draw_menu(names, 0, 0, choice1);
            const auto choice2 = menu(save_data, names, std::min(14, longest_name + 3), 0, choice1, true);
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
