cmake_minimum_required(VERSION 3.5)

add_executable(venv-python IMPORTED)
set_target_properties(
	venv-python
	PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/venv/bin/python"
)

add_custom_command(
	OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/venv"
	COMMAND Python::Interpreter
		ARGS -m venv --clear "${CMAKE_CURRENT_BINARY_DIR}/venv" > /dev/null
	COMMAND venv-python ARGS -m pip install --upgrade pip pytest > /dev/null
	COMMENT "Creating virtual environment"
)

add_custom_target(
	virtualenv ALL
	COMMAND venv-python -m pip install -I $<TARGET_PROPERTY:pyecole,SETUP_DIR> > /dev/null
	DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/venv"
	DEPENDS pyecole
	COMMENT "Installing ecole in virtual environment"
)

add_test(
	NAME test-pyecole
	COMMAND "${CMAKE_CURRENT_BINARY_DIR}/venv/bin/python" -m pytest
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
