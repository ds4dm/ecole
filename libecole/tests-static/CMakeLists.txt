cmake_minimum_required(VERSION 3.5)

add_executable(
	test-static-libecole
	main.cpp
	src/test-traits.cpp
)

target_include_directories(test-static-libecole PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(
	test-static-libecole
	PRIVATE
		Ecole::libecole
		Ecole::warnings
)

set_target_properties(test-static-libecole PROPERTIES
	# Compiling with hidden visibility
	CXX_VISIBILITY_PRESET hidden
	VISIBILITY_INLINES_HIDDEN ON
)
