rm -rf build/super_bert_bros
mkdir build
cd build
cmake ..
cmake --build . -j$(nproc)
./super_bert_bros