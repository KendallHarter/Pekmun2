#ifndef COMMON_FUNCS_HPP
#define COMMON_FUNCS_HPP

#include "gba.hpp"
#include "save_data.hpp"

const gba::keypad_status& wait_vblank_and_update(file_save_data& save_data, bool allow_soft_reset = true) noexcept;

void disable_all_sprites() noexcept;

volatile std::uint16_t* bg_screen_loc_at(gba::bg_opt::screen_base_block loc, int x, int y) noexcept;

void write_at_n(
   gba::bg_opt::screen_base_block loc, const char* to_write, const int n, const int x, const int y) noexcept;

void write_at(gba::bg_opt::screen_base_block loc, const char* to_write, const int x, const int y) noexcept;

void draw_box(gba::bg_opt::screen_base_block loc, int x, int y, int width, int height) noexcept;

void draw_menu(std::span<const char* const> options, int x, int y, int blank_arrow_loc = -1) noexcept;

int menu(
   file_save_data& save_data,
   std::span<const char* const> options,
   int x,
   int y,
   int default_choice,
   bool allow_cancel,
   bool draw_the_menu = true,
   void (*on_refresh)(int) = nullptr) noexcept;

int display_stats(file_save_data& save_data, std::span<character> char_list, int index, bool allow_equipping) noexcept;

#endif // COMMON_FUNCS_HPP
