# This file processes the flags set by conda in the compiler packages.
#
# Conda has a separate set of debug flags defined, which are not picked up by CMake.
# Similarily the regular flags set by Conda are not adapted to debug builds.
#
# This file adds to build type:
#  - CondaDebug for the the DEBUG_XXXFLAGS set by Conda,
#  - CondaRelease for the XXXFLAGS set by Conda.

# If we are building a recipe or not using the compiler packages then do nothing.
if(DEFINED ENV{CONDA_BUILD} OR NOT DEFINED ENV{CONDA_BUILD_SYSROOT})
	return()
endif()

# Utility to set the language and linker flags.
function(set_flags LANG FLAGS)
	set(BUILD "${ARGV2}")  # Optional build type
	if(BUILD)
		set(
			CMAKE_${LANG}_FLAGS_${BUILD} "${FLAGS}"
			CACHE STRING "Flags used by ${LANG} during ${BUILD} builds."
		)
		MARK_AS_ADVANCED(CMAKE_${LANG}_FLAGS_${BUILD})
	else()
		set(
			CMAKE_${LANG}_FLAGS "${FLAGS}"
			CACHE STRING "Flags used by ${LANG} during all builds."
		)
		MARK_AS_ADVANCED(CMAKE_${LANG}_FLAGS)
	endif()
endfunction()

# Define the CondaDebug build type
set_flags(Fortran "$ENV{DEBUG_FFLAGS}" CONDADEBUG)
set_flags(C "$ENV{DEBUG_CFLAGS} $ENV{DEBUG_CPPFLAGS}" CONDADEBUG)
set_flags(CXX "$ENV{DEBUG_CXXFLAGS} $ENV{DEBUG_CPPFLAGS}" CONDADEBUG)

# Define the CondaRelease build type
set_flags(Fortran "$ENV{FFLAGS}" CONDARELEASE)
set_flags(C "$ENV{CFLAGS} $ENV{CPPFLAGS}" CONDARELEASE)
set_flags(CXX "$ENV{CXXFLAGS} $ENV{CPPFLAGS}" CONDARELEASE)
set_flags(EXE_LINKER "$ENV{LDFLAGS}" CONDARELEASE)
set_flags(SHARED_LINKER "$ENV{LDFLAGS}" CONDARELEASE)
set_flags(MODULE_LINKER "$ENV{LDFLAGS}" CONDARELEASE)

# Reset the shared flags
set_flags(Fortran "")
set_flags(C "")
set_flags(CXX "")
set_flags(EXE_LINKER "")
set_flags(SHARED_LINKER "")
set_flags(MODULE_LINKER "")
