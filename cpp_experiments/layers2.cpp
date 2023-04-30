#include "gba.hpp"

#include "generated/font.hpp"
#include "generated/move_indicator.hpp"
#include "generated/snake.hpp"
#include "generated/stats_screen.hpp"
#include "generated/test_map.hpp"
#include "generated/test_tileset.hpp"

#include <charconv>

// This only works for the specific bg size/settings we use here so it isn't in gba.hpp
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
   using namespace gba::bg_opt;
   using namespace gba::lcd_opt;

   // Initital tile & palette setup (tiles and palettes are the same throughout the program)
   gba::dma3_copy(std::begin(font_pal), std::end(font_pal), gba::bg_palette_addr(0));
   gba::dma3_copy(std::begin(test_tileset_pal), std::end(test_tileset_pal), gba::bg_palette_addr(1));
   gba::dma3_copy(std::begin(move_indicator_pal), std::end(move_indicator_pal), gba::bg_palette_addr(2));
   const auto tileset_begin = gba::dma3_copy(std::begin(font), std::end(font), gba::bg_char_loc(char_base_block::b0));
   const auto move_indic_begin = gba::dma3_copy(std::begin(test_tileset), std::end(test_tileset), tileset_begin);
   gba::dma3_copy(std::begin(move_indicator), std::end(move_indicator), move_indic_begin);
   gba::dma3_copy(std::begin(snake), std::end(snake), gba::obj_tile_addr(0));
   gba::dma3_copy(std::begin(snake_pal), std::end(snake_pal), gba::obj_palette_addr(0));

   constexpr auto tileset_base = gba::num_tiles(font);
   constexpr auto move_indic_base = tileset_base + gba::num_tiles(test_tileset);

   gba::keypad_status keypad;

   const auto display_stats = [&]() {
      const auto snake_obj = gba::obj{0};

      gba::bg0.set_options(gba::bg_options{}
                              .set(priority::p0)
                              .set(char_base_block::b0)
                              .set(mosaic::disable)
                              .set(colors_palettes::c16_p16)
                              .set(screen_base_block::b62)
                              .set(display_area_overflow::transparent)
                              .set(screen_size::text_256x256));
      gba::copy_tilemap(stats_screen, screen_base_block::b62);

      using namespace gba::obj_opt;

      for (int i = 0; i < 128; ++i) {
         gba::obj{i}.set_attr0(gba::obj_attr0_options{}.set(display::disable));
      }

      snake_obj.set_attr0(gba::obj_attr0_options{}
                             .set(display::enable)
                             .set(mode::normal)
                             .set(gba::obj_opt::mosaic::disable)
                             .set(shape::vertical));

      snake_obj.set_attr1(gba::obj_attr1_options{}.set(size::h32x16));

      // Put in the upper-right corner box
      snake_obj.set_x(27 * 8);
      snake_obj.set_y(1 * 8);

      const auto write_bg0 = [&](const char* c, int x, int y) { write_at(screen_base_block::b62, c, x, y); };

      // Name and class
      write_bg0("Joe the Snake", 1, 1);
      write_bg0("Snake", 1, 3);

      // Equipment
      write_bg0("(Nothing)", 13, 7);
      write_bg0("(Nothing)", 13, 8);
      write_bg0("(Nothing)", 13, 9);
      write_bg0("(Nothing)", 13, 10);

      // Attack, Defense, M. Attack, M. Defense, Hit, Speed
      write_bg0("       20", 1, 8);
      write_bg0("        7", 1, 10);
      write_bg0("        2", 1, 12);
      write_bg0("        5", 1, 14);
      write_bg0("       15", 1, 16);
      write_bg0("        8", 1, 18);

      // HP, MP
      write_bg0("20/20", 12, 14);
      write_bg0("8/8", 12, 17);

      // Level, EXP
      write_bg0("1", 24, 2);
      write_bg0("0", 24, 4);

      gba::bg0.set_scroll(0, 0);

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

      while (true) {
         while (gba::in_vblank()) {}
         while (!gba::in_vblank()) {}

         keypad.update();

         if (keypad.a_pressed()) {
            return;
         }
      }
   };

   const auto map = [&]() {
      constexpr auto high_priority = gba::adjust_tile_array(test_map_high_priority_tiles, tileset_base, 1);
      constexpr auto low_priority = gba::adjust_tile_array(test_map_low_priority_tiles, tileset_base, 1);
      int camera_x = 0;
      int camera_y = 0;

      constexpr auto blank_tile = tileset_base + 17;

      std::array<std::uint16_t, tilemap_width * tilemap_height> low_priority_buffer;
      std::array<std::uint16_t, tilemap_width * tilemap_height> high_priority_buffer;

      gba::dma3_fill(low_priority_buffer.begin(), low_priority_buffer.end(), gba::make_tile(blank_tile, 1));
      gba::dma3_fill(high_priority_buffer.begin(), high_priority_buffer.end(), gba::make_tile(blank_tile, 1));

      const auto tile_at = [&](const std::uint16_t* layer_data, unsigned x, unsigned y) {
         if (x >= 60 || y >= 40) {
            return gba::make_tile(blank_tile, 1);
         }
         else {
            return layer_data[x + y * 60];
         }
      };

      const auto redraw_layer = [&](const std::uint16_t* layer_data, screen_base_block loc) {
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

      const auto scroll_layer = [&](const std::uint16_t* layer_data, screen_base_block loc) {
         const auto offset_x = camera_x < 0 ? (camera_x - 7) / 8 : camera_x / 8;
         const auto offset_y = camera_y < 0 ? (camera_y - 7) / 8 : camera_y / 8;
         for (auto y : {0, 20}) {
            const unsigned tile_y = y + offset_y;
            for (auto x = 0; x != 31; ++x) {
               const unsigned tile_x = x + offset_x;
               *bg_screen_loc_at(loc, tile_x % 32, tile_y % 32) = tile_at(layer_data, tile_x, tile_y);
            }
         }
         for (auto x : {0, 30}) {
            const unsigned tile_x = x + offset_x;
            for (auto y = 0; y != 21; ++y) {
               const unsigned tile_y = y + offset_y;
               *bg_screen_loc_at(loc, tile_x % 32, tile_y % 32) = tile_at(layer_data, tile_x, tile_y);
            }
         }
      };

      constexpr auto bg0_screen_block = screen_base_block::b62;
      constexpr auto bg1_screen_block = screen_base_block::b60;
      constexpr auto bg2_screen_block = screen_base_block::b58;
      constexpr auto bg3_screen_block = screen_base_block::b56;

      const auto bg0_tiles = gba::bg_screen_loc(bg0_screen_block);
      const auto bg1_tiles = gba::bg_screen_loc(bg1_screen_block);
      const auto bg2_tiles = gba::bg_screen_loc(bg2_screen_block);
      const auto bg3_tiles = gba::bg_screen_loc(bg3_screen_block);

      const auto init_screen = [&]() {
         gba::lcd.set_options(gba::lcd_options{}.set(forced_blank::on));

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

         // clear the backgrounds
         gba::dma3_fill(bg0_tiles, bg0_tiles + 32 * 32, gba::make_tile(blank_tile, 1));
         gba::dma3_fill(bg1_tiles, bg1_tiles + 32 * 32, gba::make_tile(blank_tile, 1));
         gba::dma3_fill(bg2_tiles, bg2_tiles + 32 * 32, gba::make_tile(blank_tile, 1));
         gba::dma3_fill(bg3_tiles, bg3_tiles + 32 * 32, gba::make_tile(blank_tile, 1));

         redraw_layer(high_priority_buffer.data(), bg0_screen_block);
         redraw_layer(low_priority_buffer.data(), bg1_screen_block);
         redraw_layer(high_priority.data(), bg2_screen_block);
         redraw_layer(low_priority.data(), bg3_screen_block);

         gba::bg0.set_scroll(camera_x, camera_y);
         gba::bg1.set_scroll(camera_x, camera_y);
         gba::bg2.set_scroll(camera_x, camera_y);
         gba::bg3.set_scroll(camera_x, camera_y);

         gba::lcd.set_options(gba::lcd_options{}
                                 .set(bg_mode::mode_0)
                                 .set(forced_blank::off)
                                 .set(display_bg0::on)
                                 .set(display_bg1::on)
                                 .set(display_bg2::on)
                                 .set(display_bg3::on)
                                 .set(display_obj::off)
                                 .set(display_window_0::off)
                                 .set(display_window_1::off)
                                 .set(display_window_obj::off)
                                 .set(obj_char_mapping::one_dimensional));
      };

      bool move_showing = false;
      init_screen();
      while (true) {
         while (gba::in_vblank()) {}
         while (!gba::in_vblank()) {}

         keypad.update();
         const auto change_amount = keypad.r_held() ? 8 : 1;
         if (keypad.left_held()) {
            camera_x -= change_amount;
         }
         else if (keypad.right_held()) {
            camera_x += change_amount;
         }
         if (keypad.up_held()) {
            camera_y -= change_amount;
         }
         else if (keypad.down_held()) {
            camera_y += change_amount;
         }

         if (keypad.a_pressed()) {
            if (move_showing) {
               // clear out the move location
               gba::dma3_fill(low_priority_buffer.begin(), low_priority_buffer.end(), gba::make_tile(blank_tile, 1));
               gba::dma3_fill(high_priority_buffer.begin(), high_priority_buffer.end(), gba::make_tile(blank_tile, 1));
               gba::dma3_fill(bg0_tiles, bg0_tiles + 32 * 32, gba::make_tile(blank_tile, 1));
               gba::dma3_fill(bg1_tiles, bg1_tiles + 32 * 32, gba::make_tile(blank_tile, 1));
               move_showing = false;
            }
            else {
               constexpr auto start_indic = move_indic_base;
               for (int y = 0; y < test_map.height; ++y) {
                  for (int x = 0; x < test_map.width; ++x) {
                     if (test_map.walkable_at(x, y)) {
                        auto& buffer
                           = test_map.tile_is_high_priority_at(x, y) ? high_priority_buffer : low_priority_buffer;
                        const auto tile_x = x * 2 + y * 2;
                        const auto tile_y = test_map.y_offset + y - x - test_map.height_at(x, y);
                        // TODO: This still feels really messy
                        if (buffer[tile_x + tile_y * tilemap_width] == gba::make_tile(blank_tile, 1)) {
                           buffer[tile_x + 0 + tile_y * tilemap_width] = gba::make_tile(start_indic, 2);
                           buffer[tile_x + 1 + tile_y * tilemap_width] = gba::make_tile(start_indic + 1, 2);
                        }
                        else {
                           buffer[tile_x + 0 + tile_y * tilemap_width] = gba::make_tile(start_indic + 8, 2);
                           buffer[tile_x + 1 + tile_y * tilemap_width] = gba::make_tile(start_indic + 9, 2);
                        }
                        buffer[tile_x + 2 + tile_y * tilemap_width] = gba::make_tile(start_indic + 2, 2);
                        buffer[tile_x + 3 + tile_y * tilemap_width] = gba::make_tile(start_indic + 3, 2);
                        if (buffer[tile_x + (tile_y + 1) * tilemap_width] == gba::make_tile(blank_tile, 1)) {
                           buffer[tile_x + 0 + (tile_y + 1) * tilemap_width] = gba::make_tile(start_indic + 4, 2);
                           buffer[tile_x + 1 + (tile_y + 1) * tilemap_width] = gba::make_tile(start_indic + 5, 2);
                        }
                        else {
                           buffer[tile_x + 0 + (tile_y + 1) * tilemap_width] = gba::make_tile(start_indic + 10, 2);
                           buffer[tile_x + 1 + (tile_y + 1) * tilemap_width] = gba::make_tile(start_indic + 11, 2);
                        }
                        buffer[tile_x + 2 + (tile_y + 1) * tilemap_width] = gba::make_tile(start_indic + 6, 2);
                        buffer[tile_x + 3 + (tile_y + 1) * tilemap_width] = gba::make_tile(start_indic + 7, 2);
                     }
                  }
               }
               redraw_layer(high_priority_buffer.data(), bg0_screen_block);
               redraw_layer(low_priority_buffer.data(), bg1_screen_block);
               move_showing = true;
            }
         }

         if (keypad.l_pressed()) {
            display_stats();
            init_screen();
         }

         gba::bg0.set_scroll(camera_x, camera_y);
         gba::bg1.set_scroll(camera_x, camera_y);
         gba::bg2.set_scroll(camera_x, camera_y);
         gba::bg3.set_scroll(camera_x, camera_y);

         scroll_layer(high_priority_buffer.data(), bg0_screen_block);
         scroll_layer(low_priority_buffer.data(), bg1_screen_block);
         scroll_layer(high_priority.data(), bg2_screen_block);
         scroll_layer(low_priority.data(), bg3_screen_block);
      }
   };

   while (true) {
      map();
   }
}
