# Enable tools for checking the code at compile time.

option(ENABLE_CLANG_TIDY "Enable static analysis with clang-tidy" OFF)

if(ENABLE_CLANG_TIDY)
	find_program(CLANGTIDY clang-tidy)
	if(CLANGTIDY)
		set(CMAKE_CXX_CLANG_TIDY ${CLANGTIDY})
		message(STATUS "Using clang-tidy")
	else()
		message(SEND_ERROR "Cannot find clang-tidy executable")
	endif()
endif()
