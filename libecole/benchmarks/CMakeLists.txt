add_executable(
	benchmark-libecole
	src/main.cpp
	src/benchmark.cpp
	src/bench-branching.cpp
)

target_include_directories(benchmark-libecole PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

find_or_download_package(
	NAME CLI11
	URL https://github.com/CLIUtils/CLI11/archive/v1.9.1.tar.gz
	URL_HASH SHA256=c780cf8cf3ba5ec2648a7eeb20a47e274493258f38a9b417628e0576f473a50b
	CONFIGURE_ARGS
		-D CLI11_BUILD_TESTS=OFF
		-D CLI11_BUILD_EXAMPLES=OFF
)

target_link_libraries(
	benchmark-libecole
	PRIVATE
		Ecole::libecole
		Ecole::warnings
		Ecole::sanitizers
		CLI11::CLI11
		fmt::fmt
)

set_target_properties(
	benchmark-libecole
	PROPERTIES
		CXX_VISIBILITY_PRESET hidden
		VISIBILITY_INLINES_HIDDEN ON
)
