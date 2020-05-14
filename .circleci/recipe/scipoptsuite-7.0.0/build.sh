#!/usr/bin/env bash

set -o errexit
set -o pipefail
set -o nounset

cmake -B build -S ${SRC_DIR} \
	-D CMAKE_AR="${AR}" \
	-D CMAKE_BUILD_TYPE=Release \
	-D PAPILO=OFF \
	-D PARASCIP=ON \
	-D GMP=ON \
	-D ZIMPL=0 \
	-D GCG=0

cmake --build build --parallel ${CPU_COUNT} --target all libobjscip

cmake --install build --prefix ${PREFIX}
