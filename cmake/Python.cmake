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

# Find the Python Interpreter
find_package(Python COMPONENTS Interpreter REQUIRED)
# Use FindPython to override wrong PyBind resolution
set(PYTHON_EXECUTABLE ${Python_EXECUTABLE})

# Create a virtual environment (at configure time) where Ecole can safely be installed
set(VENV_DIR "${CMAKE_CURRENT_BINARY_DIR}/venv")
execute_process(
	COMMAND "${Python_EXECUTABLE}" -m venv --system-site-package "${VENV_DIR}"
	RESULT_VARIABLE VENV_ERROR
	ERROR_VARIABLE VENV_ERROR_MESSAGE
)
# Report on virtual environment creation
if(VENV_ERROR)
	message(SEND_ERROR "${VENV_ERROR_MESSAGE}")
else()
	message(STATUS "Created Python virtual environment")
endif()

# Upgrade Pip inside virtual environment to remove warning
execute_process(
	COMMAND "${VENV_DIR}/bin/pip" install --quiet --upgrade pip
	RESULT_VARIABLE VENV_PIP_ERROR
	ERROR_VARIABLE VENV_PIP_ERROR_MESSAGE
)
# Report on pip upgrade
if(VENV_PIP_ERROR)
	message(SEND_ERROR "${VENV_PIP_ERROR_MESSAGE}")
else()
	message(STATUS "Upgraded pip in virtual environment")
endif()

# Add a target for the Python of the virtual environment
add_executable(venv-python IMPORTED)
set_target_properties(
	venv-python
	PROPERTIES IMPORTED_LOCATION "${VENV_DIR}/bin/python"
)
