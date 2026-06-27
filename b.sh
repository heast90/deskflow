#!/bin/bash
set -e
BUILD_DIR=~/.deskflow/deskflow-deepin/build
cd ~/.deskflow/deskflow-deepin
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF -DBUILD_GUI=OFF
cmake --build build --target deskflow-core -j$(nproc)
echo "EXIT:$?"
echo "Output: $BUILD_DIR/bin/deskflow-core"
ls -la "$BUILD_DIR/bin/deskflow-core" 2>/dev/null
