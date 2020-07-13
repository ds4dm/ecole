# Module to enable code coverage

function(enable_coverage project_name)

	set(SUPPORTED_COMPILERS "GNU" "Clang" "AppleClang")
	if(CMAKE_CXX_COMPILER_ID IN_LIST SUPPORTED_COMPILERS)
		option(COVERAGE "Enable coverage reporting for gcc/clang" FALSE)

		if(COVERAGE)
			target_compile_options(${project_name} INTERFACE --coverage -O0 -g)
			target_link_libraries(${project_name} INTERFACE --coverage)
		endif()
	endif()

endfunction()


# Define a target with enabled coverage
add_library(ecole_coverage INTERFACE)
enable_coverage(ecole_coverage)
add_library(Ecole::coverage ALIAS ecole_coverage)
