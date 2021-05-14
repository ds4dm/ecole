#!/usr/bin/env bash


# Directory of this file
readonly __DIR__="$(cd "$(dirname "${BASH_SOURCE[0]:?}")" && pwd)"

# Top of the repository in which this file is
readonly __ECOLE_DIR__="$(cd "${__DIR__:?}/.." && pwd)"

# If CI is defined then "true", otherwise "false" (string, not bools).
readonly __CI__="$([ -z "${CI+x}" ] && printf "false" || printf "true")"


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
		## Run the command. Indent both stdout and stderr but preserve them.
		"$@" > >(sed 's/^/  /')  2> >(sed 's/^/  /')
	fi
}


# Wrap call and set PYTHONPATH
function execute_pythonpath {
	if [ "${fix_pythonpath}" = "true" ]; then
		execute export PYTHONPATH="${build_dir}/python${PYTHONPATH+:}${PYTHONPATH:-}"
		execute "$@"
		execute unset PYTHONPATH
	else
		execute "$@"
	fi
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


function build_all {
	# List all functions in that file.
	local all_funcs
	mapfile -t all_funcs < <(declare -F)
	all_funcs=("${all_funcs[@]#declare -f }")
	# Run functions that start with test_
	local func
	for func in "${all_funcs[@]}"; do
		if [[ "${func}" = build_* && "${func}" != "build_all" ]]; then
			"${func}"
		fi
	done
}


function cmake_build {
	execute cmake --build "${build_dir}" --parallel --target "${1-all}" "${@:2}"
}


function build_lib {
	cmake_build ecole-lib "$@"
}


function build_lib_test {
	cmake_build ecole-lib-test "$@"
}


function build_py {
	cmake_build ecole-py-ext "$@"
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
	execute_pythonpath python -m sphinx "${sphinx_args[@]}" -b html "${source_doc_dir}" "${build_doc_dir}" "$@"
}


function test_all {
	# List all functions in that file.
	local all_funcs
	mapfile -t all_funcs < <(declare -F)
	all_funcs=("${all_funcs[@]#declare -f }")
	# Run functions that start with test_
	local func
	for func in "${all_funcs[@]}"; do
		if [[ "${func}" = test_* && "${func}" != "test_all" ]]; then
			"${func}"
		fi
	done
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
		cmake_build test -- ARGS="--parallel ${extra_args[@]}"
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
		execute_pythonpath python -m pytest "${source_dir}/python/tests" "${extra_args[@]}"
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
		execute python -m sphinx "${extra_args[@]}" -b linkcheck "${source_doc_dir}" "${build_doc_dir}"
		execute_pythonpath python -m sphinx "${extra_args[@]}" -b doctest "${source_doc_dir}" "${build_doc_dir}"
	else
		log "Skipping ${FUNCNAME[0]} as unchanged since ${rev}."
	fi
}


function file_version {
	local -r file_major="$(awk '/VERSION_MAJOR/{print $2}' "${source_dir}/VERSION")"
	local -r file_minor="$(awk '/VERSION_MINOR/{print $2}' "${source_dir}/VERSION")"
	local -r file_patch="$(awk '/VERSION_PATCH/{print $2}' "${source_dir}/VERSION")"
	local -r file_pre="$(awk '/VERSION_PRE/{print $2}' "${source_dir}/VERSION")"
	local -r file_post="$(awk '/VERSION_POST/{print $2}' "${source_dir}/VERSION")"
	local -r file_dev="$(awk '/VERSION_DEV/{print $2}' "${source_dir}/VERSION")"
	local version="${file_major:?}.${file_minor:?}.${file_patch:?}"
	version+="${file_pre}${file_post}${file_dev}"
	echo "${version}"
}


# Check that a string is version and print it without the leading 'v'.
function is_version {
	local -r candidate="${1-"$(git describe --tags --exact-match  2> /dev/null)"}"
	( printf "${candidate}" | grep -E '^v?[0-9]+\.[0-9]+\.[0-9]+((a|b|rc)[0-9]+)?(\.post[0-9]+)?(\.dev[0-9]+)?$' | sed 's/^v//' )  || return 1
}


function sort_versions {
	local -r sort_versions=(
		'import sys, distutils.version;'
		'lines = [distutils.version.LooseVersion(l) for l in sys.stdin.readlines()];'
		'versions = sorted(lines);'
		'print(" ".join(str(v) for v in versions));'
	)
	python -c "${sort_versions[*]}"
}


function git_version {
	local -r rev="${1-HEAD}"
	# All possible git tags.
	mapfile -t all_tags < <(git tag)
	# Find tags that are ancestor of rev and match a version.
	local prev_versions
	local tag
	for tag in "${all_tags[@]}"; do
		if git merge-base --is-ancestor "${tag}" "${rev}"; then
			if version=$(is_version "${tag}"); then
				prev_versions+=("${version}")
			fi
		fi
	done
	# Sort using proper version comparison.
	mapfile -t sorted_versions < <(echo "${prev_versions[@]}" | xargs -n1 | sort_versions | xargs -n1)
	# Take the lastest version.
	local -r latest_version="${sorted_versions[${#sorted_versions[@]}-1]}"
	echo "${latest_version}"
}


function test_version {
	# Without args, use the version from git
	if [ -z "${1+x}" ]; then
		local -r version="$(git_version)"
	# Otherwise use the arg
	else
		local -r version=$(is_version "${1}")
	fi
	[ "$(file_version)" = "${version}" ]
}


# These differential checks are the ones used in CI, for per-commit diff, install the pre-commit hooks with
#   pre-commit install
function check_code {
	if_rebuild_then cmake_build ecole-lib-version
	local extra_args=("$@")
	if [ "${diff}" = "true" ]; then
		extra_args+=("--from-ref" "${rev}" "--to-ref" "HEAD")
	else
		extra_args+=("--all-files")
	fi
	execute pre-commit run "${extra_args[@]}"
}


# Install documentation to a local folder depending on the branch/tag
# FIXME the Github Action could be moved here to a deploy_doc function
# FIXME this is not used in Github Action for now
function deploy_doc_locally {
	# Try getting from exact tag.
	local -r tag=$(git -C ${source_dir} describe --tags --exact-match HEAD 2> /dev/null)
	local -r branch="$(git rev-parse --abbrev-ref HEAD)"

	local -r install_dir="${1}"
	if_rebuild_then build_doc

	# Install master to latest
	if printf ${branch} | grep -E '(master|main)' &> /dev/null; then
		local -r dir="${install_dir}/latest"
		# Only create the parent so that source dir is not created in target
		execute mkdir -p "$(dirname "${dir}")"
		execute rm -rf "${dir}"
		execute cp -R "${build_doc_dir}/" "${dir}"
	fi

	# Install versions to v.x.x
	if version=$(is_version "${tag}"); then
		local -r version_major_minor="$(printf "${tag}" | grep -E -o '[0-9]+\.[0-9]+')"
		local -r dir="${install_dir}/v${version_major_minor}"
		# Only create the parent so that source dir is not created in target
		execute mkdir -p "$(dirname "${dir}")"
		execute rm -rf "${dir}"
		execute cp -R "${build_doc_dir}/" "${dir}"
	fi

	# Install stable
	if [[ ! -z "${dir-}" && "$(git_version origin/master)" = "${version-false}" ]]; then
		execute ln -s -f "${dir}" "${install_dir}/stable"
	fi
}


# Build Python source distribution.
function build_sdist {
	local -r dist_dir="${1:-"${build_dir}/dist"}"
	execute python "${source_dir}/setup.py" sdist --dist-dir="${dist_dir}"
}


# Install sdist into a virtual environment.
function test_sdist {
	local -r dist_dir="${build_dir}/dist"
	if_rebuild_then build_sdist "${dist_dir}"
	local -r venv="${build_dir}/venv"
	execute python -m venv --system-site-packages "${venv}"
	local -r sdists=("${dist_dir}"/ecole-*.tar.gz)
	execute "${venv}/bin/python" -m pip install --ignore-installed "${sdists[@]}"
	local extra_args=("$@")
	if [ "${fail_fast}" = "true" ]; then
		extra_args+=("--exitfirst")
	fi
	execute "${venv}/bin/python" -m pytest "${source_dir}/python/tests" "${extra_args[@]}"
}


# Deploy sdist to PyPI. Set TWINE_USERNAME and TWINE_PASSWORD environment variables or pass them as arguments.
function deploy_sdist {
	local -r dist_dir="${build_dir}/dist"
	if_rebuild_then build_sdist "${dist_dir}"
	local -r strict="$([ "${warnings_as_errors}" = "true" ] && echo -n '--strict')"
	local -r sdists=("${dist_dir}"/ecole-*.tar.gz)
	execute python -m twine check "${strict}" "${sdists[@]}"
	execute python -m twine upload --non-interactive "$@" "${sdists[@]}"
}


# The usage of this script.
function help {
	echo "${BASH_SOURCE[0]} [--options...] <cmd1> [<cmd1-args>...] [-- <cmd2> [<cmd2-args>...]]..."
	echo ""
	echo "Options:"
	echo "  --dry-run|--no-dry-run (${dry_run})"
	echo "  --source-dir=<dir> (${source_dir})"
	echo "  --build-dir=<dir> (${build_dir})"
	echo "  --source-doc-dir=<dir> (${source_doc_dir})"
	echo "  --build-doc-dir=<dir> (${build_doc_dir})"
	echo "  --warnings-as-errors|--no-warnings-as-errors (${warnings_as_errors})"
	echo "  --cmake-warnings|--no-cmake-warnings (${cmake_warnings})"
	echo "  --fail-fast|--no-fail-fast (${fail_fast})"
	echo "  --fix-pythonpath|--no-fix-pythonpath (${fix_pythonpath})"
	echo "  --rebuild|--no-rebuild (${rebuild})"
	echo "  --diff|--no-diff (${diff})"
	echo "  --rev=<rev> (${rev})"
	echo ""
	echo "Commands:"
	echo "  help, configure,"
	echo "  build-lib, build-lib-test, build-py, build-doc, build-all"
	echo "  test-lib, test-py, test-doc, test-version, test-all"
	echo "  check-code"
	echo "  build-sdist, test-sdist, deploy-sdist"
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
	local source_dir="${__ECOLE_DIR__:?}"
	# Where is the CMake build folder with the test.
	local build_dir="build"
	# Where to find sphinx conf.py.
	local source_doc_dir="${__ECOLE_DIR__}/docs"
	# Where to output the doc.
	local build_doc_dir="${build_dir}/docs/html"
	# Fail if there are warnings.
	local warnings_as_errors="${__CI__}"
	# Warning for CMake itself (not compiler).
	local cmake_warnings="${__CI__}"
	# Stop on first failure
	local fail_fast="${__CI__}"
	# Add build tree to PYTHONPATH.
	local fix_pythonpath="$([ "${__CI__}" = "true" ] && printf "false" || printf "true")"
	# Automaticaly rebuild libraries for tests and doc.
	local rebuild="true"
	# Test only if relevant differences have been made since the revision branch
	local rev="origin/master"
	local diff="$([ "${__CI__}" = "true" ] && printf "false" || printf "true")"

	# Parse all command line arguments.
	parse_argv "$@"

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
