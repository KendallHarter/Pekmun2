#include "map_data.hpp"

#include "classes.hpp"

#include "generated/test_layers.hpp"
#include "generated/test_map.hpp"

#include <array>

constexpr auto start_map_tileset = 128;
constexpr auto map_palette = 1;
constexpr auto maps_per_chapter = 9;

constexpr std::array<const char*, maps_per_chapter * num_chapters> map_names{{// Chapter 1
                                                                              "The very first map!!!",
                                                                              "The very second map!!",
                                                                              "The third map!",
                                                                              "Fourth map",
                                                                              "Eh, the fifth map I guess",
                                                                              "Sixth Map",
                                                                              "Map seven",
                                                                              "Eight",
                                                                              "9",
                                                                              // Chapter 2
                                                                              "A new beginning",
                                                                              "Snake infestation!"}};

constexpr std::array<map_data, maps_per_chapter * num_chapters> map_data_array{
   {// Chapter 1
    test_map.rebase_map(start_map_tileset, map_palette),
    test_layers.rebase_map(start_map_tileset, map_palette),
    test_map.rebase_map(start_map_tileset, map_palette),
    test_layers.rebase_map(start_map_tileset, map_palette)}};

constexpr std::array<std::pair<std::int8_t, std::int8_t>, maps_per_chapter * num_chapters> base_locs{{// Chapter 1
                                                                                                      {0, 6},
                                                                                                      {13, 0},
                                                                                                      {0, 6},
                                                                                                      {13, 0}}};

constexpr std::array<std::array<enemy_base, max_enemies>, maps_per_chapter * num_chapters> map_enemies{
   {// Chapter 1
    {{{1, evil_snake, 6, 5, false}, {1, evil_snake, 6, 6, false}, {1, evil_snake, 6, 7, false}}},
    {{{2, evil_snake, 5, 5, false}}},
    {{{3, evil_snake, 5, 5, false}}}}};

std::span<const char* const> get_map_names(int chapter, const file_save_data& data) noexcept
{
   const auto start_loc = chapter * maps_per_chapter;
   const auto end_loc = (chapter + 1) * maps_per_chapter;
   if (chapter < data.chapter) {
      return {map_names.begin() + start_loc, map_names.begin() + end_loc};
   }
   const auto progress = std::min(maps_per_chapter, data.chapter_progress + 1);
   return {map_names.begin() + start_loc, map_names.begin() + start_loc + progress};
}

full_map_info get_map_data_and_enemies(int chapter, int map) noexcept
{
   const auto data_loc = chapter * maps_per_chapter + map;
   const auto loc = base_locs[data_loc];
   return full_map_info{loc.first, loc.second, &map_data_array[data_loc], std::span{map_enemies[data_loc]}};
}
