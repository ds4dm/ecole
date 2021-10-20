cmake_minimum_required(VERSION 3.15)

project(
	Branching
	VERSION 0.0.1
	LANGUAGES CXX
	DESCRIPTION "Branching with Ecole in C++"
)

add_executable(
	branching
	src/branching.cpp
)

find_package(Ecole REQUIRED)
target_link_libraries(branching Ecole::ecole-lib)
