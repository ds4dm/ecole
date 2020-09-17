# Module to set default compiler warnings.
#
# File taken from Jason Turner's cpp_starter_project
# https://github.com/lefticus/cpp_starter_project/blob/master/cmake/CompilerWarnings.cmake

function(set_project_warnings project_name)
	option(WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

	set(MSVC_WARNINGS
		# Baseline reasonable warnings
		/W4
		# "identfier": conversion from "type1" to "type1", possible loss of data
		/w14242
		# "operator": conversion from "type1:field_bits" to "type2:field_bits", possible
		# loss of data
		/w14254
		# "function": member function does not override any base class virtual member
		# function
		/w14263
		# "classname": class has virtual functions, but destructor is not virtual instances
		# of this class may not be destructed correctly
		/w14265
		# "operator": unsigned/negative constant mismatch
		/w14287
		# Nonstandard extension used: "variable": loop control variable declared in the
		# for-loop is used outside the for-loop scope
		/we4289
		# "operator": expression is always "boolean_value"
		/w14296
		# "variable": pointer truncation from "type1" to "type2"
		/w14311
		# Expression before comma evaluates to a function which is missing an argument list
		/w14545
		# Function call before comma missing argument list
		/w14546
		# "operator": operator before comma has no effect; expected operator with side-effect
		/w14547
		# "operator": operator before comma has no effect; did you intend "operator"?
		/w14549
		# Expression has no effect; expected expression with side- effect
		/w14555
		# Pragma warning: there is no warning number "number"
		/w14619
		# Enable warning on thread un-safe static member initialization
		/w14640
		# Conversion from "type1" to "type_2" is sign-extended. This may cause unexpected
		# runtime behavior.
		/w14826
		# Wide string literal cast to "LPSTR"
		/w14905
		# String literal cast to "LPWSTR"
		/w14906
		# Illegal copy-initialization; more than one user-defined conversion has been
		# implicitly applied
		/w14928
	)

	set(CLANG_WARNINGS
		# Some default set of warnings
		-Wall
		# Reasonable and standard
		-Wextra
		# Warn the user if a variable declaration shadows one from a parent context
		-Wshadow
		# Warn the user if a class with virtual functions has a non-virtual destructor.
		# This helps catch hard to track down memory errors
		-Wnon-virtual-dtor
		# Warn for c-style casts
		-Wold-style-cast
		# Warn for potential performance problem casts
		-Wcast-align
		# Warn on anything being unused
		-Wunused
		# Warn if you overload (not override) a virtual function
		-Woverloaded-virtual
		# Warn if non-standard C++ is used
		-Wpedantic
		# Warn on type conversions that may lose data
		-Wconversion
		# Warn on sign conversions
		-Wsign-conversion
		# Warn if a null dereference is detected
		-Wnull-dereference
		# Warn if float is implicit promoted to double
		-Wdouble-promotion
		# Warn on security issues around functions that format output (ie printf)
		-Wformat=2
		# Warn on code that cannot be executed
		-Wunreachable-code
		# Warn if a variable is used before being initialized
		-Wuninitialized
	)

	if (WARNINGS_AS_ERRORS)
		set(CLANG_WARNINGS ${CLANG_WARNINGS} -Werror)
		set(MSVC_WARNINGS ${MSVC_WARNINGS} /WX)
	endif()

	set(GCC_WARNINGS
		${CLANG_WARNINGS}
		# FIXME currently not adding more warning for GCC because they fail on clang-tidy
		# warn if identation implies blocks where blocks do not exist
		# -Wmisleading-indentation
		# warn if if / else chain has duplicated conditions
		# -Wduplicated-cond
		# warn if if / else branches have duplicated code
		# -Wduplicated-branches
		# warn about logical operations being used where bitwise were probably wanted
		# -Wlogical-op
		# warn if you perform a cast to the same type
		# -Wuseless-cast
	)

	if(MSVC)
		set(PROJECT_WARNINGS ${MSVC_WARNINGS})
	elseif(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
		set(PROJECT_WARNINGS ${CLANG_WARNINGS})
	elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		set(PROJECT_WARNINGS ${CLANG_WARNINGS})
	else()
		set(PROJECT_WARNINGS ${GCC_WARNINGS})
	endif()

	target_compile_options(${project_name} INTERFACE ${PROJECT_WARNINGS})

endfunction()


# Define a target with all compiler warnings
add_library(ecole_warnings INTERFACE)
set_project_warnings(ecole_warnings)
add_library(Ecole::warnings ALIAS ecole_warnings)
