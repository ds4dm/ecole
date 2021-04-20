cmake_minimum_required(VERSION 3.14)

# Adapt compiler flags if using Conda compiler packages. Before project so they are not modified.
include(cmake/Conda.cmake)

# Read the version from file
include(cmake/Version.cmake)
read_version("${CMAKE_SOURCE_DIR}/VERSION" Ecole_VERSION)

# Set default parameters. Assumes Ecole user,
include(cmake/DefaultSettings.cmake)

project(
	Ecole
	VERSION "${Ecole_VERSION}"
	LANGUAGES CXX
	DESCRIPTION "Extensible Combinatorial Optimization Learning Environments"
)

# Add option to enable interprocedural optimization
include(cmake/InterproceduralOptimization.cmake)

# Define a target Ecole::warnings with all compiler warnings.
include(cmake/CompilerWarnings.cmake)

# Define a target Ecole::sanitizers with enabled sanitizers.
include(cmake/Sanitizers.cmake)

# Define a target Ecole::coverage with coverage options.
include(cmake/Coverage.cmake)

# Utilities to automatically download missing dependencies
include(cmake/Dependencies.cmake)

# Adapt which Python is found
include(cmake/Python.cmake)

# Enable CTest for registering tests
include(CTest)

# Ecole libraries
add_subdirectory(libecole)
add_subdirectory(python)
