if(NOT DEFINED Python_EXECUTABLE)
	# Find Python interpreter from the path and don't resolve symlinks
	# TODO: this is no longer required whrn switching to scikit-build
	execute_process(
		COMMAND "python3" "-c" "import sys; print(sys.executable)"
		OUTPUT_VARIABLE Python_EXECUTABLE
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
endif()
# Let CMake find the full package from the interpreter
find_package(Python COMPONENTS Interpreter Development NumPy REQUIRED)
