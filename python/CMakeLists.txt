cmake_minimum_required(VERSION 3.5)

# Use FindPython to override wrong pybind resolution
find_package(Python REQUIRED)
set(PYTHON_EXECUTABLE ${Python_EXECUTABLE})

find_package(pybind11 REQUIRED)

# Create all extension modules
foreach(submodule_name "observation" "scip" "base" "branching")
	set(submodule_target "py_${submodule_name}")
	pybind11_add_module(${submodule_target} "ecole/${submodule_name}.cpp")
	target_include_directories(
		${submodule_target}
		PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ecole
	)
	target_link_libraries(${submodule_target} PRIVATE Ecole::ecole)
	target_compile_features(${submodule_target} PUBLIC cxx_std_14)
	set_target_properties( ${submodule_target} PROPERTIES OUTPUT_NAME ${submodule_name})
endforeach(submodule_name)
