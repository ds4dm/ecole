cmake_minimum_required(VERSION 3.5)

add_test(
	NAME test-ecole-python
	COMMAND venv-python -m pytest
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
