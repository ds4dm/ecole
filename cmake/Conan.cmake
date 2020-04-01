# Download and inlclude the conan CMake wrapper.
#
# The Conan executable still needs to be installed separately.


function(download_cmake_conan)
	# Get Conan CMake wrapper for C++ package management
	if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
		# Download automatically, you can also just copy the conan.cmake file
		message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
		file(
			DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.15/conan.cmake"
			"${CMAKE_BINARY_DIR}/conan.cmake"
		)
	endif()
	include(${CMAKE_BINARY_DIR}/conan.cmake)

endfunction()


download_cmake_conan()
