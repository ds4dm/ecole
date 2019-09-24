cmake_minimum_required(VERSION 3.14)
project(
  Ecole
  VERSION 0.0.1
  LANGUAGES CXX
  DESCRIPTION "ECOLE are Adaptive Combinatorial Optimization Environments")

# Dependencies
find_package(SCIP REQUIRED)

add_library(ecole src/scip/model.cpp src/scip/utils.cpp)

add_library(Ecole::ecole ALIAS ecole)

target_include_directories(
  ecole
  PUBLIC $<INSTALL_INTERFACE:include>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(ecole PRIVATE libscip)
target_compile_features(ecole PUBLIC cxx_std_14)
target_compile_options(ecole
                       PRIVATE -Werror
                               -Wall
                               -Wextra
                               -Wconversion
                               -Wunreachable-code
                               -Wuninitialized
                               -pedantic-errors
                               -Wold-style-cast)

add_subdirectory(tests)
