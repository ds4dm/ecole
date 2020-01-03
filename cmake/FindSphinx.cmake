cmake_minimum_required (VERSION 3.8)

#Look for an executable called sphinx-build
find_program(
	SPHINX_EXECUTABLE
	NAMES sphinx-build
	$ENV{SPHINX_DIR}
	PATH_SUFFIXES bin
	DOC "Sphinx documentation generator"
)

include(FindPackageHandleStandardArgs)

# Handle standard arguments to find_package like REQUIRED and QUIET
find_package_handle_standard_args(Sphinx DEFAULT_MSG SPHINX_EXECUTABLE)

add_executable(Sphinx::build IMPORTED GLOBAL)
set_target_properties(Sphinx::build PROPERTIES IMPORTED_LOCATION "${SPHINX_EXECUTABLE}")

mark_as_advanced(SPHINX_EXECUTABLE)
