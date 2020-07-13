# Module to enable compiler runtime checks.
#
# File taken from Jason Turner's cpp_starter_project
# https://github.com/lefticus/cpp_starter_project/blob/master/cmake/Sanitizers.cmake


function(enable_sanitizers project_name)

	set(SUPPORTED_COMPILERS "GNU" "Clang" "AppleClang")
	if(CMAKE_CXX_COMPILER_ID IN_LIST SUPPORTED_COMPILERS)
		set(SANITIZERS "")

		option(SANITIZE_ADDRESS "Enable address sanitizer" FALSE)
		if(SANITIZE_ADDRESS)
			list(APPEND SANITIZERS "address")
		endif()

		option(SANITIZE_MEMORY "Enable memory sanitizer" FALSE)
		if(SANITIZE_MEMORY)
			list(APPEND SANITIZERS "memory")
		endif()

		option(SANITIZE_UNDEFINED_BEHAVIOR "Enable undefined behavior sanitizer" FALSE)
		if(SANITIZE_UNDEFINED_BEHAVIOR)
			list(APPEND SANITIZERS "undefined")
		endif()

		option(SANITIZE_THREAD "Enable thread sanitizer" FALSE)
		if(SANITIZE_THREAD)
			list(APPEND SANITIZERS "thread")
		endif()

		list(JOIN SANITIZERS "," LIST_OF_SANITIZERS)

	endif()

	if(LIST_OF_SANITIZERS)
		if(NOT "${LIST_OF_SANITIZERS}" STREQUAL "")
			target_compile_options(${project_name} INTERFACE -fsanitize=${LIST_OF_SANITIZERS})
			target_link_libraries(${project_name} INTERFACE -fsanitize=${LIST_OF_SANITIZERS})
		endif()
	endif()

endfunction()


# Define a target with enabled sanitizers
add_library(ecole_sanitizers INTERFACE)
enable_sanitizers(ecole_sanitizers)
add_library(Ecole::sanitizers ALIAS ecole_sanitizers)
