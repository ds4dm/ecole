#!/usr/bin/env bash

# Fail fast
set -o errexit
set -o pipefail
set -o nounset

# Directory of this file
__DIR__="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Top of the repository in which this file is
__ECOLE_DIR__="$(git -C "${__DIR__}" rev-parse --show-toplevel)"


function skip_testing {
	# May skip building doc if relevant files are not edited
	local -r branch="${1}"
	git diff --name-only --exit-code "${branch}" -- docs/ python/src > /dev/null \
		&& return 0 \
		|| return 1
}


function main {
	# If a branch is given, we only test if relevant files have been changes wrt it.
	local differential=""
	# Fail if there are warnings.
	local warnings_as_errors="${CI:-0}"
	# Additional testing of the documentation.
	local enable_testing="${CI:-0}"
	# Where to find sphinx conf.py.
	local source_dir="${__ECOLE_DIR__}/docs"
	# Where to output the doc.
	local build_dir="${__ECOLE_DIR__}/build/doc"
	# Other arguments to pass to sphinx.
	local sphinx_args=()

	# Parse all arguments
	while [[ $# -gt 0 ]]; do
		local arg="${1}"
		case "${arg}" in
			--differential)
				differential="origin/master"
				shift
				;;
			--differential=*)
				differential="${arg#*=}"
				shift
				;;
			--warnings-as-errors)
				warnings_as_errors=1
				shift
				;;
			--enable-testing)
				enable_testing=1
				shift
				;;
			--source-dir=*)
				source_dir="${arg#*=}"
				shift
				;;
			--build-dir=*)
				build_dir="${arg#*=}"
				shift
				;;
			*)
				sphinx_args+=("$1")
				shift
				;;
		esac
	done

	# If differential and testing is not needed return.
	if [ "${differential+set}" == set ] && skip_testing "${differential}" ; then
		return 0
	fi

	# Build according to testing/error policy
	sphinx_args+=("$([ ${warnings_as_errors} ] && printf -- '-W')")
	if [ ${enable_testing} ]; then
		python -m sphinx "${sphinx_args}" -b html "${source_dir}" "${build_dir}/html"
		python -m sphinx "${sphinx_args}" -b linkcheck "${source_dir}" "${build_dir}"
		python -m sphinx "${sphinx_args}" -b doctest "${source_dir}" "${build_dir}"
	else
		python -m pshinx "${sphinx_args}" -b html "${source_dir}" "${build_dir}/html"
	fi
}

main "$@"
