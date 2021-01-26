add_executable(
	benchmark-libecole
	src/main.cpp
	src/benchmark.cpp
	src/bench-branching.cpp
)

target_include_directories(benchmark-libecole PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(
	benchmark-libecole
	PRIVATE
		Ecole::libecole
		Ecole::warnings
		Ecole::sanitizers
)

set_target_properties(
	benchmark-libecole
	PROPERTIES
		CXX_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
)
