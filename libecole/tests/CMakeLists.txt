add_executable(
	test-libecole
	src/main.cpp
	src/conftest.cpp

	src/test-traits.cpp
	src/test-random.cpp

	src/utility/test-chrono.cpp
	src/utility/test-vector.cpp
	src/utility/test-random.cpp

	src/scip/test-scimpl.cpp
	src/scip/test-model.cpp

	src/data/test-constant.cpp
	src/data/test-none.cpp
	src/data/test-tuple.cpp
	src/data/test-vector.cpp
	src/data/test-map.cpp
	src/data/test-multiary.cpp
	src/data/test-parser.cpp
	src/data/test-timed.cpp

	src/instance/unit-tests.cpp
	src/instance/test-set-cover.cpp
	src/instance/test-independent-set.cpp
	src/instance/test-combinatorial-auction.cpp
	src/instance/test-capacitated-facility-location.cpp

	src/reward/test-lpiterations.cpp
	src/reward/test-isdone.cpp
	src/reward/test-nnodes.cpp
	src/reward/test-solvingtime.cpp

	src/observation/test-nodebipartite.cpp
	src/observation/test-strongbranchingscores.cpp
	src/observation/test-pseudocosts.cpp
	src/observation/test-khalil-2016.cpp

	src/dynamics/test-branching.cpp
	src/dynamics/test-configuring.cpp

	src/environment/test-environment.cpp
)

target_compile_definitions(
	test-libecole PRIVATE TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
)

target_include_directories(test-libecole PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(
	test-libecole
	PRIVATE
		Ecole::libecole
		Ecole::warnings
		Ecole::sanitizers
		CONAN_PKG::range-v3
		CONAN_PKG::catch2
		libscip
)

set_target_properties(test-libecole PROPERTIES
	# Compiling with hidden visibility
	CXX_VISIBILITY_PRESET hidden
	VISIBILITY_INLINES_HIDDEN ON
)
