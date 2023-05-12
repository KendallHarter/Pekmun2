#ifndef CLASSES_HPP
#define CLASSES_HPP

#include <array>

#include "data.hpp"

#include "generated/pal1.hpp"

// TODO: There's probably a better way of doing this?
enum class_ids : std::uint8_t {
   snake,
   snake_minion,
   evil_snake
};

inline constexpr auto class_names = std::to_array<const char*>({"Snake", "Snake minion"});

inline constexpr auto class_sprites = std::to_array({pal1::snake, pal1::snake_minion, pal1::evil_snake});

inline constexpr auto class_palettes = std::to_array({1, 1, 1});

inline constexpr auto class_base_stats = std::to_array(
   {base_stats{
       .hp = 10,
       .mp = 5,
       .attack = 10,
       .defense = 5,
       .m_attack = 10,
       .m_defense = 5,
       .speed = 5,
       .hit = 10,
       .move = 5,
       .jump = 5},
    base_stats{
       .hp = 7,
       .mp = 3,
       .attack = 7,
       .defense = 4,
       .m_attack = 7,
       .m_defense = 4,
       .speed = 4,
       .hit = 7,
       .move = 4,
       .jump = 3},
    base_stats{
       .hp = 7,
       .mp = 3,
       .attack = 8,
       .defense = 4,
       .m_attack = 8,
       .m_defense = 4,
       .speed = 3,
       .hit = 3,
       .move = 4,
       .jump = 3}});

#endif // CLASSES_HPP
