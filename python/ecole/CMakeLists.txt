find_package(Python COMPONENTS Interpreter Development NumPy REQUIRED)
find_package(pybind11 REQUIRED)
find_package(xtensor REQUIRED)
find_package(xtensor-python REQUIRED)

pybind11_add_module(
	ecole-py-ext
	src/ecole/core/core.cpp
	src/ecole/core/version.cpp
	src/ecole/core/scip.cpp
	src/ecole/core/instance.cpp
	src/ecole/core/data.cpp
	src/ecole/core/observation.cpp
	src/ecole/core/reward.cpp
	src/ecole/core/information.cpp
	src/ecole/core/dynamics.cpp
)

target_include_directories(
	ecole-py-ext
		PRIVATE
			${CMAKE_CURRENT_SOURCE_DIR}/src/ecole/core
		SYSTEM PRIVATE
			# Include the headers directly instead of using the CMake target due to it wrongly
			# linking against libpython
			"${Python_NumPy_INCLUDE_DIRS}"
)

target_link_libraries(
	ecole-py-ext
	PRIVATE
		Ecole::ecole-lib
		Ecole::ecole-py-ext-helper
		xtensor-python
)

ecole_target_add_compile_warnings(ecole-py-ext)

set_target_properties(
	ecole-py-ext PROPERTIES
	OUTPUT_NAME core
)
# If no output directory specified, preserve the ecole layout in the build tree
if(NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
	set_target_properties(
		ecole-py-ext PROPERTIES
		LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/ecole"
	)
endif()


set(
	python_files
	__init__.py
	py.typed
	typing.py
	version.py
	doctor.py
	scip.py
	instance.py
	data.py
	observation.py
	reward.py
	information.py
	dynamics.py
	environment.py
)
set(PYTHON_SOURCE_FILES ${python_files})
list(TRANSFORM PYTHON_SOURCE_FILES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/src/ecole/")

add_custom_target(
	ecole-py-files
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PYTHON_SOURCE_FILES} "${CMAKE_CURRENT_BINARY_DIR}/ecole"
	COMMENT "Copying Python files"
)
add_dependencies(ecole-py-ext ecole-py-files)

# Scikit build relies on an installation of the Python module to find the library.
# Set through the setup.py script.
if(SKBUILD)
	# Rpath for the Ecole Python extension module
	if(NOT ECOLE_PY_EXT_INSTALL_RPATH)
		set(ECOLE_PY_EXT_INSTALL_LIBDIR "\\\${ORIGIN}/../${CMAKE_INSTALL_LIBDIR}")
	endif()
	set_target_properties(
		ecole-py-ext
		PROPERTIES
			INSTALL_RPATH "${ECOLE_PY_EXT_INSTALL_RPATH}"
	)

	# Relative (to install prefix) path where to install the Python extension module
	if(NOT ECOLE_PY_EXT_INSTALL_LIBDIR)
		set(ECOLE_PY_EXT_INSTALL_LIBDIR "ecole")
	endif()
	install(
		TARGETS ecole-py-ext
		DESTINATION "${ECOLE_PY_EXT_INSTALL_LIBDIR}"
		COMPONENT Ecole_Python_Extension
	)
endif()
