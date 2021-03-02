# Find Python interpreter from the path and don't resolve symlinks.
execute_process(
	COMMAND "python3" "-c" "import sys; print(sys.executable)"
	OUTPUT_VARIABLE Python_EXECUTABLE
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
# Let CMake find the full package from the interpreter
find_package(Python COMPONENTS Interpreter Development NumPy REQUIRED)
