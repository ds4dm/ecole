cmake_minimum_required(VERSION 3.5)

add_executable(
	run
	main.cpp
	src/conftest.cpp
	src/scip/test-model.cpp
	src/scip/test-variable.cpp
	src/scip/test-view.cpp
	src/test-branching.cpp
	src/test-configuring.cpp
)
target_compile_definitions(
	run PRIVATE TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
)
target_compile_options(ecole PRIVATE -Werror -Wall -Wextra)

target_include_directories(run PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

find_package(Catch2 REQUIRED)
find_package(SCIP REQUIRED)
target_link_libraries(run PRIVATE Catch2::Catch2 Ecole::ecole libscip)
