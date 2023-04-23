#!/bin/bash

# clangd setup
mkdir -p build
pushd build
cmake --toolchain ../cmake/devkitarm.cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=True -DCMAKE_BUILD_TYPE=Release ..
popd
ln -s build/compile_commands.json

# Hook install
./hooks/install.sh 
