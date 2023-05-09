#ifndef CLASSES_HPP
#define CLASSES_HPP

#include <array>

#include "generated/snake.hpp"

// TODO: There's probably a better way of doing this?
enum class_ids : std::uint8_t {
   snake_,
   snake_minion
};

inline constexpr auto class_names = std::to_array<const char*>({"Snake", "Snake minion"});

inline constexpr auto class_sprites = std::to_array({snake, snake});

// TODO: These need to be indexed instead of absolute references
inline constexpr auto class_palettes = std::to_array({snake_pal, snake_pal});

#endif // CLASSES_HPP
