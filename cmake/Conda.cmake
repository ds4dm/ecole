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
	set(
		CMAKE_${LANG}_FLAGS_${BUILD} "${FLAGS}"
		CACHE STRING "Flags used by ${LANG} during ${BUILD} builds."
	)
	MARK_AS_ADVANCED(CMAKE_${LANG}_FLAGS_${BUILD})
endfunction()

# Define the CondaDebug build type
set_flags(Fortran "$ENV{DEBUG_FFLAGS}" CONDADEBUG)
set_flags(C "$ENV{DEBUG_CFLAGS} $ENV{DEBUG_CPPFLAGS}" CONDADEBUG)
set_flags(CXX "$ENV{DEBUG_CXXFLAGS} $ENV{DEBUG_CPPFLAGS}" CONDADEBUG)
# Unset the environment flags in order to prevent CMake from reading them
set(ENV{DEBUG_FFLAGS} "")
set(ENV{DEBUG_CFLAGS} "")
set(ENV{DEBUG_CXXFLAGS} "")

# Define the CondaRelease build type
set_flags(Fortran "$ENV{FFLAGS}" CONDARELEASE)
set_flags(C "$ENV{CFLAGS} $ENV{CPPFLAGS}" CONDARELEASE)
set_flags(CXX "$ENV{CXXFLAGS} $ENV{CPPFLAGS}" CONDARELEASE)
set_flags(EXE_LINKER "$ENV{LDFLAGS}" CONDARELEASE)
set_flags(SHARED_LINKER "$ENV{LDFLAGS}" CONDARELEASE)
set_flags(MODULE_LINKER "$ENV{LDFLAGS}" CONDARELEASE)
# Unset the environment flags in order to prevent CMake from reading them
set(ENV{FFLAGS} "")
set(ENV{CFLAGS} "")
set(ENV{CXXFLAGS} "")
set(ENV{LDFLAGS} "")
