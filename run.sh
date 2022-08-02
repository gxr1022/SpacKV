# cd ./build
# rm CMakeCache.txt 
# cmake -DCMAKE_BUILD_TYPE=Release .. &&cmake --build .
# cmake -DCMAKE_BUILD_TYPE=Debug .. &&cmake --build .
# ./db_bench  --benchmarks="fillrandom stats" --num=16383 --value_size=262144

sudo rm -rf /mnt/pmem0/VTable.pool
./build/db_bench
