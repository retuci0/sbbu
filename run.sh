#!/bin/bash
mkdir build
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)

echo "copying assets..."
cp -r assets/ build/

cd build
./super_bert_bros