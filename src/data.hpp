#ifndef DATA_HPP
#define DATA_HPP

#include <array>
#include <cstdint>

struct item {
   std::array<char, 16> name;
   std::int64_t hp;
   std::int64_t mp;
   std::int32_t attack;
   std::int32_t defense;
   std::int32_t m_attack;
   std::int32_t m_defense;
   std::int32_t speed;
   std::int32_t hit;
};

struct character {
   std::array<char, 16> name;
   std::int32_t level;
   std::int64_t hp;
   std::int64_t max_hp;
   std::int64_t mp;
   std::int64_t max_mp;
   std::int32_t attack;
   std::int32_t defense;
   std::int32_t m_attack;
   std::int32_t m_defense;
   std::int32_t speed;
   std::int32_t hit;
   std::array<item, 4> equips;
};

#endif // DATA_HPP
