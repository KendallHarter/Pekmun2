#ifndef MAP_DATA_HPP
#define MAP_DATA_HPP

#include "gba.hpp"
#include "save_data.hpp"

#include <cstdint>
#include <span>

// TODO: Same as in the Python script
//       Is there a good way to set these programatically so it doesn't need to be manually
//       adjusted in two different places?
inline constexpr auto map_data_max_width = 16;
inline constexpr auto tilemap_width = 60;
inline constexpr auto tilemap_height = 40;
inline constexpr auto num_chapters = 1;
inline constexpr auto max_enemies = 12;
inline constexpr auto max_player_units_on_map = 8;

// This is a common enough operation to have a function for it
template<typename T, std::size_t Size>
constexpr std::array<T, Size> adjust_tile_array(std::span<const T, Size> array, int tile_adj, int palette_num) noexcept
{
   GBA_ASSERT(palette_num >= 0 && palette_num < 16);
   std::array<T, Size> to_ret;
   for (std::size_t i = 0; i < Size; ++i) {
      to_ret[i] = (array[i] + tile_adj) | (palette_num << 12);
   }
   return to_ret;
}

struct map_data {
   int width;
   int height;
   int y_offset;
   std::array<std::uint16_t, 2400> high_priority_tiles;
   std::array<std::uint16_t, 2400> low_priority_tiles;
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

   inline constexpr map_data rebase_map(int tile_adj, int palette_num) const noexcept
   {
      const auto high_p = adjust_tile_array(std::span{high_priority_tiles}, tile_adj, palette_num);
      const auto low_p = adjust_tile_array(std::span{low_priority_tiles}, tile_adj, palette_num);
      return map_data{
         width, height, y_offset, high_p, low_p, walkable_map, height_map, sprite_high_priority, tile_high_priority};
   }
};

struct enemy_base {
   std::int32_t level;
   std::uint8_t class_;
   std::int8_t x;
   std::int8_t y;
   bool is_boss;
};

struct full_map_info {
   std::int8_t base_x;
   std::int8_t base_y;
   const map_data* map;
   std::span<const enemy_base> base_enemies;
};

std::span<const char* const> get_map_names(int chapter, const file_save_data& data) noexcept;
full_map_info get_map_data_and_enemies(int chapter, int map) noexcept;

#endif // MAP_DATA_HPP
