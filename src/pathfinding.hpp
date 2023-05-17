#ifndef PATHFINDING_HPP
#define PATHFINDING_HPP

#include "data.hpp"
#include "map_data.hpp"
#include "static_vector.hpp"

#include <span>

// Max number of tiles for move:
// 1 = 5 = 3 * 3 / 2 + 1
// 2 =    x
//       xxx
//      xxxxx
//       xxx
//        x
//   = 13
//   = 5 * 5 / 2 + 1
// 3 =    x
//       xxx
//      xxxxx
//     xxxxxxx
//      xxxxx
//       xxx
//        x
//   = 25
//   = 7 * 7 / 2 + 1
// 4 =    x
//       xxx
//      xxxxx
//     xxxxxxx
//    xxxxxxxxx
//     xxxxxxx
//      xxxxx
//       xxx
//        x
//   = 41
//   = 9 * 9 / 2 + 1
// n = (n * 2 + 1) * (n * 2 + 1) / 2 + 1

struct pos {
   std::int8_t x;
   std::int8_t y;

   friend constexpr bool operator==(const pos&, const pos&) noexcept = default;
};

inline constexpr auto num_squares = (max_move * 2 + 1) * (max_move * 2 + 1) / 2 + 1;

static_vector<pos, num_squares> find_path(
   std::int8_t x,
   std::int8_t y,
   std::int8_t range,
   std::int8_t jump,
   const full_map_info& map_info,
   std::span<const combatant> enemy_units);

#endif // PATHFINDING_HPP
