#!/usr/bin/env bash

# Fail fast
set -o errexit
set -o pipefail
set -o nounset

# Directory of this file
__DIR__="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"


function main {
	# Default values
	local warnings_as_errors="${CI-0}"
	local enable_testing="${CI-0}"
	local source_dir="${__DIR__}"
	local build_dir="build/doc"
	local sphinx_args=()

	# Parse all arguments
	while [[ $# -gt 0 ]]; do
		key="$1"

		case $key in
			--warnings-as-errors)
				warnings_as_errors=1
				shift 1
				;;
			--enable-testing)
				enable_testing=1
				shift 1
				;;
			--source-dir)
				source_dir="$2"
				shift 2
				;;
			--build-dir)
				build_dir="$2"
				shift 2
				;;
			*)
				sphinx_args+=("$1")
				shift 1
				;;
		esac
	done

	sphinx_args+=("$([ ${warnings_as_errors} ] && printf -- '-W')")

	if [ ${enable_testing} ]; then
		python -m sphinx "${sphinx_args}" -b html "${source_dir}" "${build_dir}"
		python -m sphinx "${sphinx_args}" -b linkcheck "${source_dir}" "${build_dir}"
		python -m sphinx "${sphinx_args}" -b doctest "${source_dir}" "${build_dir}"
	else
		python -m pshinx "${sphinx_args}" -b html "${source_dir}" "${build_dir}"
	fi
}

main "$@"
