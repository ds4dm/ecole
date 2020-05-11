cmake -B build -S scip/examples/Queens -D CMAKE_BUILD_TYPE=Release

cmake --build build --parallel ${CPU_COUNT}

./build/queens 5
scip --version
