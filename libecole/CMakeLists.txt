add_library(
	ecole-lib

	src/version.cpp
	src/random.cpp
	src/exception.cpp

	src/utility/reverse-control.cpp
	src/utility/chrono.cpp
	src/utility/graph.cpp

	src/scip/scimpl.cpp
	src/scip/model.cpp
	src/scip/cons.cpp
	src/scip/var.cpp
	src/scip/row.cpp
	src/scip/col.cpp
	src/scip/exception.cpp

	src/instance/files.cpp
	src/instance/set-cover.cpp
	src/instance/independent-set.cpp
	src/instance/combinatorial-auction.cpp
	src/instance/capacitated-facility-location.cpp

	src/reward/is-done.cpp
	src/reward/lp-iterations.cpp
	src/reward/solving-time.cpp
	src/reward/n-nodes.cpp
	src/reward/bound-integral.cpp

	src/observation/node-bipartite.cpp
	src/observation/milp-bipartite.cpp
	src/observation/khalil-2016.cpp
	src/observation/hutter-2011.cpp
	src/observation/strong-branching-scores.cpp
	src/observation/pseudocosts.cpp

	src/dynamics/parts.cpp
	src/dynamics/branching.cpp
	src/dynamics/configuring.cpp
	src/dynamics/primal-search.cpp
)

add_library(Ecole::ecole-lib ALIAS ecole-lib)

# Unconditionally generate version file at build time
string(TIMESTAMP Ecole_BUILD_TIME)
add_custom_target(
	ecole-lib-version
	COMMAND ${CMAKE_COMMAND}
		-D SOURCE_FILE="${CMAKE_CURRENT_SOURCE_DIR}/include/ecole/version.hpp.in"
		-D TARGET_FILE="${CMAKE_CURRENT_BINARY_DIR}/include/ecole/version.hpp"
		-D Ecole_VERSION_MAJOR="${Ecole_VERSION_MAJOR}"
		-D Ecole_VERSION_MINOR="${Ecole_VERSION_MINOR}"
		-D Ecole_VERSION_PATCH="${Ecole_VERSION_PATCH}"
		-D Ecole_VERSION_REVISION="${Ecole_VERSION_REVISION}"  # Not defined by default, but let if override for conda
		-D Ecole_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
		-D Ecole_BUILD_OS="${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_VERSION}"
		-D Ecole_BUILD_TIME="${Ecole_BUILD_TIME}"
		-D Ecole_BUILD_COMPILER="${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}"
		-P "${Ecole_SOURCE_DIR}/cmake/CreateVersionFile.cmake" > /dev/null
)
add_dependencies(ecole-lib ecole-lib-version)

# Control symbol visibility
include(GenerateExportHeader)
generate_export_header(
	ecole-lib
	BASE_NAME Ecole
	EXPORT_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/include/ecole/export.hpp"
)

# Set library file name and soname.
# ABI compatibility (SOVERSION) is kept only at the minor level.
set_target_properties(
	ecole-lib
	PROPERTIES
		OUTPUT_NAME ecole
		VERSION "${Ecole_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.${${PROJECT_NAME}_VERSION_PATCH}"
		SOVERSION "${Ecole_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}"
)

target_include_directories(
	ecole-lib
	PUBLIC
		$<INSTALL_INTERFACE:include>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Files that download the dependencies of libecole
include(dependencies/public.cmake)
include(dependencies/private.cmake)

find_package(SCIP 7 REQUIRED)
find_package(Threads REQUIRED)
find_package(xtl REQUIRED)
find_package(xsimd REQUIRED)
find_package(xtensor REQUIRED)
find_package(span-lite REQUIRED)
find_package(range-v3 REQUIRED)
find_package(fmt REQUIRED)
find_package(robin_hood REQUIRED)


target_link_libraries(
	ecole-lib
	PUBLIC
		libscip
		xtensor
		xtensor::use_xsimd
		nonstd::span-lite
		Threads::Threads
	PRIVATE
		fmt::fmt
		range-v3::range-v3
		robin_hood::robin_hood
)

ecole_target_add_compile_warnings(ecole-lib)
ecole_target_add_sanitizers(ecole-lib)
ecole_target_add_coverage(ecole-lib)

# System CPU time, silently ignored if LibRT is not present
find_library(LIBRT rt)
if(LIBRT)
	target_link_libraries(ecole-lib PRIVATE "${LIBRT}")
endif()

target_compile_features(ecole-lib PUBLIC cxx_std_17)

# Installation library and symlink
include(GNUInstallDirs)
install(
	TARGETS ecole-lib
	EXPORT "EcoleTargets"
	RUNTIME
		DESTINATION "${CMAKE_INSTALL_BINDIR}"
		COMPONENT Ecole_Runtime
	LIBRARY
		DESTINATION "${CMAKE_INSTALL_LIBDIR}"
		COMPONENT Ecole_Runtime
		NAMELINK_COMPONENT Ecole_Development
	ARCHIVE
		DESTINATION "${CMAKE_INSTALL_LIBDIR}"
		COMPONENT Ecole_Development
)

# Install CMake targets definition
install(
	EXPORT "EcoleTargets"
	FILE "EcoleTargets.cmake"
	NAMESPACE Ecole::
	COMPONENT Ecole_Development
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Ecole"
)

# Install headers and generated headers
install(
	DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/ecole" "${CMAKE_CURRENT_BINARY_DIR}/include/ecole"
	DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
	COMPONENT Ecole_Development
	FILES_MATCHING PATTERN "*.hpp"
)

# Generate and install config and version files
include(CMakePackageConfigHelpers)
configure_package_config_file(
	"EcoleConfig.cmake.in"
	"EcoleConfig.cmake"
	INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Ecole"
)
write_basic_package_version_file(
	"EcoleConfigVersion.cmake"
	VERSION "${Ecole_VERSION}"
	COMPATIBILITY SameMinorVersion
)
install(
	FILES
		"${CMAKE_CURRENT_BINARY_DIR}/EcoleConfig.cmake"
		"${CMAKE_CURRENT_BINARY_DIR}/EcoleConfigVersion.cmake"
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Ecole"
	COMPONENT Ecole_Development
)

# Install the files to download dependencies (not mandatory but useful for users)
install(
	FILES
		"${Ecole_SOURCE_DIR}/cmake/DependenciesResolver.cmake"
		"${CMAKE_CURRENT_SOURCE_DIR}/dependencies/public.cmake"
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Ecole"
	COMPONENT Ecole_Development
)

# Add test if this is the main project and testing is enabled
if(ECOLE_BUILD_TESTS)
	add_subdirectory(tests)
endif()

if(ECOLE_BUILD_BENCHMARKS)
	add_subdirectory(benchmarks)
endif()
