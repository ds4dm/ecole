# Find or doawnload dependencies.
#
# Utility to try to find a package, or downloaad it, configure it, and install it inside
# the build tree.
# Based and fetch content, it avoids using `add_subdirectory` which exposes other project
# targets and errors as part of this project.


include(FetchContent)


# Where downloaded dependencies will be installed (in the build tree by default).
set(FETCHCONTENT_INSTALL_DIR "${FETCHCONTENT_BASE_DIR}/local")
option(ECOLE_FORCE_DOWNLOAD "Don't look for dependencies locally, rather always download them." OFF)


# Execute command at comfigure time and handle errors and output.
function(execute_process_handle_output)
	execute_process(
		${ARGV}
		RESULT_VARIABLE ERROR
		OUTPUT_VARIABLE STD_OUT
		ERROR_VARIABLE STD_ERR
	)
	if(ERROR)
		message(FATAL_ERROR "${STD_OUT} ${STD_ERR}")
	else()
		message(DEBUG "${STD_OUT}")
	endif()
endfunction()


# Configure, build and install the a CMake project
#
# The source of the project must have been made available prior to calling this function.
function(build_package)
	set(options)
	set(oneValueArgs SOURCE_DIR BUILD_DIR INSTALL_DIR)
	set(multiValueArgs CONFIGURE_ARGS)
	cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	message(DEBUG "${CMAKE_COMMAND}" -S "${ARG_SOURCE_DIR}" -B "${ARG_BUILD_DIR}" ${ARG_CONFIGURE_ARGS})
	execute_process_handle_output(
		COMMAND "${CMAKE_COMMAND}" -S "${ARG_SOURCE_DIR}" -B "${ARG_BUILD_DIR}" -G "${CMAKE_GENERATOR}" ${ARG_CONFIGURE_ARGS}
	)
	execute_process_handle_output(COMMAND "${CMAKE_COMMAND}" --build "${ARG_BUILD_DIR}" --parallel)
	execute_process_handle_output(COMMAND "${CMAKE_COMMAND}" --install "${ARG_BUILD_DIR}" --prefix "${ARG_INSTALL_DIR}")
endfunction()


# Try to find a package or downloads it at configure time.
#
# Use FetchContent to download a package if it was not found and build it inside the build tree.
function(find_or_download_package)
	set(options)
	set(oneValueArgs NAME URL URL_HASH)
	set(multiValueArgs CONFIGURE_ARGS)
	cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	if(${ARG_NAME}_FOUND)
		return()
	endif()

	if(NOT ECOLE_FORCE_DOWNLOAD)
		find_package(${ARG_NAME} QUIET)
	endif()

	if(${ARG_NAME}_FOUND)
		message(STATUS "Found ${ARG_NAME}")
	else()
		FetchContent_Declare(
			${ARG_NAME}
			URL ${ARG_URL}
			URL_HASH ${ARG_URL_HASH}
		)
		FetchContent_GetProperties(${ARG_NAME})
		if(NOT ${ARG_NAME}_POPULATED)
			message(STATUS "Downloading ${ARG_NAME}")
			FetchContent_Populate(${ARG_NAME})
			message(STATUS "Building ${ARG_NAME}")
			# FetchContent_Populate uses lower case name of FetchContent_Declare for directories
			string(TOLOWER "${ARG_NAME}" ARG_NAME_LOWER)
			build_package(
				CONFIGURE_ARGS ${ARG_CONFIGURE_ARGS} -D "CMAKE_PREFIX_PATH=${FETCHCONTENT_INSTALL_DIR}"
				SOURCE_DIR "${${ARG_NAME_LOWER}_SOURCE_DIR}"
				BUILD_DIR "${${ARG_NAME_LOWER}_BINARY_DIR}"
				INSTALL_DIR "${FETCHCONTENT_INSTALL_DIR}"
			)
			find_package(${ARG_NAME} PATHS "${FETCHCONTENT_INSTALL_DIR}" NO_DEFAULT_PATH QUIET)
		endif()
	endif()
endfunction()
