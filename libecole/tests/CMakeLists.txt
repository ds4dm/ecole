cmake_minimum_required(VERSION 3.5)

add_executable(
	test-libecole
	main.cpp
	src/conftest.cpp
	src/scip/test-model.cpp
	src/scip/test-variable.cpp
	src/scip/test-column.cpp
	src/scip/test-row.cpp
	src/scip/test-view.cpp
	src/test-branching.cpp
	src/test-configuring.cpp
)
target_compile_definitions(
	test-libecole PRIVATE TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
)
target_compile_options(test-libecole PRIVATE -Werror -Wall -Wextra)

target_include_directories(test-libecole PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

conan_cmake_run(CONANFILE conanfile.txt BASIC_SETUP CMAKE_TARGETS NO_OUTPUT_DIRS SKIP_STD)
find_package(SCIP REQUIRED)
target_link_libraries(test-libecole PRIVATE CONAN_PKG::Catch2 Ecole::libecole libscip)
