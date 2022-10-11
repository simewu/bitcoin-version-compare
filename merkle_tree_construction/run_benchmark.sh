rm -rf benchmark
g++ -g --std=c++17 benchmark.cpp -o benchmark -lssl -lcrypto

./benchmark