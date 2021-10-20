add_executable(
	ecole-lib-benchmark
	src/main.cpp
	src/benchmark.cpp
	src/bench-branching.cpp
)

target_include_directories(ecole-lib-benchmark PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# File that download the dependencies of libecole
include(dependencies/private.cmake)

find_package(CLI11 REQUIRED)

target_link_libraries(
	ecole-lib-benchmark
	PRIVATE
		Ecole::ecole-lib
		CLI11::CLI11
		fmt::fmt
)

ecole_target_add_compile_warnings(ecole-lib)
ecole_target_add_sanitizers(ecole-lib)

set_target_properties(
	ecole-lib-benchmark
	PROPERTIES
		CXX_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
)
