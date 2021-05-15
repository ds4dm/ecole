add_library(
	ecole-lib

	src/version.cpp
	src/random.cpp
	src/exception.cpp

	src/utility/reverse-control.cpp
	src/utility/chrono.cpp

	src/scip/scimpl.cpp
	src/scip/model.cpp
	src/scip/cons.cpp
	src/scip/var.cpp
	src/scip/row.cpp
	src/scip/col.cpp
	src/scip/exception.cpp

	src/instance/set-cover.cpp
	src/instance/independent-set.cpp
	src/instance/independent-set-graph.cpp
	src/instance/combinatorial-auction.cpp
	src/instance/capacitated-facility-location.cpp

	src/reward/isdone.cpp
	src/reward/lpiterations.cpp
	src/reward/solvingtime.cpp
	src/reward/nnodes.cpp

	src/observation/nodebipartite.cpp
	src/observation/milpbipartite.cpp
	src/observation/khalil-2016.cpp
	src/observation/strongbranchingscores.cpp
	src/observation/pseudocosts.cpp

	src/dynamics/branching.cpp
	src/dynamics/configuring.cpp
)
set_target_properties(ecole-lib PROPERTIES OUTPUT_NAME ecole)

add_library(Ecole::lib ALIAS ecole-lib)

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
		-P "${CMAKE_SOURCE_DIR}/cmake/CreateVersionFile.cmake" > /dev/null
)
add_dependencies(ecole-lib ecole-lib-version)


target_include_directories(
	ecole-lib
	PUBLIC
		$<INSTALL_INTERFACE:include>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
)

find_package(SCIP 7 REQUIRED)
find_package(Threads REQUIRED)

option(XTENSOR_USE_XSIMD "Use xsimd with xtensor" ON)
find_or_download_package(
	NAME xtl
	URL https://github.com/xtensor-stack/xtl/archive/0.7.2.tar.gz
	URL_HASH SHA256=95c221bdc6eaba592878090916383e5b9390a076828552256693d5d97f78357c
	CONFIGURE_ARGS -D BUILD_TESTS=OFF
)
find_or_download_package(
	NAME xsimd
	URL https://github.com/xtensor-stack/xsimd/archive/7.4.9.tar.gz
	URL_HASH SHA256=f6601ffb002864ec0dc6013efd9f7a72d756418857c2d893be0644a2f041874e
	CONFIGURE_ARGS -D BUILD_TESTS=OFF
)
find_or_download_package(
	NAME xtensor
	URL https://github.com/xtensor-stack/xtensor/archive/0.23.1.tar.gz
	URL_HASH SHA256=b9bceea49db240ab64eede3776d0103bb0503d9d1f3ce5b90b0f06a0d8ac5f08
	CONFIGURE_ARGS -D BUILD_TESTS=OFF
)

find_or_download_package(
	NAME span-lite
	URL https://github.com/martinmoene/span-lite/archive/v0.9.0.tar.gz
	URL_HASH SHA256=cdb5f86e5f5e679d63700a56de734c44fe22a574a17347d09dbaaef80619af91
	CONFIGURE_ARGS
		-D SPAN_LITE_OPT_BUILD_TESTS=OFF
		-D SPAN_LITE_OPT_BUILD_EXAMPLES=OFF
)
find_or_download_package(
	NAME range-v3
	URL https://github.com/ericniebler/range-v3/archive/0.11.0.tar.gz
	URL_HASH SHA256=376376615dbba43d3bef75aa590931431ecb49eb36d07bb726a19f680c75e20c
	CONFIGURE_ARGS
		-D RANGE_V3_TESTS=OFF
		-D RANGE_V3_EXAMPLES=OFF
		-D RANGE_V3_PERF=OFF
		-D RANGE_V3_DOCS=OFF
)
find_or_download_package(
	NAME fmt
	URL https://github.com/fmtlib/fmt/archive/7.1.3.tar.gz
	URL_HASH SHA256=5cae7072042b3043e12d53d50ef404bbb76949dad1de368d7f993a15c8c05ecc
	CONFIGURE_ARGS
		-D FMT_TEST=OFF
		-D FMT_DOC=OFF
		-D FMT_INSTALL=ON
		-D CMAKE_BUILD_TYPE=Release
		-D CMAKE_POSITION_INDEPENDENT_CODE=${CMAKE_POSITION_INDEPENDENT_CODE}
)

target_link_libraries(
	ecole-lib
	PUBLIC
		libscip
		xtensor
		nonstd::span-lite
	PRIVATE
		Ecole::warnings
		Ecole::sanitizers
		Ecole::coverage
		Threads::Threads
		fmt::fmt
		range-v3::range-v3
)

# System CPU time, silently ignored if LibRT is not present
find_library(LIBRT rt)
if(LIBRT)
	target_link_libraries(ecole-lib PRIVATE "${LIBRT}")
endif()

target_compile_features(ecole-lib PUBLIC cxx_std_17)

# Add test if this is the main project and testing is enabled
if(ECOLE_BUILD_TESTS)
	add_subdirectory(tests)
endif()

if(ECOLE_BUILD_BENCHMARKS)
	add_subdirectory(benchmarks)
endif()
