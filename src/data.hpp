#ifndef DATA_HPP
#define DATA_HPP

#include <array>
#include <cmath>
#include <cstdint>
#include <random>

constexpr int max_move = 15;

struct item {
   bool exists = false;
   std::uint8_t item_name;
   std::uint8_t move;
   std::uint8_t jump;
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
   std::int8_t move;
   std::int8_t jump;
};

struct character {
   bool exists = false;
   bool deployed = false;
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
   std::int32_t exp = 0;

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
         const int random_factor = randomize ? std::uniform_int_distribution<int>{950, 1050}(prng) : 1000;
         stat = base * std::pow(level + 3, 1.05) * random_factor / 1000;
      }
      const std::array<std::pair<std::int64_t&, std::uint8_t&>, 2> stats_and_bases2{
         {{max_hp, bases.hp}, {max_mp, bases.mp}}};
      for (auto& [stat, base] : stats_and_bases2) {
         const int random_factor = randomize ? std::uniform_int_distribution<int>{950, 1050}(prng) : 1000;
         stat = base * 2 * std::pow(level + 3, 1.25) * random_factor / 1000;
      }
   }

   void fully_heal() noexcept
   {
      hp = max_hp;
      mp = max_mp;
   }

   std::int32_t remaining_exp() const noexcept
   {
      const auto exp_needed = std::min(50 * static_cast<std::int64_t>(level), static_cast<std::int64_t>(999'999'999));
      return exp_needed - exp;
   }
};

struct combatant {
   std::int8_t x;
   std::int8_t y;
   std::int8_t start_x;
   std::int8_t start_y;
   std::int8_t index;
   bool moved = false;
   bool acted = false;
   int tile_no;
   character* stats;
};

#endif // DATA_HPP
