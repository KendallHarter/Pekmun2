#ifndef CLASSES_HPP
#define CLASSES_HPP

#include <array>

// TODO: There's probably a better way of doing this?
enum class_ids : std::uint8_t {
   snake,
   snake_minion
};

const auto class_names = std::to_array<const char*>({"Snake", "Snake minion"});

#endif // CLASSES_HPP
