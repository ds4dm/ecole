cmake_minimum_required(VERSION 3.14)

project(
	Ecole
	VERSION 0.0.2
	LANGUAGES CXX
	DESCRIPTION "Extensible Combinatorial Optimization Learning Environments"
)

include(CTest)  # Enable testing and set BUILD_TESTING
add_subdirectory(libecole)
add_subdirectory(python)
