# This file, when inlcuded with -D CMAKE_TOOLCHAIN_FILE=... is exectued first.
# Toolchain files are used to set the compiler and other build tools (usually for
# cross-compiling).
# Here, we use it to set the AR and RANLIB, not detected automatically by CMake but set
# by conda.

macro(set_from_env cmake_var env_var)
	if(DEFINED ENV{${env_var}})
		message(STATUS "Setting ${cmake_var} to $ENV{${env_var}}")
		set(${cmake_var} $ENV{${env_var}})
	endif()
endmacro()

set_from_env(CMAKE_AR AR)
set_from_env(CMAKE_RANLIB RANLIB)
