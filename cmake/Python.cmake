# Set some variables to find the proper Python version

if(SKBUILD)
	# If scikit-build is compiling, let if define the interpreter
	set(Python_EXECUTABLE "${PYTHON_EXECUTABLE}")
	set(Python_INCLUDE_DIR "${PYTHON_INCLUDE_DIR}")
	set(Python_LIBRARY "${PYTHON_LIBRARY}")
	set(DUMMY "${PYTHON_VERSION_STRING}")  # Not needed, silences a warning

elseif(NOT DEFINED Python_EXECUTABLE)
	# Find Python interpreter from the path and don't resolve symlinks
	execute_process(
		COMMAND "python3" "-c" "import sys; print(sys.executable)"
		OUTPUT_VARIABLE Python_EXECUTABLE
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
endif()
