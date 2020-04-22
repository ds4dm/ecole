# Download and inlclude the conan CMake wrapper.
#
# The Conan executable still needs to be installed separately.


function(download_cmake_conan)
	set(CONAN_URL "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.15/conan.cmake")
	set(CONAN_SHA1 "d1f980d55623fd426fc3ad4c79ae0eba45141310")
	set(CONAN_PATH "${CMAKE_BINARY_DIR}/conan.cmake")

	# Include and exit if correct file exists
	if(EXISTS "${CONAN_PATH}")
		file(SHA1 "${CONAN_PATH}" LOCAL_SHA1)
		if("${LOCAL_SHA1}" STREQUAL "${CONAN_SHA1}")
			include("${CONAN_PATH}")
			return()
		endif()
	endif()

	# Download automatically, you can also just copy the conan.cmake file
	message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
	file(DOWNLOAD "${CONAN_URL}" "${CONAN_PATH}" EXPECTED_HASH SHA1=${CONAN_SHA1})

	include("${CONAN_PATH}")

endfunction()


download_cmake_conan()
