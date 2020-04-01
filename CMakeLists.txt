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

# Define a target Ecole::warnings with all compiler warnings.
include(cmake/CompilerWarnings.cmake)

# Define a target Ecole::sanitizers with enabled sanitizers.
include(cmake/Sanitizers.cmake)

# Activate the Conan CMake wrapper (download if necessary).
include(cmake/Conan.cmake)

# Set a default build type (required for conan)
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE None CACHE STRING "Set build type, see CMake documentation" FORCE)
endif()

# Activate Testing by default
if(NOT DEFINED BUILD_TESTING)
	set(BUILD_TESTING ON)
endif()

add_subdirectory(libecole)
add_subdirectory(python)
