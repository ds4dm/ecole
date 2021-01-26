add_executable(
	benchmark-libecole
	src/main.cpp
	src/benchmark.cpp
	src/bench-branching.cpp
)

target_include_directories(benchmark-libecole PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
find_package(nlohmann_json REQUIRED)

target_link_libraries(
	benchmark-libecole
	PRIVATE
		Ecole::libecole
		Ecole::warnings
		Ecole::sanitizers
		nlohmann_json::nlohmann_json
)

set_target_properties(
	benchmark-libecole
	PROPERTIES
		CXX_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
)
