cmake_minimum_required(VERSION 3.5)

# Use FindPython to override wrong pybind resolution
find_package(Python REQUIRED)
set(PYTHON_EXECUTABLE ${Python_EXECUTABLE})

find_package(pybind11 REQUIRED)

# Create all extension modules
foreach(submodule_name "observation" "scip" "base" "branching")
	set(submodule_target "py_${submodule_name}")
	pybind11_add_module(${submodule_target} "src/ecole/${submodule_name}.cpp")
	target_include_directories(
		${submodule_target}
		PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/ecole
	)
	target_link_libraries(${submodule_target} PRIVATE Ecole::ecole)
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

# Moving files to binary dir to create python package
foreach(py_file "setup.py" "src/ecole/__init__.py")
	configure_file(
		"${CMAKE_CURRENT_SOURCE_DIR}/${py_file}.in" "${CMAKE_CURRENT_BINARY_DIR}/${py_file}"
		@ONLY
	)
endforeach(py_file)
