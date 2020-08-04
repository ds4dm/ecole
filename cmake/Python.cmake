# Prefer finding Python from path if not defined
if(NOT DEFINED PYTHON_EXECUTABLE)
	find_program (PYTHON_EXECUTABLE NAMES python3 python NO_DEFAULT_PATH)
endif()
