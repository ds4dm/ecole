option(ENABLE_PYTHON_VENV "Install Ecole Python library in a virtual environment" OFF)

if(NOT ENABLE_PYTHON_VENV)
	return()
endif()

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
