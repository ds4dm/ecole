cmake_minimum_required(VERSION 3.14)

add_library(
	libecole
	src/version.cpp
	src/scip/scimpl.cpp
	src/scip/model.cpp
	src/scip/exception.cpp
	src/utility/reverse-control.cpp
	src/reward/isdone.cpp
	src/reward/lpiterations.cpp
	src/reward/nnodes.cpp
	src/observation/nodebipartite.cpp
	src/observation/strongbranchingscores.cpp
	src/observation/pseudocosts.cpp
	src/environment/branching-dynamics.cpp
	src/environment/configuring-dynamics.cpp
	src/environment/exception.cpp
)
set_target_properties(libecole PROPERTIES OUTPUT_NAME ecole)

add_library(Ecole::libecole ALIAS libecole)

# Unconditionally generate version file at build time
add_custom_target(
	ecole-version
	COMMAND ${CMAKE_COMMAND}
		-D SOURCE_FILE="${CMAKE_CURRENT_SOURCE_DIR}/include/ecole/version.hpp.in"
		-D TARGET_FILE="${CMAKE_CURRENT_BINARY_DIR}/include/ecole/version.hpp"
		-D Ecole_VERSION_MAJOR="${Ecole_VERSION_MAJOR}"
		-D Ecole_VERSION_MINOR="${Ecole_VERSION_MINOR}"
		-D Ecole_VERSION_PATCH="${Ecole_VERSION_PATCH}"
		-P "${CMAKE_SOURCE_DIR}/cmake/VersionScript.cmake" > /dev/null
)
add_dependencies(libecole ecole-version)


target_include_directories(
	libecole
	PUBLIC
		$<INSTALL_INTERFACE:include>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
)

find_package(SCIP 7 REQUIRED)
find_package(Threads REQUIRED)
target_link_libraries(libecole PUBLIC libscip PRIVATE Threads::Threads)

# Temporary fix for hardcoding SCIP missing RPath. MacOS bug only?
get_target_property(SCIP_LOCATION libscip LOCATION)
get_filename_component(SCIP_RPATH "${SCIP_LOCATION}" DIRECTORY)
list(APPEND CMAKE_BUILD_RPATH "${SCIP_RPATH}")

conan_cmake_run(
	CONANFILE conanfile.txt
	BASIC_SETUP
	CMAKE_TARGETS
	NO_OUTPUT_DIRS
	SKIP_STD
	KEEP_RPATHS
	BUILD missing
	ENV CONAN_CMAKE_TOOLCHAIN_FILE="${CMAKE_TOOLCHAIN_FILE}"
	OUTPUT_QUIET
)
target_link_libraries(
	libecole
	PUBLIC
		libscip
		CONAN_PKG::xtensor
		CONAN_PKG::optional-lite
		CONAN_PKG::variant-lite
		CONAN_PKG::span-lite
		CONAN_PKG::fmt
	PRIVATE
		Ecole::warnings
		Ecole::sanitizers
		Ecole::coverage
		Threads::Threads
)

# Force optional-lite and variant-lite to not use std:: as an implementation.
# Required as we are using C++14, but conda export a C++17 flag.
target_compile_definitions(
	libecole
	PUBLIC
		optional_CONFIG_SELECT_OPTIONAL=optional_OPTIONAL_NONSTD
		variant_CONFIG_SELECT_VARIANT=variant_VARIANT_NONSTD
)

target_compile_features(libecole PUBLIC cxx_std_17)

set_target_properties(libecole PROPERTIES
	# All code ending in a shared library should be made PIC
	POSITION_INDEPENDENT_CODE ON
	# Compiling with hidden visibility
	# FIXME use CMAKE_GENERATE_EXPORT_HEADER when supporting building shared library
	CXX_VISIBILITY_PRESET hidden
	VISIBILITY_INLINES_HIDDEN ON
)

# Add test if this is the main project and testing is enabled
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
	add_subdirectory(tests)
endif()
