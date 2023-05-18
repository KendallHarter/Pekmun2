#include "map_data.hpp"

#include "classes.hpp"

#include "generated/arena.hpp"
#include "generated/cross.hpp"
#include "generated/test_layers.hpp"
#include "generated/test_map.hpp"

#include <array>

constexpr auto start_map_tileset = 128;
constexpr auto map_palette = 1;
constexpr auto maps_per_chapter = 9;

constexpr std::array<const char*, maps_per_chapter * num_chapters> map_names{{
   // Chapter 1
   "The very first map!!!",
   "The very second map!!",
   "Snakes up high in the sky",
   "Unlikely ambush",
   "Snake infestation!",
   "Faces of evil",
   "Danger near, danger far",
   "Training grounds",
   "Supreme Snake",
   // Chapter 2
   // "A new beginning",
   // "Snake infestation!"
}};

constexpr std::array<map_data, maps_per_chapter * num_chapters> map_data_array{
   {// Chapter 1
    test_map.rebase_map(start_map_tileset, map_palette),
    test_layers.rebase_map(start_map_tileset, map_palette),
    test_map.rebase_map(start_map_tileset, map_palette),
    test_layers.rebase_map(start_map_tileset, map_palette),
    cross.rebase_map(start_map_tileset, map_palette),
    test_map.rebase_map(start_map_tileset, map_palette),
    cross.rebase_map(start_map_tileset, map_palette),
    arena.rebase_map(start_map_tileset, map_palette),
    arena.rebase_map(start_map_tileset, map_palette)}};

constexpr std::array<std::pair<std::int8_t, std::int8_t>, maps_per_chapter * num_chapters> base_locs{{// Chapter 1
                                                                                                      {0, 6},
                                                                                                      {13, 0},
                                                                                                      {0, 6},
                                                                                                      {0, 6},
                                                                                                      {5, 4},
                                                                                                      {0, 2},
                                                                                                      {5, 0},
                                                                                                      {1, 1},
                                                                                                      {0, 0}}};

constexpr std::array<std::array<enemy_base, max_enemies>, maps_per_chapter * num_chapters> map_enemies{{
   // Chapter 1
   {{{1, evil_snake, 6, 5, false}, {1, evil_snake, 6, 6, false}, {1, evil_snake, 6, 7, false}}},
   {{{2, evil_snake, 5, 5, false}, {2, evil_snake, 0, 0, false}, {2, evil_snake, 0, 6, false}}},
   {{{3, evil_snake, 15, 0, false},
     {3, evil_snake, 15, 4, false},
     {3, evil_snake, 15, 8, false},
     {3, evil_snake, 15, 12, false}}},
   // 1-4
   {{{5, snail, 0, 5, false}, {5, snail, 1, 6, false}, {4, evil_snake, 1, 5, false}}},
   // 1-5
   {{{1, evil_snake, 1, 4, false},
     {1, evil_snake, 2, 4, false},
     {1, evil_snake, 3, 4, false},
     {1, evil_snake, 7, 4, false},
     {1, evil_snake, 8, 4, false},
     {1, evil_snake, 9, 4, false},
     {1, evil_snake, 5, 0, false},
     {1, evil_snake, 5, 1, false},
     {1, evil_snake, 5, 2, false},
     {1, evil_snake, 5, 6, false},
     {1, evil_snake, 5, 7, false},
     {1, evil_snake, 5, 8, false}}},
   // 1-6
   {{{8, face_, 0, 4, false},
     {8, face_, 0, 0, false},
     {7, evil_snake, 2, 1, false},
     {7, evil_snake, 2, 2, false},
     {7, evil_snake, 2, 3, false}}},
   // 1-7
   {{{10, snail, 5, 8, false},
     {10, snail, 9, 4, false},
     {10, snail, 1, 4, false},
     {10, face_, 4, 3, false},
     {10, face_, 5, 3, false},
     {10, face_, 6, 3, false}}},
   // 1-8
   {{{15, evil_snake, 2, 1, false}}},
   // 1-9
   {{2, super_snake, 3, 3, true}},
   // Chapter 2
   // {{}}
}};

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
