#ifndef MAP_HPP
#define MAP_HPP

#include <cstdint>

// TODO: Same as in the Python script
//       Is there a good way to set these programatically so it doesn't need to be manually
//       adjusted in two different places?
inline constexpr auto map_data_max_width = 16;
inline constexpr auto tilemap_width = 60;
inline constexpr auto tilemap_height = 40;

struct map_data {
   int width;
   int height;
   int y_offset;
   const std::uint16_t* high_priority_tiles;
   const std::uint16_t* low_priority_tiles;
   const std::uint8_t* walkable_map;
   const std::uint8_t* height_map;
   const bool* sprite_high_priority;
   const bool* tile_high_priority;

   inline constexpr std::uint8_t walkable_at(int x, int y) const noexcept
   {
      return walkable_map[x + y * map_data_max_width];
   }

   inline constexpr std::uint8_t height_at(int x, int y) const noexcept
   {
      return height_map[x + y * map_data_max_width];
   }

   inline constexpr bool sprite_is_high_priority_at(int x, int y) const noexcept
   {
      return sprite_high_priority[x + y * map_data_max_width];
   }

   inline constexpr bool tile_is_high_priority_at(int x, int y) const noexcept
   {
      return tile_high_priority[x + y * map_data_max_width];
   }
};

#endif
