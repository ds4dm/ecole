cmake_minimum_required (VERSION 3.8)

option(ENABLE_DOCUMENTATION "Build documentation with Doxygen and Sphinx" OFF)

if(NOT ENABLE_DOCUMENTATION)
	return()
endif()

find_package(Doxygen REQUIRED)

# Equivalent to Doxyfile configurations
set(DOXYGEN_QUIET YES)
set(DOXYGEN_GENERATE_XML YES)
set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/doxygen")
set(DOXYGEN_WARN_AS_ERROR YES)
set(DOXYGEN_WARN_IF_UNDOCUMENTED NO)

doxygen_add_docs(
	ecole-doxygen
	"${PROJECT_SOURCE_DIR}/libecole"
	COMMENT "Parsing C++ documentation with Doxygen"
)

set(SPHINX_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(SPHINX_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/sphinx")
set(SPHINX_HTML_DIR "${SPHINX_BUILD_DIR}/html")

file(MAKE_DIRECTORY "${SPHINX_BUILD_DIR}")
configure_file("${SPHINX_SOURCE_DIR}/conf.py.in" "${SPHINX_BUILD_DIR}/conf.py" @ONLY)
add_custom_target(
	ecole-sphinx
	COMMAND
	venv-python -m sphinx -q -W -b html
		-c "${SPHINX_BUILD_DIR}"  # Find config file in build directory
		-D breathe_projects.libecole="${DOXYGEN_OUTPUT_DIRECTORY}/xml"
		"${SPHINX_SOURCE_DIR}" "${SPHINX_HTML_DIR}"
	BYPRODUCTS "${SPHINX_BUILD_DIR}"
	DEPENDS ecole-doxygen ecole-venv
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	COMMENT "Generating documentation with Sphinx"
)
