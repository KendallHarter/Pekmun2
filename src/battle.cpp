#include "battle.hpp"

#include "classes.hpp"
#include "common_funcs.hpp"
#include "constants.hpp"
#include "gba.hpp"
#include "pathfinding.hpp"
#include "static_vector.hpp"

#include <tuple>

namespace {

constexpr auto screen_width = 240;
constexpr auto screen_height = 160;

constexpr auto bg0_screen_block = gba::bg_opt::screen_base_block::b62;
constexpr auto bg1_screen_block = gba::bg_opt::screen_base_block::b60;
constexpr auto bg2_screen_block = gba::bg_opt::screen_base_block::b58;
constexpr auto bg3_screen_block = gba::bg_opt::screen_base_block::b56;

constexpr auto blank_tile = tile_locs::start_tileset + 17;

const auto tile_at = [](const std::uint16_t* layer_data, unsigned x, unsigned y) {
   if (x >= tilemap_width || y >= tilemap_height) {
      return gba::make_tile(blank_tile, 1);
   }
   else {
      return layer_data[x + y * 60];
   }
};

const auto redraw_layer
   = [](int camera_x, int camera_y, const std::uint16_t* layer_data, gba::bg_opt::screen_base_block loc) {
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

const auto scroll_layer = [](int camera_x,
                             int camera_y,
                             const std::uint16_t* layer_data,
                             gba::bg_opt::screen_base_block loc,
                             int delta_x,
                             int delta_y) {
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

const auto fill_move_buffers = [](const full_map_info& map_info,
                                  int palette_num,
                                  const static_vector<pos, num_squares>& tiles,
                                  std::array<std::uint16_t, tilemap_width * tilemap_height>& low_priority_buffer,
                                  std::array<std::uint16_t, tilemap_width * tilemap_height>& high_priority_buffer) {
   for (const auto& tile : tiles) {
      auto& buffer
         = map_info.map->tile_is_high_priority_at(tile.x, tile.y) ? high_priority_buffer : low_priority_buffer;
      const auto tile_x = tile.x * 2 + tile.y * 2;
      const auto tile_y = map_info.map->y_offset + tile.y - tile.x - map_info.map->height_at(tile.x, tile.y);
      // This is really messy/bad but it works so oh well
      constexpr auto start_move_indic = tile_locs::start_move_indic;
      if (buffer[tile_x + tile_y * tilemap_width] == blank_tile) {
         buffer[tile_x + 0 + tile_y * tilemap_width] = gba::make_tile(start_move_indic, palette_num);
         buffer[tile_x + 1 + tile_y * tilemap_width] = gba::make_tile(start_move_indic + 1, palette_num);
      }
      else {
         buffer[tile_x + 0 + tile_y * tilemap_width] = gba::make_tile(start_move_indic + 8, palette_num);
         buffer[tile_x + 1 + tile_y * tilemap_width] = gba::make_tile(start_move_indic + 9, palette_num);
      }
      if (buffer[tile_x + 2 + tile_y * tilemap_width] == blank_tile) {
         buffer[tile_x + 2 + tile_y * tilemap_width] = gba::make_tile(start_move_indic + 2, palette_num);
         buffer[tile_x + 3 + tile_y * tilemap_width] = gba::make_tile(start_move_indic + 3, palette_num);
      }
      else {
         buffer[tile_x + 2 + tile_y * tilemap_width] = gba::make_tile(start_move_indic + 10, palette_num);
         buffer[tile_x + 3 + tile_y * tilemap_width] = gba::make_tile(start_move_indic + 11, palette_num);
      }
      if (buffer[tile_x + (tile_y + 1) * tilemap_width] == blank_tile) {
         buffer[tile_x + 0 + (tile_y + 1) * tilemap_width] = gba::make_tile(start_move_indic + 4, palette_num);
         buffer[tile_x + 1 + (tile_y + 1) * tilemap_width] = gba::make_tile(start_move_indic + 5, palette_num);
      }
      else {
         buffer[tile_x + 0 + (tile_y + 1) * tilemap_width] = gba::make_tile(start_move_indic + 10, palette_num);
         buffer[tile_x + 1 + (tile_y + 1) * tilemap_width] = gba::make_tile(start_move_indic + 11, palette_num);
      }
      if (buffer[tile_x + 2 + (tile_y + 1) * tilemap_width] == blank_tile) {
         buffer[tile_x + 2 + (tile_y + 1) * tilemap_width] = gba::make_tile(start_move_indic + 6, palette_num);
         buffer[tile_x + 3 + (tile_y + 1) * tilemap_width] = gba::make_tile(start_move_indic + 7, palette_num);
      }
      else {
         buffer[tile_x + 2 + (tile_y + 1) * tilemap_width] = gba::make_tile(start_move_indic + 8, palette_num);
         buffer[tile_x + 3 + (tile_y + 1) * tilemap_width] = gba::make_tile(start_move_indic + 9, palette_num);
      }
   }
};

} // anonymous namespace

// Returns true if the map is beaten, false otherwise
bool do_battle(file_save_data& save_data, const full_map_info& map_info) noexcept
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

   const auto bg0_tiles = gba::bg_screen_loc(bg0_screen_block);
   const auto bg1_tiles = gba::bg_screen_loc(bg1_screen_block);
   // const auto bg2_tiles = gba::bg_screen_loc(bg2_screen_block);
   // const auto bg3_tiles = gba::bg_screen_loc(bg3_screen_block);

   int camera_x = 0;
   int camera_y = 0;

   const auto set_camera = [&](int x, int y) {
      camera_x = -screen_width / 2 + x * 16 + y * 16;
      camera_y = -screen_height / 2 + y * 8 - x * 8 - map_info.map->height_at(x, y) * 8 + map_info.map->y_offset * 8;
   };

   set_camera(map_info.base_x, map_info.base_y);

   combatant cursor;
   character cursor_char;
   cursor.x = map_info.base_x;
   cursor.y = map_info.base_y;
   cursor.tile_no = 0;
   cursor.stats = &cursor_char;
   cursor_char.class_ = cursor_class;

   std::array<std::uint16_t, tilemap_width * tilemap_height> low_priority_buffer;
   std::array<std::uint16_t, tilemap_width * tilemap_height> high_priority_buffer;

   gba::dma3_fill(low_priority_buffer.begin(), low_priority_buffer.end(), blank_tile);
   gba::dma3_fill(high_priority_buffer.begin(), high_priority_buffer.end(), blank_tile);

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

      // gba::dma3_fill(bg0_tiles, bg0_tiles + 32 * 32, gba::make_tile(blank_tile, 1));
      // gba::dma3_fill(bg1_tiles, bg1_tiles + 32 * 32, gba::make_tile(blank_tile, 1));
      // gba::dma3_fill(bg2_tiles, bg2_tiles + 32 * 32, gba::make_tile(blank_tile, 1));
      // gba::dma3_fill(bg3_tiles, bg3_tiles + 32 * 32, gba::make_tile(blank_tile, 1));

      redraw_layer(camera_x, camera_y, high_priority_buffer.data(), bg0_screen_block);
      redraw_layer(camera_x, camera_y, low_priority_buffer.data(), bg1_screen_block);
      redraw_layer(camera_x, camera_y, map_info.map->high_priority_tiles.data(), bg2_screen_block);
      redraw_layer(camera_x, camera_y, map_info.map->low_priority_tiles.data(), bg3_screen_block);

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
         = gba::dma3_copy(std::begin(obj_pal1::cursor), std::end(obj_pal1::cursor), gba::base_obj_tile_addr(0));

      // Set-up base sprite
      gba::dma3_copy(std::begin(obj_pal1::base), std::end(obj_pal1::base), start_base);
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
         display_sprite(map_info.base_x, map_info.base_y, 127, 0, -1);
      }
   };

   init_screen();
   std::array<bool, 8> player_unit_present;

   std::fill(player_unit_present.begin(), player_unit_present.end(), false);
   combatant* moving_unit = nullptr;
   static_vector<pos, num_squares> move_tiles;
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

         const auto old_cx = camera_x;
         const auto old_cy = camera_y;

         set_camera(cursor.x, cursor.y);

         display_sprite(map_info.base_x, map_info.base_y, 127, 0, -1);

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
         scroll_layer(camera_x, camera_y, map_info.map->high_priority_tiles.data(), bg2_screen_block, delta_x, delta_y);
         scroll_layer(camera_x, camera_y, map_info.map->low_priority_tiles.data(), bg3_screen_block, delta_x, delta_y);
         scroll_layer(camera_x, camera_y, high_priority_buffer.data(), bg0_screen_block, delta_x, delta_y);
         scroll_layer(camera_x, camera_y, low_priority_buffer.data(), bg1_screen_block, delta_x, delta_y);
      };

      const auto matches_cursor_loc = [&](const auto& obj) { return cursor.x == obj.x && cursor.y == obj.y; };

      const auto& keypad = wait_vblank_and_update(save_data);

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

      const auto finish_or_cancel_move = [&]() {
         moving_unit = nullptr;
         gba::dma3_fill(bg0_tiles, bg0_tiles + 32 * 32, blank_tile);
         gba::dma3_fill(bg1_tiles, bg1_tiles + 32 * 32, blank_tile);
         gba::dma3_fill(low_priority_buffer.begin(), low_priority_buffer.end(), blank_tile);
         gba::dma3_fill(high_priority_buffer.begin(), high_priority_buffer.end(), blank_tile);
         update_screen();
      };

      const auto player_unit_menu = [&](combatant& unit) {
         constexpr std::array<const char*, 2> options{"Move", "Attack"};
         gba::bg0.set_scroll(0, 0);
         gba::dma3_fill(bg0_tiles, bg0_tiles + 32 * 32, blank_tile);
         const auto choice = menu(save_data, options, 0, 0, 0, true);
         gba::dma3_fill(bg0_tiles, bg0_tiles + 32 * 32, gba::make_tile(blank_tile, 1));
         if (choice == 0 && !unit.acted) {
            moving_unit = &unit;
            move_tiles = find_path(
               unit.start_x, unit.start_y, unit.stats->bases.move, unit.stats->bases.jump, map_info, enemies);
            fill_move_buffers(map_info, 2, move_tiles, low_priority_buffer, high_priority_buffer);
         }
         init_screen();
         update_screen();
      };

      if (keypad.b_pressed()) {
         const auto player_iter = std::find_if(player_units.begin(), player_units.end(), matches_cursor_loc);
         if (moving_unit != nullptr) {
            // Don't allow moving to the same panel
            if (cursor.x != moving_unit->x || cursor.y != moving_unit->y) {
               cursor.x = moving_unit->x;
               cursor.y = moving_unit->y;
               auto& unit = *moving_unit;
               finish_or_cancel_move();
               unit.moved = false;
               player_unit_menu(unit);
            }
         }
         else if (player_iter != player_units.end()) {
            auto& unit = *player_iter;
            if (
               unit.start_x == map_info.base_x && unit.start_y == map_info.base_y && unit.start_x == unit.x
               && unit.start_y == unit.y) {
               // Stuff them back into the base
               player_unit_present[unit.index] = false;
               unit.stats->deployed = false;
               player_units.erase(player_iter);
               // Disable all sprites so the sprite is no longer drawn (the relevant ones will be enabled later)
               disable_all_sprites();
            }
            else if (unit.moved && !unit.acted) {
               unit.x = unit.start_x;
               unit.y = unit.start_y;
               cursor.x = unit.start_x;
               cursor.y = unit.start_y;
               unit.moved = false;
            }
         }
      }

      else if (keypad.l_pressed()) {
         const auto enemy_iter = std::find_if(enemies.begin(), enemies.end(), matches_cursor_loc);
         const auto player_iter = std::find_if(player_units.begin(), player_units.end(), matches_cursor_loc);
         if (enemy_iter != enemies.end()) {
            gba::bg0.set_scroll(0, 0);
            const auto index = std::distance(enemies.begin(), enemy_iter);
            display_stats(save_data, enemy_stats, index, false);
            init_screen();
            gba::dma3_fill(bg0_tiles, bg0_tiles + 32 * 32, gba::make_tile(blank_tile, 1));
         }
         else if (player_iter != player_units.end()) {
            gba::bg0.set_scroll(0, 0);
            const auto index = std::distance(player_units.begin(), player_iter);
            const auto& unit = player_units[index];
            // Very hack-ish fix to show just the selected member
            const auto char_span = std::span<character>{unit.stats, unit.stats + 1};
            display_stats(save_data, char_span, index, false);
            init_screen();
            gba::dma3_fill(bg0_tiles, bg0_tiles + 32 * 32, gba::make_tile(blank_tile, 1));
         }
      }

      else if (keypad.a_pressed()) {
         const auto player_iter = std::find_if(player_units.begin(), player_units.end(), matches_cursor_loc);
         if (moving_unit != nullptr) {
            const auto move_to = pos{cursor.x, cursor.y};
            if (std::find(move_tiles.begin(), move_tiles.end(), move_to) != move_tiles.end()) {
               moving_unit->x = move_to.x;
               moving_unit->y = move_to.y;
               moving_unit->moved = true;
               auto& unit = *moving_unit;
               finish_or_cancel_move();
               if (unit.x == map_info.base_x && unit.y == map_info.base_y) {
                  // if moved into the base put the character away
                  player_unit_present[unit.index] = false;
                  unit.stats->deployed = false;
                  const auto player_iter = std::find_if(
                     player_units.begin(), player_units.end(), [&](const auto& value) { return &value == &unit; });
                  player_units.erase(player_iter);
                  disable_all_sprites();
               }
            }
         }
         else if (player_iter != player_units.end()) {
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
                  gba::dma3_fill(bg0_tiles, bg0_tiles + 32 * 32, blank_tile);
                  const auto choice = menu(save_data, char_names, 0, 0, 0, true);
                  if (choice != -1) {
                     player_units.push_back({});
                     auto& new_unit = player_units.back();
                     new_unit.x = map_info.base_x;
                     new_unit.y = map_info.base_y;
                     new_unit.start_x = map_info.base_x;
                     new_unit.start_y = map_info.base_y;
                     new_unit.stats = char_mapping[choice];
                     char_mapping[choice]->deployed = true;
                     const auto index_loc = std::find(player_unit_present.begin(), player_unit_present.end(), false);
                     *index_loc = true;
                     const auto index = std::distance(player_unit_present.begin(), index_loc);
                     new_unit.index = index;
                     const auto& tile_data = class_data[char_mapping[choice]->class_].sprite;
                     const auto tile_no = start_tile_offset + max_enemies * 8 + index * 8;
                     const auto write_loc = gba::base_obj_tile_addr(0) + tile_no * 8;
                     new_unit.tile_no = tile_no;
                     gba::dma3_copy(std::begin(tile_data), std::end(tile_data), write_loc);
                     gba::dma3_fill(bg0_tiles, bg0_tiles + 32 * 32, gba::make_tile(blank_tile, 1));
                     init_screen();
                     update_screen();
                     player_unit_menu(new_unit);
                  }
                  else {
                     gba::dma3_fill(bg0_tiles, bg0_tiles + 32 * 32, gba::make_tile(blank_tile, 1));
                  }
               }
            }
         }
      }

      update_screen();
   }
}
