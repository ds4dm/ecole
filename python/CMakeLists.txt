cmake_minimum_required(VERSION 3.5)

add_subdirectory("extern/pybind11")

foreach(submodule_name "branching" "observation" "scip")
	set(submodule_target "py_${submodule_name}")
	pybind11_add_module(${submodule_target} "src/${submodule_name}.cpp")
	target_include_directories(
		${submodule_target}
		PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
	)
	target_compile_features(${submodule_target} PUBLIC cxx_std_14)
	set_target_properties(
		${submodule_target} PROPERTIES
		OUTPUT_NAME ${submodule_name}
		LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/ecole"
	)
	target_link_libraries(${submodule_target} PRIVATE Ecole::ecole)
endforeach(submodule_name)
