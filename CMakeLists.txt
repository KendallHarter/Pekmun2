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

# I've had some weird errors/it not working unless it's release mode so warn if so
if ($<CONFIG:Release>)
   message(WARN Building in non-release mode!  This may not function correctly.)
endif()

# We're always going to be GCC based so can just add these
add_compile_options(-Wall -Wextra -Wpedantic)

add_executable(pekmun2 src/main.cpp)
fix_gba_target(pekmun2)
