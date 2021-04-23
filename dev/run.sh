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
	if [ "${dry_run}" = "false" ]; then
		"$@"
	fi
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


function build {
	execute cmake --build "${build_dir}" --parallel --target "${1-all}" "${@:2}"
}


function build_lib {
	build ecole-lib
}


function build_lib_test {
	build ecole-lib-test
}


function build_py {
	execute cmake --build "${build_dir}" --parallel --target ecole-py-ext
}


# Execute a command if rebuild is true.
function if_rebuild_then {
	if [ "${rebuild}" = "true" ]; then
		"${@}"
	fi
}


function build_doc {
	if_rebuild_then build_py
	if [ "${warnings_as_errors}" = "true" ]; then
		local sphinx_args+=("-W")
	fi
	execute python -m sphinx "${sphinx_args[@]}" -b html "${source_doc_dir}" "${build_doc_dir}/html"
}


# Return false (1) when `diff` is set and given files pattern have modifications since `rev`.
function files_have_changed {
	if [ "${diff}" = "true" ]; then
		git -C "${__ECOLE_DIR__}" diff --name-only --exit-code "${rev}" -- "${@}" > /dev/null && return 1 || return 0
	fi
}


function test_lib {
	if files_have_changed 'CMakeLists.txt' 'libecole';  then
		if_rebuild_then build_lib_test
		local extra_args=("$@")
		if [ "${fail_fast}" = "true" ]; then
			extra_args+=("--stop-on-failure ")
		fi
		build test -- ARGS="--parallel ${extra_args}"
	else
		log "Skipping ${FUNCNAME[0]} as unchanged since ${rev}."
	fi
}


function test_py {
	local -r relevant_files=('CMakeLists.txt' 'libecole/CMakeLists' 'libecole/src' 'libecole/include' 'python')
	if files_have_changed "${relevant_files}";  then
		if_rebuild_then build_py
		local extra_args=("$@")
		if [ "${fail_fast}" = "true" ]; then
			extra_args+=("--exitfirst")
		fi
		execute python -m pytest "${source_dir}/python/tests" "${extra_args[@]}"
	else
		log "Skipping ${FUNCNAME[0]} as unchanged since ${rev}."
	fi
}


function test_doc {
	if files_have_changed 'doc' 'python/src'; then
		if_rebuild_then build_doc
		if [ "${warnings_as_errors}" = "true" ]; then
			local sphinx_args+=("-W")
		fi
		execute python -m sphinx "${sphinx_args[@]}" -b linkcheck "${source_doc_dir}" "${build_doc_dir}/html"
		execute python -m sphinx "${sphinx_args[@]}" -b doctest "${source_doc_dir}" "${build_doc_dir}/html"
	else
		log "Skipping ${FUNCNAME[0]} as unchanged since ${rev}."
	fi
}


# Set update variable if it exists or throw an error.
function set_option {
	local -r key="${1}"
	local -r val="${2}"
	# If variable referenced in key is not set throw error
	if [ -z "${!key+x}" ]; then
		echo "Invalid option ${key}." 1>&2
		return 1
	# Otherwise update it's value
	else
		printf -v "${key}" "%s" "${val}"
	fi
}

# Parse command line parameters into variables.
#
# Parsing is done as follows. The output variables must be previously defined to avoid errors.
#   --some-key=val  -> some_key="val"
#   --some-key      -> some_key="true"
#   --no-some-key   -> some_key="false"
# Remaining parameter are set unchanged in a positional array.
function parse_argv {
	positional=()
	while [[ $# -gt 0 ]]; do
		local arg="${1}"
		case "${arg}" in
			--*=*)
				local key="${arg%=*}"
				local key="${key#--}"
				local key="${key//-/_}"
				set_option "${key}" "${arg#*=}"
				shift
				;;
			--no-*)
				local key="${arg#--no-}"
				local key="${key//-/_}"
				set_option "${key}" "false"
				shift
				;;
			--*)
				local key="${arg#--}"
				local key="${key//-/_}"
				set_option "${key}" "true"
				shift
				;;
			*)
				positional+=("${arg}")
				shift
				;;
		esac
	done
}


function run_main {
	# Only print the commands that would be executed.
	local dry_run="false"
	# Where the top-level CMakeLists.txt is.
	local source_dir="${__ECOLE_DIR__}"
	# Where is the CMake build folder with the test.
	local build_dir="${__ECOLE_DIR__}/build"
	# Fail if there are warnings.
	local warnings_as_errors="${__CI__}"
	# Warning for CMake itself (not compiler).
	local cmake_warnings="${__CI__}"
	# Stop on first failure
	local fail_fast="${__CI__}"
	# CMake build type.
	local build_type="Release"
	# Add build tree to PYTHONPATH.
	local fix_pythonpath="false"
	# Automaticaly rebuild libraries for tests and doc.
	local rebuild="true"
	# Test only if relevant differences have been made since the revision branch
	local rev="origin/master"
	local diff="$([ "${__CI__}" = "true" ] && printf "false" || printf "true")"

	# Parse all command line arguments.
	parse_argv "$@"

	# Where to find sphinx conf.py.
	local -r source_doc_dir="${__ECOLE_DIR__}/docs"
	# Where to output the doc.
	local -r build_doc_dir="${__ECOLE_DIR__}/build/docs"
	# Functions to execute are positional arguments with - replaced by _.
	local -r commands=("${positional[@]//-/_}")

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
