# Script to configure version files at build time.
#
# This is not meant to be included directly in CMakeLists.txt but to be called with `cmake -P`
# script mode.
# This way, one can build a target that regenerate the version file at every compilation.
# It avoids getting an outdated Git revision.
# All other variable defined before running the script can also be used for templating the
# versio file.

cmake_minimum_required(VERSION 3.14)

# Default working directory
if(NOT WORKING_DIR)
	get_filename_component(WORKING_DIR "${SOURCE_FILE}" DIRECTORY)
endif()

if(NOT Ecole_VERSION_MAJOR)
	set(Ecole_VERSION_MAJOR 0)
endif()
if(NOT Ecole_VERSION_MINOR)
	set(Ecole_VERSION_MINOR 0)
endif()
if(NOT Ecole_VERSION_PATCH)
	set(Ecole_VERSION_PATCH 0)
endif()

message(STATUS "Resolving Git Version")

set(GIT_REVISION "unknown")

find_package(Git)
if(GIT_FOUND)
	execute_process(
		COMMAND ${GIT_EXECUTABLE} rev-parse --verify HEAD
		WORKING_DIRECTORY "${WORKING_DIR}"
		OUTPUT_VARIABLE GIT_REVISION
		ERROR_QUIET
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	message(STATUS "Git revision: ${GIT_REVISION}")
else()
	message(STATUS "Git not found")
endif()

string(TIMESTAMP BUILD_TIMESTAMP)

configure_file("${SOURCE_FILE}" "${TARGET_FILE}" @ONLY)
