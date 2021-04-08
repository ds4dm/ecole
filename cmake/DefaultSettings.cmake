# Set default parameters depending if user or developer.


# Set the default build type to the given value if no build type was specified
function(set_default_build_type DEFAULT_BUILD_TYPE)
	if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
		message(STATUS "Setting build type to ${DEFAULT_BUILD_TYPE} as none was specified")
		set(
			CMAKE_BUILD_TYPE ${DEFAULT_BUILD_TYPE}
			CACHE STRING "Choose the type of build" FORCE
		)
		# Set the possible values of build type for cmake-gui, ccmake
		set_property(
			CACHE CMAKE_BUILD_TYPE
			PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo"
		)
	endif()
endfunction()

# Set default common for all cases
macro(set_common_defaults)
	option(CMAKE_POSITION_INDEPENDENT_CODE "Position Independent Code for building shared libraries." ON)
	option(CMAKE_VISIBILITY_INLINES_HIDDEN "Hidden symbol visibility for inline functions in shared libraries" ON)
	set(CMAKE_CXX_VISIBILITY_PRESET hidden CACHE STRING "Hidden visibility of symbols in shared libraries.")
	option(ECOLE_BENCHMARK "Build Ecole benchmakrs" OFF)
endmacro()


# Set of defaults for Ecole users
macro(set_user_defaults)
	set_common_defaults()
	option(ENABLE_IPO "Enable Interprocedural Optimization, aka Link Time Optimization (LTO)" ON)
	set_default_build_type(RelWithDebInfo)
endmacro()


# Set of defaults for Ecole developers (anyone contributing)
macro(set_developer_defaults)
	set_common_defaults()
	set_default_build_type(Debug)

	option(ENABLE_PYTHON_VENV "Install Ecole Python library in a virtual environment" ON)
	option(BUILD_TESTING "Build tests in Ecole" ON)

	# Generate compile_commands.json to make it easier to work with clang based tools
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

	# Enable compiler cache if found
	find_program(CCACHE ccache)
	if(CCACHE)
		message(STATUS "Using ccache")
		set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE} CACHE FILEPATH "Compiler launching tool")
	else()
		message(STATUS "Cannot find requirement ccache")
	endif()

	option(ENABLE_DOCUMENTATION "Build documentation with Doxygen and Sphinx" ON)
	option(ENABLE_DOCUMENTATION_TESTING "Enable documentation testing" ON)

endmacro()


macro(set_defaults)
	if(ECOLE_DEVELOPER)
		set_developer_defaults()
	else()
		set_user_defaults()
	endif()
endmacro()


set_defaults()
