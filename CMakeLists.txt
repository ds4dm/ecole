cmake_minimum_required(VERSION 3.14)

# RPATH escaping (only in old CMake versions)
if (CMAKE_VERSION VERSION_LESS "3.24.0")
	cmake_policy(SET CMP0095 OLD)
endif()

# Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP in CMake 3.24:
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
	cmake_policy(SET CMP0135 NEW)
endif()

# Adapt compiler flags if using Conda compiler packages. Before project so they are not modified.
include(cmake/Conda.cmake)

# Read the version from file
include(cmake/Version.cmake)
read_version("VERSION" Ecole_VERSION)

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
include(cmake/DependenciesResolver.cmake)

# Adapt which Python is found
include(cmake/Python.cmake)

# Enable CTest for registering tests
include(CTest)

# Ecole library
if(ECOLE_BUILD_LIB)
	# Build the Ecole library
	add_subdirectory(libecole)
else()
	# Find the Ecole library of same version already installed
	option(ECOLE_DOWNLOAD_DEPENDENCIES "Download the static and header libraries used in Ecole public interface" ON)
	find_package(Ecole ${Ecole_VERSION} EXACT REQUIRED)
endif()

# Ecole Python extension
if(ECOLE_BUILD_PY_EXT)
	add_subdirectory(python/extension-helper)
	add_subdirectory(python/ecole)
endif()
