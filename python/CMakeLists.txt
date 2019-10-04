cmake_minimum_required(VERSION 3.5)

add_subdirectory("extern/pybind11")

pybind11_add_module(ecole_python src/learn2branch.cpp)
set_target_properties(ecole_python PROPERTIES OUTPUT_NAME ecole)

target_link_libraries(ecole_python PRIVATE Ecole::ecole)
