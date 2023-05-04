cmake_minimum_required(VERSION 3.12)

project(pekmun2)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)

function(clone_repo name url tag)
   execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/repos/${name})
   execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/repo_versions/)
   if (EXISTS ${CMAKE_BINARY_DIR}/repo_versions/${name})
      file(READ ${CMAKE_BINARY_DIR}/repo_versions/${name} version)
      if (${version} STREQUAL ${tag})
         return()
      endif()
   endif()
   find_package(Git)
   # gcmd = git command
   function(gcmd)
      execute_process(
         COMMAND ${GIT_EXECUTABLE} ${ARGV}
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/repos/${name}
      )
   endfunction()

   gcmd(init)
   gcmd(remote add origin ${url})
   gcmd(fetch --depth 1 origin ${tag})
   gcmd(checkout FETCH_HEAD)
   file(WRITE ${CMAKE_BINARY_DIR}/repo_versions/${name} ${tag})
endfunction()

clone_repo(libfmt git@github.com:fmtlib/fmt.git 9.1.0)
# Disable OS support
set(FMT_OS OFF)
add_subdirectory(${CMAKE_BINARY_DIR}/repos/libfmt)

function(fix_gba_target target)
   add_custom_command(TARGET ${target} POST_BUILD
      COMMAND "${DEVKITARM_OBJCOPY}" -O binary "$<TARGET_FILE:${target}>" "$<TARGET_FILE:${target}>.gba"
      COMMAND "${DEVKITPRO_GBAFIX}" "$<TARGET_FILE:${target}>.gba" > /dev/null
      BYPRODUCTS "$<TARGET_FILE:${target}>.gba"
      VERBATIM
   )
endfunction()

find_package(Python3 COMPONENTS Interpreter REQUIRED)

# Test to see if cv2 is installed and error if not
execute_process(
   COMMAND "${Python3_EXECUTABLE}" -c "import cv2"
   RESULT_VARIABLE cv2_installed
   OUTPUT_QUIET
   ERROR_QUIET
)

if (NOT ${cv2_installed} EQUAL 0)
   message(FATAL_ERROR "CV2 must be installed for Python3! See https://pypi.org/project/opencv-python/")
endif()

# Create a generated folder to put generated files
execute_process(
   COMMAND "${CMAKE_COMMAND}" -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/generated
)

macro(create_image_func func_name script_name)
   function(${func_name} input_file output_name)
      add_custom_command(
         OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/generated/${output_name}.hpp"
         COMMAND Python3::Interpreter
            "${CMAKE_CURRENT_SOURCE_DIR}/scripts/${script_name}"
            "${CMAKE_CURRENT_SOURCE_DIR}/assets/${input_file}"
            "${CMAKE_CURRENT_BINARY_DIR}/generated/${output_name}.hpp"
            "${output_name}"
         DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/assets/${input_file}"
         VERBATIM
      )

      add_custom_target(${output_name} DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/generated/${output_name}.hpp")
   endfunction()
endmacro()

create_image_func(process_image process_image.py)
create_image_func(process_fullscreen_tilemap process_fullscreen_tilemap.py)
create_image_func(process_bitmap_image process_bitmap_image.py)

function(process_map input_map output_name)
   add_custom_command(
      OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/generated/${output_name}.hpp"
      COMMAND Python3::Interpreter
         "${CMAKE_CURRENT_SOURCE_DIR}/scripts/process_map.py"
         "${input_map}"
         "${CMAKE_CURRENT_BINARY_DIR}/generated/${output_name}.hpp"
         "${output_name}"
      DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/assets/maps/${input_map}/info.json"
      DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/assets/maps/${input_map}/tiles.json"
   )

   add_custom_target(${output_name} DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/generated/${output_name}.hpp")
endfunction()

# We're always going to be GCC based so can just add these
add_compile_options(-Wall -Wextra -Wpedantic)

add_executable(pekmun2 src/main.cpp)
fix_gba_target(pekmun2)

add_executable(scrolling cpp_experiments/scrolling.cpp)
fix_gba_target(scrolling)

add_executable(layers cpp_experiments/layers.cpp)
fix_gba_target(layers)

add_executable(layers2 cpp_experiments/layers2.cpp)
fix_gba_target(layers2)

add_executable(health_bar_test cpp_experiments/health_bar_test.cpp)
fix_gba_target(health_bar_test)

# For experiments use this target
add_library(standard_includes INTERFACE)
target_include_directories(standard_includes INTERFACE "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Include the build directory so generated files can be accessed
target_link_libraries(pekmun2 PUBLIC standard_includes)
target_link_libraries(scrolling PUBLIC standard_includes)
target_link_libraries(layers PUBLIC standard_includes)
target_link_libraries(layers2 PUBLIC standard_includes)
target_link_libraries(health_bar_test PUBLIC standard_includes fmt::fmt)

process_image(font.png font)
process_image(test_tileset.png test_tileset)
process_image(snake.png snake)
process_image(move_indicator.png move_indicator)
process_image(health_bar.png health_bar)

process_fullscreen_tilemap(stats.json stats_screen)

process_bitmap_image(title.png title)

process_map(test_map test_map)

add_dependencies(pekmun2 font title)
add_dependencies(scrolling test_tileset font)
add_dependencies(layers test_tileset snake move_indicator)
add_dependencies(layers2 test_tileset snake font stats_screen test_map move_indicator)
add_dependencies(health_bar_test health_bar font)
