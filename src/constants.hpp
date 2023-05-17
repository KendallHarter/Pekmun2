#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include "generated/font.hpp"
#include "generated/test_tileset.hpp"

#include <iterator>

// This isn't an enum struct because we want them to be chars
namespace font_chars {

inline constexpr char box_ul = 0x01;
inline constexpr char box_top = 0x02;
inline constexpr char box_ur = 0x03;
inline constexpr char box_left_vertical = 0x04;
inline constexpr char arrow = 0x05;
inline constexpr char hollow_arrow = 0x06;
inline constexpr char down_arrow = 0x07;
inline constexpr char up_arrow = 0x08;
inline constexpr char box_bl = 0x11;
inline constexpr char box_bottom = 0x12;
inline constexpr char box_br = 0x13;
inline constexpr char box_right_vertical = 0x14;

} // namespace font_chars

// Common tile locations
// TODO: There's probably a better/more flexible way to do this than hard coding stuff like this
namespace tile_locs {

inline constexpr auto start_tileset = std::size(font) / 8;
inline constexpr auto start_move_indic = start_tileset + std::size(test_tileset) / 8;

} // namespace tile_locs

#endif
