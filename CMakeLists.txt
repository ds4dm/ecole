cmake_minimum_required(VERSION 3.14)

# The following are set by conda compiler tools but not read by CMake
include(cmake/Environment.cmake)
set_from_env(CMAKE_AR AR CACHE FILEPATH "Archiving tool for static libraries")
set_from_env(CMAKE_RANLIB RANLIB CACHE FILEPATH "Randomizing tool for static libraries")


project(
	Ecole
	VERSION 0.0.2
	LANGUAGES CXX
	DESCRIPTION "Extensible Combinatorial Optimization Learning Environments"
)

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

# Ecole libraries
add_subdirectory(libecole)
add_subdirectory(python)
