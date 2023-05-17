#include "pathfinding.hpp"

#include <array>

static_vector<pos, num_squares> find_path(
   std::int8_t x,
   std::int8_t y,
   std::int8_t range,
   std::int8_t jump,
   const full_map_info& map_info,
   std::span<const combatant> enemy_units)
{
   constexpr std::array<pos, 4> offsets{{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}};
   // also the value to return
   static_vector<pos, num_squares> seen;
   static_vector<std::int8_t, num_squares> seen_depths;
   static_vector<std::pair<pos, std::int8_t>, 4 * (max_move + 1)> frontier;
   frontier.push_back({{x, y}, 0});
   while (!frontier.empty()) {
      const auto [current, depth] = frontier.back();
      frontier.pop_back();
      // Make sure duplicates don't get into the seen list
      const auto seen_loc = std::find(seen.begin(), seen.end(), current);
      if (seen_loc != seen.end()) {
         // if this is at a lesser depth we expand it
         const auto offset = seen_loc - seen.begin();
         if (depth >= seen_depths[offset]) {
            continue;
         }
      }
      else {
         seen.push_back(current);
         seen_depths.push_back(depth);
      }
      if (depth == range) {
         continue;
      }
      for (const auto& offset : offsets) {
         const auto i8 = [](const auto& val) { return static_cast<std::int8_t>(val); };
         const auto new_pos = pos{i8(current.x + offset.x), i8(current.y + offset.y)};
         const auto matches_loc = [&](const auto& unit) { return unit.x == new_pos.x && unit.y == new_pos.y; };
         // If it's out of bound skip it
         if (new_pos.x < 0 || new_pos.y < 0 || new_pos.x >= map_info.map->width || new_pos.y >= map_info.map->height) {
            continue;
         }
         // If it's not walkable don't add the location
         if (!map_info.map->walkable_at(new_pos.x, new_pos.y)) {
            continue;
         }
         // If there's an enemy unit in the way don't add this location
         if (std::find_if(enemy_units.begin(), enemy_units.end(), matches_loc) != enemy_units.end()) {
            continue;
         }
         // If the jump is too large don't add the location
         const auto cur_height = map_info.map->height_at(current.x, current.y);
         const auto next_height = map_info.map->height_at(new_pos.x, new_pos.y);
         if (next_height - cur_height > jump) {
            continue;
         }
         // Otherwise we're good to go
         frontier.push_back({new_pos, i8(depth + 1)});
      }
   }
   return seen;
}
