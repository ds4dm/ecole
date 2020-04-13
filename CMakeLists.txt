cmake_minimum_required(VERSION 3.14)


project(
	Ecole
	VERSION 0.0.2
	LANGUAGES CXX
	DESCRIPTION "Extensible Combinatorial Optimization Learning Environments"
)
set(Ecole_AUTHORS "Antoine Prouvost")

# Add the cmake folder for external packages
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# Set default parameters. Assumes Ecole user,
include(cmake/DefaultSettings.cmake)

# Define a target Ecole::warnings with all compiler warnings.
include(cmake/CompilerWarnings.cmake)

# Define a target Ecole::sanitizers with enabled sanitizers.
include(cmake/Sanitizers.cmake)

# Activate the Conan CMake wrapper (download if necessary).
include(cmake/Conan.cmake)

# Enable tools for checking the code at compile time.
include(cmake/StaticAnalysis.cmake)

# Find Python and create virtual environment for Ecole
include(cmake/Python.cmake)

# Ecole libraries
add_subdirectory(libecole)
add_subdirectory(python)

# Documentation with Doxygen and Sphinx
add_subdirectory(docs EXCLUDE_FROM_ALL)
