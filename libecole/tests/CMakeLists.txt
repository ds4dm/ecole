add_executable(
	ecole-lib-test

	src/main.cpp
	src/conftest.cpp

	src/test-utility/tmp-folder.cpp

	src/test-traits.cpp
	src/test-random.cpp

	src/utility/test-chrono.cpp
	src/utility/test-coroutine.cpp
	src/utility/test-vector.cpp
	src/utility/test-random.cpp
	src/utility/test-graph.cpp
	src/utility/test-sparse-matrix.cpp

	src/scip/test-scimpl.cpp
	src/scip/test-model.cpp

	src/instance/unit-tests.cpp
	src/instance/test-files.cpp
	src/instance/test-set-cover.cpp
	src/instance/test-independent-set.cpp
	src/instance/test-combinatorial-auction.cpp
	src/instance/test-capacitated-facility-location.cpp

	src/data/test-constant.cpp
	src/data/test-none.cpp
	src/data/test-tuple.cpp
	src/data/test-vector.cpp
	src/data/test-map.cpp
	src/data/test-multiary.cpp
	src/data/test-parser.cpp
	src/data/test-timed.cpp
	src/data/test-dynamic.cpp

	src/reward/test-lp-iterations.cpp
	src/reward/test-is-done.cpp
	src/reward/test-n-nodes.cpp
	src/reward/test-solving-time.cpp
	src/reward/test-bound-integral.cpp

	src/observation/test-node-bipartite.cpp
	src/observation/test-milp-bipartite.cpp
	src/observation/test-strong-branching-scores.cpp
	src/observation/test-pseudocosts.cpp
	src/observation/test-khalil-2016.cpp
	src/observation/test-hutter-2011.cpp

	src/dynamics/test-parts.cpp
	src/dynamics/test-branching.cpp
	src/dynamics/test-configuring.cpp
	src/dynamics/test-primal-search.cpp

	src/environment/test-environment.cpp
)

target_compile_definitions(
	ecole-lib-test PRIVATE TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
)

target_include_directories(
	ecole-lib-test
	PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/src"
		"${${PROJECT_NAME}_SOURCE_DIR}/libecole/src"  # Add libecole private include
)

# File that download the dependencies of libecole
include(dependencies/private.cmake)

find_package(Catch2 REQUIRED)

target_link_libraries(
	ecole-lib-test
	PRIVATE
		Ecole::ecole-lib
		range-v3::range-v3
		Catch2::Catch2
		libscip
)

ecole_target_add_compile_warnings(ecole-lib)
ecole_target_add_sanitizers(ecole-lib)

include("${Catch2_DIR}/Catch.cmake")
enable_testing()
catch_discover_tests(ecole-lib-test)
