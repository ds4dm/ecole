cmake_minimum_required(VERSION 3.5)

# Moving files to binary dir to create python package
foreach(py_file "setup.py" "src/ecole/__init__.py")
	configure_file(
		"${CMAKE_CURRENT_SOURCE_DIR}/${py_file}.in" "${CMAKE_CURRENT_BINARY_DIR}/${py_file}"
		@ONLY
	)
endforeach(py_file)


find_package(Python COMPONENTS NumPy REQUIRED)
find_package(pybind11 REQUIRED)
find_package(xtensor REQUIRED)
find_package(xtensor-python REQUIRED)

pybind11_add_module(
	ecole-python
	src/ecole/core.cpp
	src/ecole/scip.cpp
	src/ecole/abstract.cpp
	src/ecole/observation/observation.cpp
	src/ecole/reward/reward.cpp
	src/ecole/termination/termination.cpp
	src/ecole/environment/environment.cpp
)

target_include_directories(ecole-python PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/ecole)

target_link_libraries(
	ecole-python
	PRIVATE
		Ecole::libecole
		Ecole::warnings
		Python::NumPy
		xtensor
		xtensor-python
)
if(CMAKE_BUILD_TYPE MATCHES RELEASE)
	target_link_libraries(ecole-python PRIVATE xtensor::optimize)
endif()

target_compile_features(ecole-python PUBLIC cxx_std_14)

set_target_properties(
	ecole-python PROPERTIES
	OUTPUT_NAME core
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


add_custom_command(
	TARGET ecole-python POST_BUILD
	COMMAND venv-python
	ARGS -m pip install --upgrade ${CMAKE_CURRENT_BINARY_DIR} > /dev/null
	COMMENT "Installing ecole in virtual environment"
)

# Add test if this is the main project and testing is enabled
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
	add_subdirectory(tests)
endif()
