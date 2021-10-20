# Module to enable code coverage

function(ecole_target_add_coverage target)

	set(supported_compilers "GNU" "Clang" "AppleClang")
	if(CMAKE_CXX_COMPILER_ID IN_LIST supported_compilers)
		option(COVERAGE "Enable coverage reporting for gcc/clang" FALSE)
		if(COVERAGE)
			target_compile_options("${target}" PRIVATE --coverage -O0 -g)
			target_link_libraries("${target}" PRIVATE --coverage)
		endif()
	endif()

endfunction()
