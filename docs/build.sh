#!/usr/bin/env bash


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


# Build the html documentation.
function build_doc {
	if [ ${warnings_as_errors} = "true" ]; then
		local sphinx_args+=("-W")
	fi
	execute python -m sphinx "${sphinx_args[@]}" -b html "${source_doc_dir}" "${build_doc_dir}/html"
}


# Build the html documentation and test the code snippets and links,
function test_doc {
	if [ ${warnings_as_errors} = "true" ]; then
		local sphinx_args+=("-W")
	fi
	execute python -m sphinx "${sphinx_args[@]}" -b linkcheck "${source_doc_dir}" "${build_doc_dir}/html"
	execute python -m sphinx "${sphinx_args[@]}" -b doctest "${source_doc_dir}" "${build_doc_dir}/html"
}


# Parse arguments, build, and test documentation.
function doc_main {
	# Fail if there are warnings.
	local warnings_as_errors="false"
	# Additional testing of the documentation.
	local test="false"
	# Where to find sphinx conf.py.
	local source_doc_dir="${__ECOLE_DIR__}/docs"
	# Where to output the doc.
	local build_doc_dir="${__ECOLE_DIR__}/build/docs"
	# Other arguments to pass to sphinx.
	local sphinx_args=()

	# Parse all arguments
	while [[ $# -gt 0 ]]; do
		local arg="${1}"
		case "${arg}" in
			--warnings-as-errors|--wae)
				warnings_as_errors="true"
				shift
				;;
			--test)
				test="true"
				shift
				;;
			--source-doc-dir=*)
				source_doc_dir="${arg#*=}"
				shift
				;;
			--build-doc-dir=*)
				build_doc_dir="${arg#*=}"
				shift
				;;
			*)
				sphinx_args+=("$1")
				shift
				;;
		esac
	done

	# Build according to testing/error policy
	build_doc
	if [ ${test} = "true" ]; then
		test_doc
	fi
}

# Run the main when script is not being sourced
if [[ "${BASH_SOURCE[0]}" = "${0}" ]] ; then

	# Fail fast
	set -o errexit
	set -o pipefail
	set -o nounset

	# Directory of this file
	__DIR__="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

	# Top of the repository in which this file is
	__ECOLE_DIR__="$(git -C "${__DIR__}" rev-parse --show-toplevel)"

	doc_main "$@"

fi
