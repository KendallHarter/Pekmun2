#ifndef DATA_HPP
#define DATA_HPP

#include <array>
#include <cstdint>
#include <random>

struct item {
   bool exists = false;
   const char* name;
   std::int64_t hp;
   std::int64_t mp;
   std::int32_t attack;
   std::int32_t defense;
   std::int32_t m_attack;
   std::int32_t m_defense;
   std::int32_t speed;
   std::int32_t hit;
};

struct base_stats {
   std::uint8_t hp;
   std::uint8_t mp;
   std::uint8_t attack;
   std::uint8_t defense;
   std::uint8_t m_attack;
   std::uint8_t m_defense;
   std::uint8_t speed;
   std::uint8_t hit;
};

struct character {
   bool exists = false;
   bool is_enemy = false;
   std::uint8_t class_;
   base_stats bases;
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

   // Calc stats DOES NOT set HP/MP; just the max values
   void calc_stats(bool randomize) noexcept
   {
      static std::minstd_rand0 prng;
      const std::array<std::pair<std::int32_t&, std::uint8_t&>, 6> stats_and_bases{
         {{attack, bases.attack},
          {defense, bases.defense},
          {m_attack, bases.m_attack},
          {m_defense, bases.m_defense},
          {speed, bases.speed},
          {hit, bases.hit}}};
      for (auto& [stat, base] : stats_and_bases) {
         const int random_factor = randomize ? std::uniform_int_distribution<int>{75, 125}(prng) : 100;
         stat = base * (level + 3) * random_factor / 100;
      }
      const std::array<std::pair<std::int64_t&, std::uint8_t&>, 2> stats_and_bases2{
         {{max_hp, bases.hp}, {max_mp, bases.mp}}};
      for (auto& [stat, base] : stats_and_bases2) {
         const int random_factor = randomize ? std::uniform_int_distribution<int>{75, 125}(prng) : 100;
         stat = base * 2 * (level + 3) * random_factor / 100;
      }
   }

   void fully_heal() noexcept
   {
      hp = max_hp;
      mp = max_mp;
   }
};

inline constexpr base_stats default_snake_base_stats{
   .hp = 10, .mp = 5, .attack = 10, .defense = 5, .m_attack = 10, .m_defense = 5, .speed = 5, .hit = 10};

inline constexpr base_stats default_snake_minion_stats{
   .hp = 7, .mp = 3, .attack = 7, .defense = 4, .m_attack = 7, .m_defense = 4, .speed = 4, .hit = 7};

#endif // DATA_HPP
