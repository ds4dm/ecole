cmake_minimum_required(VERSION 3.14)

add_library(
	libecole
	src/scip/model.cpp
	src/scip/utils.cpp
	src/scip/variable.cpp
	src/scip/column.cpp
	src/scip/row.cpp
	src/scip/exception.cpp
	src/utility/reverse-control.cpp
	src/reward/isdone.cpp
	src/termination/whensolved.cpp
	src/environment/branching.cpp
	src/environment/exception.cpp
)
set_target_properties(libecole PROPERTIES OUTPUT_NAME ecole)

add_library(Ecole::libecole ALIAS libecole)

target_include_directories(
	libecole
	PUBLIC
		$<INSTALL_INTERFACE:include>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
)

find_package(SCIP REQUIRED)
find_package(Threads REQUIRED)
target_link_libraries(libecole PUBLIC libscip PRIVATE Threads::Threads)

conan_cmake_run(CONANFILE conanfile.txt BASIC_SETUP CMAKE_TARGETS NO_OUTPUT_DIRS SKIP_STD)
target_link_libraries(
	libecole
	PUBLIC
		libscip
		CONAN_PKG::xtensor
		CONAN_PKG::optional-lite
		CONAN_PKG::variant-lite
	PRIVATE
		Threads::Threads
)

target_compile_features(libecole PUBLIC cxx_std_14)
target_compile_options(
	libecole
	PRIVATE
		-Werror
		-Wall
		-Wextra
		-Wconversion
		-Wunreachable-code
		-Wuninitialized
		-pedantic-errors
		-Wold-style-cast
)
set_target_properties(libecole PROPERTIES
	# All code ending in a shared library should be made PIC
	POSITION_INDEPENDENT_CODE ON
	# Compiling wiht hidden visibility
	# FIXME use CMAKE_GENERATE_EXPORT_HEADER when supporting building shared library
	CXX_VISIBILITY_PRESET hidden
	CMAKE_VISIBILITY_INLINES_HIDDEN ON
)

# Add test if this is the main project and testing is enabled
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
	add_subdirectory(tests)
endif()
