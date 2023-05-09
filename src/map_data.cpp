#include "map_data.hpp"

#include <array>

constexpr std::array<std::array<const char*, 9>, 7> map_names{{{"The very first map!!!"}, {"The very second map!!"}}};

std::span<const char* const> get_map_names(int chapter, const file_save_data& data) noexcept
{
   if (chapter < data.chapter_progress) {
      const auto end_maps = std::find(map_names[chapter].begin(), map_names[chapter].end(), nullptr);
      return {map_names[chapter].begin(), end_maps};
   }
   return {map_names[chapter].begin(), map_names[chapter].begin() + data.chapter_progress + 1};
}
