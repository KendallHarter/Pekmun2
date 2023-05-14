#ifndef CLASSES_HPP
#define CLASSES_HPP

#include <array>

#include "data.hpp"

#include "generated/pal1.hpp"

// TODO: There's probably a better way of doing this?
enum class_ids : std::uint8_t {
   // dummy class used for placing the cursor
   cursor_class,
   snake,
   snake_minion,
   evil_snake
};

struct class_info {
   std::uint8_t palette;
   const char* name;
   const std::uint32_t (&sprite)[64];
   base_stats stats;
};

inline constexpr class_info class_data[]{
   {1, "", pal1::snake, {}},
   {1,
    "Snake",
    pal1::snake,
    {.hp = 10,
     .mp = 5,
     .attack = 10,
     .defense = 5,
     .m_attack = 10,
     .m_defense = 5,
     .speed = 5,
     .hit = 10,
     .move = 5,
     .jump = 5}},
   {1,
    "Snake minion",
    pal1::snake_minion,
    {.hp = 7,
     .mp = 3,
     .attack = 7,
     .defense = 4,
     .m_attack = 7,
     .m_defense = 4,
     .speed = 4,
     .hit = 7,
     .move = 4,
     .jump = 3}},
   {1,
    "Evil snake",
    pal1::evil_snake,
    {.hp = 7,
     .mp = 3,
     .attack = 8,
     .defense = 4,
     .m_attack = 8,
     .m_defense = 4,
     .speed = 3,
     .hit = 3,
     .move = 4,
     .jump = 3}}};

#endif // CLASSES_HPP
