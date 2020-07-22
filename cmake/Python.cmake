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
