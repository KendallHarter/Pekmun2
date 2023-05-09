#include "map_data.hpp"

#include "classes.hpp"

#include "generated/test_layers.hpp"
#include "generated/test_map.hpp"

#include <array>

constexpr auto start_map_tileset = 128;
constexpr auto map_palette = 1;
constexpr auto max_enemies = 12;

constexpr std::array<std::array<const char*, 9>, num_chapters> map_names{
   {{"The very first map!!!",
     "The very second map!!",
     "The third map!",
     "Fourth map",
     "Eh, the fifth map I guess",
     "Sixth Map",
     "Map seven",
     "Eight",
     "9"},
    {"A new beginning"}}};

constexpr std::array<std::array<map_data, 9>, num_chapters> map_data_array{
   {{test_map.rebase_map(start_map_tileset, map_palette), test_layers.rebase_map(start_map_tileset, map_palette)}}};

constexpr std::array<std::array<std::array<enemy_base, max_enemies>, 9>, 7> map_enemies{
   {{{1, evil_snake, 5, 5, false}}}};

std::span<const char* const> get_map_names(int chapter, const file_save_data& data) noexcept
{
   if (chapter < data.chapter) {
      return {map_names[chapter].begin(), map_names[chapter].end()};
   }
   const auto progress = std::min(9, data.chapter_progress + 1);
   return {map_names[chapter].begin(), map_names[chapter].begin() + progress};
}

full_map_info get_map_data_and_enemies(int chapter, int map) noexcept
{
   return full_map_info{0, 0, &map_data_array[chapter][map], std::span{map_enemies[chapter][map]}};
}
