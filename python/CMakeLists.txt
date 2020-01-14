cmake_minimum_required(VERSION 3.5)

# Names of all Python modules built for Ecole
set(
	Ecole_PYTHON_MODULE_NAMES
	"observation" "scip" "base" "branching" "configuring" "reward" "termination"
)

# Moving files to binary dir to create python package
foreach(py_file "setup.py" "src/ecole/__init__.py")
	configure_file(
		"${CMAKE_CURRENT_SOURCE_DIR}/${py_file}.in" "${CMAKE_CURRENT_BINARY_DIR}/${py_file}"
		@ONLY
	)
endforeach(py_file)

# Abstract target to build all python submodules
add_custom_target(pyecole ALL)
set_target_properties(pyecole PROPERTIES SETUP_DIR ${CMAKE_CURRENT_BINARY_DIR})

# Use FindPython to override wrong pybind resolution
find_package(Python REQUIRED)
set(PYTHON_EXECUTABLE ${Python_EXECUTABLE})

find_package(pybind11 REQUIRED)
find_package(xtensor REQUIRED)
find_package(xtensor-python REQUIRED)

foreach(submodule_name ${Ecole_PYTHON_MODULE_NAMES})
	set(submodule_target "py-${submodule_name}")
	pybind11_add_module(${submodule_target} "src/ecole/${submodule_name}.cpp")
	add_dependencies(pyecole ${submodule_target})
	target_include_directories(
		${submodule_target}
		PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/ecole
	)
	target_link_libraries(${submodule_target} PRIVATE Ecole::libecole xtensor xtensor-python)
	target_compile_features(${submodule_target} PUBLIC cxx_std_14)
	set_target_properties( ${submodule_target} PROPERTIES OUTPUT_NAME ${submodule_name})
	# If no output directory specified, preserve the src/ecole layout
	if(DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
		set_target_properties(
			${submodule_target} PROPERTIES
			LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
		)
	else()
		set_target_properties(
			${submodule_target} PROPERTIES
			LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/src/ecole"
		)
	endif()
endforeach(submodule_name)

# Add test if this is the main project and testing is enabled
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
		add_subdirectory(tests)
endif()
