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
	# Fail if there are warnings.
	local warnings_as_errors="false"
	# Additional testing of the documentation.
	local enable_testing="false"
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
				warnings_as_errors="true"
				shift
				;;
			--enable-testing)
				enable_testing="true"
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

	# Build according to testing/error policy
	sphinx_args+=("$([ ${warnings_as_errors} = "true" ] && printf -- '-W')")
	if [ ${enable_testing} = "true" ]; then
		python -m sphinx "${sphinx_args}" -b html "${source_dir}" "${build_dir}/html"
		python -m sphinx "${sphinx_args}" -b linkcheck "${source_dir}" "${build_dir}"
		python -m sphinx "${sphinx_args}" -b doctest "${source_dir}" "${build_dir}"
	else
		python -m pshinx "${sphinx_args}" -b html "${source_dir}" "${build_dir}/html"
	fi
}

main "$@"
