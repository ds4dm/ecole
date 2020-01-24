cmake_minimum_required(VERSION 3.5)

add_executable(
	test-libecole
	main.cpp
	src/conftest.cpp
	src/scip/test-model.cpp
	src/scip/test-variable.cpp
	src/scip/test-column.cpp
	src/scip/test-view.cpp
	src/test-branching.cpp
	src/test-configuring.cpp
)
target_compile_definitions(
	test-libecole PRIVATE TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
)
target_compile_options(test-libecole PRIVATE -Werror -Wall -Wextra)

target_include_directories(test-libecole PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

find_package(Catch2 REQUIRED)
find_package(SCIP REQUIRED)
target_link_libraries(test-libecole PRIVATE Catch2::Catch2 Ecole::libecole libscip)

# Register tests in CTest
include(Catch)
catch_discover_tests(test-libecole)
