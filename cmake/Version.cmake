function(read_version file version_var)
	file(READ "${file}" version_text)

	string(REGEX MATCH "VERSION_MAJOR ([0-9]+)" _ "${version_text}")
	set(version_major "${CMAKE_MATCH_1}")

	string(REGEX MATCH "VERSION_MINOR ([0-9]+)" _ "${version_text}")
	set(version_minor "${CMAKE_MATCH_1}")

	string(REGEX MATCH "VERSION_PATCH ([0-9]+)" _ "${version_text}")
	set(version_patch "${CMAKE_MATCH_1}")

	set("${version_var}" "${version_major}.${version_minor}.${version_patch}" PARENT_SCOPE)
	message(STATUS "Ecole version ${version_major}.${version_minor}.${version_patch}")
endfunction()
