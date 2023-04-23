set(CMAKE_SYSTEM_NAME generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)

set(CMAKE_C_COMPILER "$ENV{DEVKITARM}/bin/arm-none-eabi-gcc")
set(CMAKE_CXX_COMPILER "$ENV{DEVKITARM}/bin/arm-none-eabi-g++")

set(CMAKE_C_FLAGS_INIT "-mcpu=arm7tdmi -mtune=arm7tdmi -mthumb -mthumb-interwork -specs=gba.specs")
set(CMAKE_CXX_FLAGS_INIT "-fno-rtti -fno-exceptions -mcpu=arm7tdmi -mtune=arm7tdmi -mthumb -mthumb-interwork -specs=gba.specs")

set(DEVKITARM_OBJCOPY "$ENV{DEVKITARM}/bin/arm-none-eabi-objcopy")
set(DEVKITPRO_GBAFIX "$ENV{DEVKITPRO}/tools/bin/gbafix")
