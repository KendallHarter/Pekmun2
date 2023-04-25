cmake_minimum_required(VERSION 3.12)

project(pekmun2)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)

function(fix_gba_target target)
   add_custom_command(TARGET ${target} POST_BUILD
      COMMAND "${DEVKITARM_OBJCOPY}" -O binary "$<TARGET_FILE:${target}>" "$<TARGET_FILE:${target}>.gba"
      COMMAND "${DEVKITPRO_GBAFIX}" "$<TARGET_FILE:${target}>.gba"
      BYPRODUCTS "$<TARGET_FILE:${target}>.gba"
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

function(process_image input_file output_name)
   add_custom_command(
      OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/generated/${output_name}.hpp"
      COMMAND Python3::Interpreter
         "${CMAKE_CURRENT_SOURCE_DIR}/scripts/process_image.py"
         "${CMAKE_CURRENT_SOURCE_DIR}/assets/${input_file}"
         "${CMAKE_CURRENT_BINARY_DIR}/generated/${output_name}.hpp"
         "${output_name}"
      DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/assets/${input_file}"
      VERBATIM
   )

   add_custom_target(generate_${output_name} DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/generated/${output_name}.hpp")
endfunction()

# I've had some weird errors/it not working unless it's release mode so warn if so
if (NOT $<CONFIG:Release>)
   message(WARN "Building in non-release mode! This may not function correctly.")
endif()

# We're always going to be GCC based so can just add these
add_compile_options(-Wall -Wextra -Wpedantic)

add_executable(pekmun2 src/main.cpp)
fix_gba_target(pekmun2)

# Include the build directory so generated files can be accessed
target_include_directories(pekmun2 PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")

# Process images
process_image(font.png font)
add_dependencies(pekmun2 generate_font)
