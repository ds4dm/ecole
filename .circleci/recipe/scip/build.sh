#!/usr/bin/env bash

set -o errexit
set -o pipefail
set -o nounset

# FIXME Add patch files in meta.yaml to avoid modifying LDFLAGS
LDFLAGS="-lrt ${LDFLAGS}" cmake -B build -S ${SRC_DIR} \
	-D CMAKE_AR="${AR}" \
	-D CMAKE_BUILD_TYPE=Release \
	-D PARASCIP=1 \
	-D GMP=0 \
	-D ZIMPL=0 \
	-D GCG=0

cmake --build build --parallel ${CPU_COUNT} --target all libobjscip

cmake --install build --prefix ${PREFIX}
