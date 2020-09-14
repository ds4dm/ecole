option(ENABLE_DOCUMENTATION "Build documentation with Doxygen and Sphinx" OFF)
option(DOC_WARNINGS_AS_ERRORS "Treat documentation warnings as errors" OFF)
option(ENABLE_DOCUMENTATION_TESTING "Enable documentation testing" OFF)

if(NOT ENABLE_DOCUMENTATION)
	return()
endif()

find_package(Doxygen REQUIRED)

# Equivalent to Doxyfile configurations
set(DOXYGEN_QUIET YES)
set(DOXYGEN_GENERATE_XML YES)
set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/doxygen")
set(DOXYGEN_WARN_IF_UNDOCUMENTED NO)
# Doxygen accepts only yes/no (not true/false, on/off) so we need to make a disjunction
if(DOC_WARNINGS_AS_ERRORS)
	set(DOXYGEN_WARN_AS_ERROR YES)
else()
	set(DOXYGEN_WARN_AS_ERROR NO)
endif()

doxygen_add_docs(
	ecole-doxygen
	"${PROJECT_SOURCE_DIR}/libecole"
	COMMENT "Parsing C++ documentation with Doxygen"
)

set(SPHINX_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(SPHINX_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/sphinx")
set(SPHINX_HTML_DIR "${SPHINX_BUILD_DIR}/html")
if(WARNINGS_AS_ERRORS)
	set(SPHINX_WARNINGS_AS_ERRORS "-W")
else()
	set(SPHINX_WARNINGS_AS_ERRORS "")
endif()

# If some CMake expert knows how to to do simply this using the commands args from a variable... (╯°□°）╯︵ ┻━┻
if(ENABLE_DOCUMENTATION_TESTING)
	add_custom_target(
		ecole-sphinx
		COMMAND venv-python -m sphinx -q "${SPHINX_WARNINGS_AS_ERRORS}" -b html
			-D breathe_projects.libecole="${DOXYGEN_OUTPUT_DIRECTORY}/xml"
			"${SPHINX_SOURCE_DIR}" "${SPHINX_HTML_DIR}"
		COMMAND venv-python -m sphinx -q "${SPHINX_WARNINGS_AS_ERRORS}" -b linkcheck
			-D breathe_projects.libecole="${DOXYGEN_OUTPUT_DIRECTORY}/xml"
			"${SPHINX_SOURCE_DIR}" "${SPHINX_BUILD_DIR}"
		COMMAND venv-python -m sphinx -q "${SPHINX_WARNINGS_AS_ERRORS}" -b doctest
			-D breathe_projects.libecole="${DOXYGEN_OUTPUT_DIRECTORY}/xml"
			"${SPHINX_SOURCE_DIR}" "${SPHINX_BUILD_DIR}"
		BYPRODUCTS "${SPHINX_BUILD_DIR}"
		DEPENDS ecole-doxygen ecole-venv
		COMMENT "Generating documentation with Sphinx"
	)
else()
	add_custom_target(
		ecole-sphinx
		COMMAND venv-python -m sphinx -q "${SPHINX_WARNINGS_AS_ERRORS}" -b html
			-D breathe_projects.libecole="${DOXYGEN_OUTPUT_DIRECTORY}/xml"
			"${SPHINX_SOURCE_DIR}" "${SPHINX_HTML_DIR}"
		BYPRODUCTS "${SPHINX_BUILD_DIR}"
		DEPENDS ecole-doxygen ecole-venv
		COMMENT "Generating documentation with Sphinx"
	)
endif()
