#ifndef SAVE_DATA_HPP
#define SAVE_DATA_HPP

#include "data.hpp"
#include "gba.hpp"

#include <fmt/core.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>

struct global_save_data {
   std::int8_t last_file_select;
};

constexpr std::array<char, 4> file_exists_text{{'P', 'e', 'k', '2'}};

inline constexpr int max_characters = 16;
inline constexpr int max_items = 48;

struct file_save_data {
   std::array<char, 4> exists_text = file_exists_text;
   bool game_completed = false;
   std::uint16_t enemy_strength = 0;
   std::array<character, max_characters> characters;
   std::array<item, max_items> items;
   std::uint8_t chapter = 0;
   std::uint8_t chapter_progress = 0;
   std::array<char, 16> file_name;
   std::int32_t file_level;
   std::uint64_t frame_count;
   std::uint64_t money = 0;
};

struct file_load_data {
   decltype(file_save_data::file_name) file_name;
   std::int32_t file_level;
   std::uint64_t frame_count;
};

// SRAM can only be accessed with 8-bit reads/writes so we need specialized function for access
template<typename T>
inline void sram_read(T& value, volatile std::uint8_t* loc) noexcept
{
   const auto ptr = reinterpret_cast<std::uint8_t*>(&value);
   for (int i = 0; i < static_cast<int>(sizeof(value)); ++i) {
      ptr[i] = loc[i];
   }
}

template<typename T>
inline T sram_read(volatile std::uint8_t* loc) noexcept
{
   T read_into;
   sram_read(read_into, loc);
   return read_into;
}

template<typename T>
inline void sram_write(const T& val, volatile std::uint8_t* loc) noexcept
{
   const auto ptr = reinterpret_cast<const std::uint8_t*>(&val);
   for (int i = 0; i < static_cast<int>(sizeof(val)); ++i) {
      loc[i] = ptr[i];
   }
}

inline volatile std::uint8_t* file_save_loc(int file_no) noexcept
{
   return gba::sram_addr() + sizeof(global_save_data) + file_no * sizeof(file_save_data);
}

inline bool file_exists(int file_no) noexcept
{
   const auto base_addr = file_save_loc(file_no);
   const auto exists_offset = offsetof(file_save_data, exists_text);
   const auto header = sram_read<decltype(file_save_data::exists_text)>(base_addr + exists_offset);
   return header == file_exists_text;
}

inline file_load_data get_file_load_data(int file_no) noexcept
{
   file_load_data to_ret;
   const auto base_addr = file_save_loc(file_no);
   sram_read(to_ret.file_name, base_addr + offsetof(file_save_data, file_name));
   sram_read(to_ret.file_level, base_addr + offsetof(file_save_data, file_level));
   sram_read(to_ret.frame_count, base_addr + offsetof(file_save_data, frame_count));
   return to_ret;
}

inline std::array<char, 14> frames_to_time(std::uint64_t frame_count) noexcept
{
   std::array<char, 14> to_ret;
   std::fill(to_ret.begin(), to_ret.end(), '\0');
   // GBA runs at 59.7275Hz; this is equivalent to dividing by that value
   const auto seconds = frame_count * 10000 / 597275;
   const auto minutes = seconds / 60;
   const auto hours = minutes / 60;
   fmt::format_to_n(to_ret.data(), std::size(to_ret) - 1, "{}:{:0>2}:{:0>2}", hours, minutes % 60, seconds % 60);
   return to_ret;
}

inline global_save_data get_global_save_data() noexcept { return sram_read<global_save_data>(gba::sram_addr()); }

inline void write_global_save_data(const global_save_data& data) noexcept { sram_write(data, gba::sram_addr()); }

// Ensure there's enough SRAM to save everything
// (Using SRAM larger than 0x7FFF requires special commands and stuff)
static_assert(sizeof(global_save_data) + 4 * sizeof(file_save_data) <= 0x7FFF);

#endif // SAVE_DATA_HPP
