#!/usr/bin/env bash

# Directory of this file
__DIR__="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Top of the repository in which this file is
__ECOLE_DIR__="$(git -C "${__DIR__}" rev-parse --show-toplevel)"

# If CI is defined then "true", otherwise "false" (string, not bools).
__CI__="$([ -z "${CI+x}" ] && printf "false" || printf "true")"


# Print in yellow with a new line.
function echo_yellow {
	local -r yellow="\033[1;33m"
	local -r nc="\033[0m"
	printf "${yellow}$*${nc}\n"
}


# Logging entry point
function log {
	echo_yellow "$@"
}


# Wrap calls to manage verbosity, dry-run, ...
function execute {
	log "$@"
	"$@"
}


# Add Ecole build tree to PYTHONPATH.
function add_build_to_pythonpath {
	execute export PYTHONPATH="${build_dir}/python${PYTHONPATH+:}${PYTHONPATH:-}"
}


function configure {
	local cmd=(
		"cmake" "-S" "${source_dir}" "-B" "${build_dir}"
		"-D" "ECOLE_BUILD_TESTS=ON" "-D" "ECOLE_BUILD_BENCHMARKS=ON"
	)
	[ "${cmake_warnings}" = "true" ] && cmd+=("-Wdev")
	[ "${warnings_as_errors}" = "true" ] && cmd+=("-Werror=dev" "-D" "WARNINGS_AS_ERRORS=ON")
	execute "${cmd[@]}"
	execute ln -nfs "${build_dir}/compile_commands.json"
}


function build_lib {
	execute cmake --build "${build_dir}" --parallel --target ecole-lib
}


function build_lib_test {
	execute cmake --build "${build_dir}" --parallel --target ecole-lib-test
}


function test_lib {
	if [ ${rebuild} = "true" ]; then
		build_lib_test
	fi
	execute cmake --build "${build_dir}" --target test -- ARGS="--parallel --stop-on-failure $@"
}


function build_py {
	execute cmake --build "${build_dir}" --parallel --target ecole-py-ext
}


function test_py {
	if [ ${rebuild} = "true" ]; then
		build_py
	fi
	execute python -m pytest "${source_dir}/python/tests" --exitfirst
}

function build_doc {
	if [ ${rebuild} = "true" ]; then
		build_py
	fi
	if [ ${warnings_as_errors} = "true" ]; then
		local sphinx_args+=("-W")
	fi
	execute python -m sphinx "${sphinx_args[@]}" -b html "${source_doc_dir}" "${build_doc_dir}/html"
}


function test_doc {
	if [ ${warnings_as_errors} = "true" ]; then
		local sphinx_args+=("-W")
	fi
	execute python -m sphinx "${sphinx_args[@]}" -b linkcheck "${source_doc_dir}" "${build_doc_dir}/html"
	execute python -m sphinx "${sphinx_args[@]}" -b doctest "${source_doc_dir}" "${build_doc_dir}/html"
}



function run_main {
	# Where the top-level CMakeLists.txt is.
	local source_dir="${__ECOLE_DIR__}"
	# Where is the CMake build folder with the test.
	local build_dir="${__ECOLE_DIR__}/build"
	# Fail if there are warnings.
	local warnings_as_errors="${__CI__}"
	# Warning for CMake itself (not compiler).
	local cmake_warnings="${__CI__}"
	# CMake build type.
	local build_type="Release"
	# Add build tree to P
	local fix_pythonpath="false"
	# Automaticaly rebuild libraries for tests and doc
	local rebuild="true"
	# Functions to execute
	local commands=()

	# Parse all arguments
	while [[ $# -gt 0 ]]; do
		local arg="${1}"
		case "${arg}" in
			--source-dir=*)
				source_dir="${arg#*=}"
				shift
				;;
			--build-dir=*)
				build_dir="${arg#*=}"
				shift
				;;
			--warnings-as-errors|--wae)
				warnings_as_errors="true"
				shift
				;;
			--no-warnings-as-errors|--no-wae)
				warnings_as_errors="fase"
				shift
				;;
			--cmake-warnings)
				cmake_warnings="true"
				shift
				;;
			--no-cmake-warnings)
				cmake_warnings="true"
				shift
				;;
			--build-type=*)
				build_type="${arg#*=}"
				shift
				;;
			--fix-pythonpath)
				fix_pythonpath="true"
				shift
				;;
			--no-fix-pythonpath)
				fix_pythonpath="false"
				shift
				;;
			--rebuild)
				rebuild="true"
				shift
				;;
			--no-rebuild)
				rebuild="false"
				shift
				;;
			*)
				commands+=("${arg//-/_}")
				shift
				;;
		esac
	done

	# Where to find sphinx conf.py.
	local -r source_doc_dir="${__ECOLE_DIR__}/docs"
	# Where to output the doc.
	local -r build_doc_dir="${__ECOLE_DIR__}/build/docs"

	# Fix Python ecole import
	if [ "${fix_pythonpath}" = "true" ]; then
		add_build_to_pythonpath
	fi

	for cmd in "${commands[@]}"; do
		"${cmd}"
	done
}


# Run the main when script is not being sourced
if [[ "${BASH_SOURCE[0]}" = "${0}" ]] ; then

	# Fail fast
	set -o errexit
	set -o pipefail
	set -o nounset

	run_main "$@"

fi
