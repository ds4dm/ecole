cmake_minimum_required(VERSION 3.5)

# Moving files to binary dir to create python package
foreach(py_file "setup.py" "src/ecole/__init__.py")
	configure_file(
		"${CMAKE_CURRENT_SOURCE_DIR}/${py_file}.in" "${CMAKE_CURRENT_BINARY_DIR}/${py_file}"
		@ONLY
	)
endforeach(py_file)

# Prefer finding Python from path if possible
if(NOT DEFINED Python_ROOT_DIR)
	execute_process(
		COMMAND which python
		COMMAND xargs dirname  # Pipe
		OUTPUT_VARIABLE Python_ROOT_DIR
		RESULTS_VARIABLE Python_IN_PATH
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
endif()
find_package(Python COMPONENTS Interpreter NumPy REQUIRED)
# Use FindPython to override wrong pybind resolution
set(PYTHON_EXECUTABLE ${Python_EXECUTABLE})

find_package(pybind11 REQUIRED)
find_package(xtensor REQUIRED)
find_package(xtensor-python REQUIRED)

pybind11_add_module(
	ecole-python
	src/ecole/core.cpp
	src/ecole/scip.cpp
	src/ecole/base.cpp
	src/ecole/observation.cpp
	src/ecole/reward.cpp
	src/ecole/termination.cpp
	src/ecole/configuring.cpp
	src/ecole/branching.cpp
)

target_include_directories(ecole-python PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/ecole)

target_link_libraries(
	ecole-python PRIVATE Ecole::libecole xtensor xtensor-python Python::NumPy
)
if(CMAKE_BUILD_TYPE MATCHES RELEASE)
	target_link_libraries(ecole-python PRIVATE xtensor::optimize)
endif()

target_compile_features(ecole-python PUBLIC cxx_std_14)

set_target_properties(
	ecole-python PROPERTIES
	OUTPUT_NAME core
	SETUP_DIR ${CMAKE_CURRENT_BINARY_DIR}
)
# If no output directory specified, preserve the src/ecole layout
if(DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
	set_target_properties(
		ecole-python PROPERTIES
		LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
	)
else()
	set_target_properties(
		ecole-python PROPERTIES
		LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/src/ecole"
	)
endif()

# Add test if this is the main project and testing is enabled
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
		add_subdirectory(tests)
endif()
