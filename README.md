# Pekmun2

## Building
```
mkdir build
cd build
cmake --toolchain ../cmake/devkitarm.cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

## CMake error
Sometimes (always?) CMake errors when first configuring the project.
This is fixed simply by trying to build it anyways.
