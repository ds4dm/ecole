#!/usr/bin/env bash

# Fail fast
set -o errexit
set -o pipefail
set -o nounset

# Directory of this file
__DIR__="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Top of the repository in which this file is
__ECOLE_DIR__="$(git -C "${__DIR__}" rev-parse --show-toplevel)"


function main {
	# Where is the CMake build folder with the test
	local build_dir="${__ECOLE_DIR__}/build"

	# Parse all arguments
	while [[ $# -gt 0 ]]; do
		local arg="${1}"
		case "${arg}" in
			--build-dir=*)
				build_dir="${arg#*=}"
				shift
				;;
		esac
	done

	# Run CTest through the CMake target
	cmake --build "${build_dir}" --target test -- ARGS="--parallel"
}

main "$@"
