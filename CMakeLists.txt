cmake_minimum_required(VERSION 3.14)

project(
	Ecole
	VERSION 0.0.2
	LANGUAGES CXX
	DESCRIPTION "Extensible Combinatorial Optimization Learning Environments"
)

add_subdirectory(libecole)
add_subdirectory(python)
