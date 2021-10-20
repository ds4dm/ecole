# Module to enable compiler runtime checks.
#
# File adapted from Jason Turner's cpp_starter_project
# https://github.com/lefticus/cpp_starter_project/blob/master/cmake/Sanitizers.cmake
# Using INTERFACE targets is not so desirable as they need to be installed when building
# static libraries.

function(ecole_target_add_sanitizers target)

	set(supported_compilers "GNU" "Clang" "AppleClang")
	if(CMAKE_CXX_COMPILER_ID IN_LIST supported_compilers)
		set(sanitizers "")

		option(SANITIZE_ADDRESS "Enable address sanitizer" FALSE)
		if(SANITIZE_ADDRESS)
			list(APPEND sanitizers "address")
		endif()

		option(SANITIZE_MEMORY "Enable memory sanitizer" FALSE)
		if(SANITIZE_MEMORY)
			list(APPEND sanitizers "memory")
		endif()

		option(SANITIZE_UNDEFINED_BEHAVIOR "Enable undefined behavior sanitizer" FALSE)
		if(SANITIZE_UNDEFINED_BEHAVIOR)
			list(APPEND sanitizers "undefined")
		endif()

		option(SANITIZE_THREAD "Enable thread sanitizer" FALSE)
		if(SANITIZE_THREAD)
			list(APPEND sanitizers "thread")
		endif()

		list(JOIN sanitizers "," list_of_sanitizers)
		if(NOT "${list_of_sanitizers}" STREQUAL "")
			target_compile_options("${target}" PRIVATE -fsanitize=${list_of_sanitizers})
			target_link_libraries("${target}" PRIVATE -fsanitize=${list_of_sanitizers})
		endif()

	endif()

endfunction()
