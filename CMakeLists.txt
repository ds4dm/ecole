cmake_minimum_required(VERSION 3.14)

project(
	Ecole
	VERSION 0.0.2
	LANGUAGES CXX
	DESCRIPTION "Extensible Combinatorial Optimization Learning Environments"
)

string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE)

# Get Conan CMake wrapper for C++ package management
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
	# Download automatically, you can also just copy the conan.cmake file
	message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
	file(
		DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.15/conan.cmake"
		"${CMAKE_BINARY_DIR}/conan.cmake"
	)
endif()
include(${CMAKE_BINARY_DIR}/conan.cmake)

include(CTest)  # Enable testing and set BUILD_TESTING
add_subdirectory(libecole)
add_subdirectory(python)
