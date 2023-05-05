#ifndef SAVE_DATA_HPP
#define SAVE_DATA_HPP

#include "data.hpp"
#include "gba.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

struct global_save_data {
   std::int8_t last_file_select;
};

struct file_save_data {
   std::array<char, 4> exists_text;
   std::array<character, 32> characters;
   std::array<item, 64> items;
   std::uint32_t progress;
   std::array<char, 16> file_name;
};

constexpr std::array<char, 4> file_exists_text{{'P', 'e', 'k', '2'}};

// SRAM can only be accessed with 8-bit reads/writes so we need specialized function for access
template<typename T>
inline T sram_read(volatile std::uint8_t* loc) noexcept
{
   T read_into;
   const auto ptr = reinterpret_cast<std::uint8_t*>(&read_into);
   for (int i = 0; i < static_cast<int>(sizeof(read_into)); ++i) {
      ptr[i] = loc[i];
   }
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

inline bool file_exists(int num) noexcept
{
   const auto base_addr = gba::sram_addr() + sizeof(global_save_data) + num * sizeof(file_save_data);
   const auto exists_offset = offsetof(file_save_data, exists_text);
   const auto header = sram_read<decltype(file_save_data::exists_text)>(base_addr + exists_offset);
   return header == file_exists_text;
}

inline global_save_data get_global_save_data() noexcept { return sram_read<global_save_data>(gba::sram_addr()); }

inline void write_global_save_data(const global_save_data& data) noexcept { sram_write(data, gba::sram_addr()); }

// Ensure there's enough SRAM to save everything
static_assert(sizeof(global_save_data) + 4 * sizeof(file_save_data) <= 0xFFFF);

#endif // SAVE_DATA_HPP
