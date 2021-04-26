#!/usr/bin/env bash


# Directory of this file
__DIR__="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Top of the repository in which this file is
__ECOLE_DIR__="$(git -C "${__DIR__:?}" rev-parse --show-toplevel)"

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
	local extra_args=("$@")
	if [ "${cmake_warnings}" = "true" ]; then
		extra_args+=("-Wdev")
	fi
	if [ "${warnings_as_errors}" = "true" ]; then
		extra_args+=("-Werror=dev" "-D" "WARNINGS_AS_ERRORS=ON")
	fi
	execute cmake -S "${source_dir}" -B "${build_dir}" -D ECOLE_BUILD_TESTS=ON -D ECOLE_BUILD_BENCHMARKS=ON "${extra_args[@]}"
	execute ln -nfs "${build_dir}/compile_commands.json"
}


function build {
	execute cmake --build "${build_dir}" --parallel --target "${1-all}" "${@:2}"
}


function build_lib {
	build ecole-lib "$@"
}


function build_lib_test {
	build ecole-lib-test "$@"
}


function build_py {
	build ecole-py-ext "$@"
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
	execute python -m sphinx "${sphinx_args[@]}" -b html "${source_doc_dir}" "${build_doc_dir}/html" "$@"
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
		local extra_args=("$@")
		if [ "${warnings_as_errors}" = "true" ]; then
			extra_args+=("-W")
		fi
		execute python -m sphinx "${extra_args[@]}" -b linkcheck "${source_doc_dir}" "${build_doc_dir}/html"
		execute python -m sphinx "${extra_args[@]}" -b doctest "${source_doc_dir}" "${build_doc_dir}/html"
	else
		log "Skipping ${FUNCNAME[0]} as unchanged since ${rev}."
	fi
}


# These differential checks are the ones used in CI, for per-commit diff, install the pre-commit hooks with
#   pre-commit install
function check_code {
	if_rebuild_then build ecole-lib-version
	local extra_args=("$@")
	if [ "${diff}" = "true" ]; then
		extra_args+=("--from-ref" "${rev}" "--to-ref" "HEAD")
	else
		extra_args+=("--all-files")
	fi
	execute pre-commit run "${extra_args[@]}"
}


# The usage of this script.
function help {
	echo "${BASH_SOURCE[0]} [--options...] <cmd1> [<cmd1-args>...] [-- <cmd2> [<cmd2-args>...]]..."
	echo ""
	echo "Options:"
	echo "  --dry-run|--no-dry-run (${dry_run})"
	echo "  --source-dir=<dir> (${source_dir})"
	echo "  --build-dir=<dir> (${build_dir})"
	echo "  --warnings-as-errors|--no-warnings-as-errors (${warnings_as_errors})"
	echo "  --cmake-warnings|--no-cmake-warnings (${cmake_warnings})"
	echo "  --fail-fast|--no-fail-fast (${fail_fast})"
	echo "  --fix-pythonpath|--no-fix-pythonpath (${fix_pythonpath})"
	echo "  --rebuild|--no-rebuild (${rebuild})"
	echo "  --diff|--no-diff (${diff})"
	echo "  --rev=<rev> (${rev})"
	echo ""
	echo "Commands:"
	echo "  help, configure, build-lib, build-lib-test, build-py, build-doc,"
	echo "  test-lib, test-py, test-doc, check-code"
	echo ""
	echo "Example:"
	echo "  ${BASH_SOURCE[0]} --warnings-as-errors configure -D ECOLE_DEVELOPER=ON -- test-lib -- test-py"
}


# Update variable if it exists or throw an error.
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
# As soon as one of this case does not match, all the remaining parameters are put unchanged in
# a `positional` array.
function parse_argv {
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
				positional=("$@")
				return 0
				;;
		esac
	done
}


# Parse the positional arguments and run the commands
#   configure -D ECOLE_DEVELOPER=ON -- test-lib -- test-py --pdb
# Will execute
#   configure -D ECOLE_DEVELOPER=ON
#   test_lib
#   test_py --pdb
function parse_and_run_commands {
	if [ $# = 0 ]; then
		return 0
	fi
	local -r args=("$@")  # Somehow we need to syntax this to use the following syntax.
	# First item in -- separated list is the name of the function where we replace - > _.
	local last_cmd_idx=0
	local func="${args[$last_cmd_idx]//-/_}"

	for idx in ${!args[@]}; do
		# -- is the delimitor that end the parameters for the current function
		if [ "${args[$idx]}" = "--" ]; then
			# Run current function with its args
			${func} "${args[@]:$last_cmd_idx+1:$idx-$last_cmd_idx-1}"
			# Next fucntion start at the position after --
			last_cmd_idx=$(($idx + 1))
			func="${args[$last_cmd_idx]//-/_}"
		fi
	done
	# Run the last function that does not terminate with a -- separator
	${func} "${args[@]:$last_cmd_idx+1}"
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

	# Fix Python ecole import
	if [ "${fix_pythonpath}" = "true" ]; then
		add_build_to_pythonpath
	fi

	# Functions to execute are positional arguments with - replaced by _.
	parse_and_run_commands "${positional[@]}"
	# local -r commands_and_extta args=("${positional[@]//-/_}")

	# for cmd in "${commands[@]}"; do
	#   "${cmd}"
	# done
}


# Run the main when script is not being sourced
if [[ "${BASH_SOURCE[0]}" = "${0}" ]] ; then

	# Fail fast
	set -o errexit
	set -o pipefail
	set -o nounset

	run_main "$@"

fi
