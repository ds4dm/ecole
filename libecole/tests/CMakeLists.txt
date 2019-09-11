cmake_minimum_required(VERSION 3.5)

add_executable(run
               main.cpp
               src/scip/test-model.cpp)
target_compile_definitions(run PRIVATE
                           TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")
target_compile_options(ecole PRIVATE -Werror -Wall -Wextra)

target_include_directories(run PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_subdirectory("${CMAKE_SOURCE_DIR}/extern/Catch2"
                 "${CMAKE_BINARY_DIR}/Catch2" EXCLUDE_FROM_ALL)
target_link_libraries(run PRIVATE Catch2::Catch2 Ecole::ecole)

find_package(SCIP REQUIRED)
target_link_libraries(run PRIVATE libscip)
# set_test_properties( tests PROPERTIES ENVIRONMENT
# TEST_DATA_DIR="${CMAKE_CURRENT_SOUCRE_DIR}/data")
