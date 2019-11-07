cmake_minimum_required(VERSION 3.14)

# Dependencies
find_package(SCIP REQUIRED)

add_library(
	ecole
	src/scip/model.cpp
	src/scip/utils.cpp
	src/scip/variable.cpp
	src/exception.cpp
	src/observation.cpp
	src/branching.cpp
	src/base/environment.cpp
)

add_library(Ecole::ecole ALIAS ecole)

target_include_directories(
	ecole
	PUBLIC
		$<INSTALL_INTERFACE:include>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(ecole PRIVATE libscip)
target_compile_features(ecole PUBLIC cxx_std_14)
target_compile_options(
	ecole
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
set_target_properties(ecole PROPERTIES
	# All code ending in a shared library should be made PIC
	POSITION_INDEPENDENT_CODE ON
	# Compiling wiht hidden visibility
	# FIXME use CMAKE_GENERATE_EXPORT_HEADER when supporting building shared library
	CXX_VISIBILITY_PRESET hidden
	CMAKE_VISIBILITY_INLINES_HIDDEN ON
)

add_subdirectory(tests)
