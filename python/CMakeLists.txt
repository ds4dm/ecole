# Moving files to binary dir to create python package
foreach(py_file "setup.py" "src/ecole/__init__.py")
	configure_file(
		"${CMAKE_CURRENT_SOURCE_DIR}/${py_file}.in" "${CMAKE_CURRENT_BINARY_DIR}/${py_file}"
		@ONLY
	)
endforeach(py_file)

find_or_download_package(
	NAME pybind11
	URL https://github.com/pybind/pybind11/archive/v2.6.2.tar.gz
	URL_HASH SHA256=8ff2fff22df038f5cd02cea8af56622bc67f5b64534f1b83b9f133b8366acff2
	CONFIGURE_ARGS -D PYBIND11_TEST=OFF
)
find_package(xtensor REQUIRED) # It's already found by libecole but this seems to be required
find_or_download_package(
	NAME xtensor-python
	URL https://github.com/xtensor-stack/xtensor-python/archive/0.25.1.tar.gz
	URL_HASH SHA256=1e70db455a4dcba226c450bf9261a05a0c2fad513b84be35a3d139067356e6a1
	CONFIGURE_ARGS -D BUILD_TESTS=OFF
)

pybind11_add_module(
	ecole-python
	src/ecole/core/core.cpp
	src/ecole/core/scip.cpp
	src/ecole/core/instance.cpp
	src/ecole/core/data.cpp
	src/ecole/core/observation.cpp
	src/ecole/core/reward.cpp
	src/ecole/core/information.cpp
	src/ecole/core/dynamics.cpp
)

target_include_directories(ecole-python PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/ecole/core)

# Include the headers directly instead of using the CMake target due to it wrongly linking against
# libpython
target_include_directories(ecole-python SYSTEM PRIVATE "${Python_NumPy_INCLUDE_DIRS}")

target_link_libraries(
	ecole-python
	PRIVATE
		Ecole::libecole
		Ecole::warnings
		xtensor-python
)

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


set(
	PYTHON_FILES
	"py.typed" "typing.py"
	"version.py" "scip.py" "instance.py"
	"data.py" "observation.py" "reward.py" "information.py" "dynamics.py" "environment.py"
)
set(PYTHON_SOURCE_FILES ${PYTHON_FILES})
list(TRANSFORM PYTHON_SOURCE_FILES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/src/ecole/")
set(PYTHON_COPIED_FILES ${PYTHON_FILES})
list(TRANSFORM PYTHON_COPIED_FILES PREPEND "${CMAKE_CURRENT_BINARY_DIR}/src/ecole/")

add_custom_target(
	ecole-python-files
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PYTHON_SOURCE_FILES} "${CMAKE_CURRENT_BINARY_DIR}/src/ecole"
	COMMENT "Copying Python files"
)
add_dependencies(ecole-python ecole-python-files)

if(ENABLE_PYTHON_VENV)
	add_custom_command(
		OUTPUT "${CMAKE_BINARY_DIR}/venv"
		DEPENDS ecole-python ecole-python-files
		COMMAND venv-python -m pip install --upgrade ${CMAKE_CURRENT_BINARY_DIR} > /dev/null
		COMMENT "Installing ecole in virtual environment"
	)

	add_custom_target(ecole-venv ALL DEPENDS "${CMAKE_BINARY_DIR}/venv")
endif()
