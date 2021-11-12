# File that download the dependencies of libecole
include(dependencies/public.cmake)

find_package(pybind11 REQUIRED)
find_package(xtensor REQUIRED)
find_package(xtensor-python REQUIRED)

add_library(ecole-py-ext-helper INTERFACE)
add_library(Ecole::ecole-py-ext-helper ALIAS ecole-py-ext-helper)

target_include_directories(
	ecole-py-ext-helper
	INTERFACE
		$<INSTALL_INTERFACE:include>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(
	ecole-py-ext-helper
	INTERFACE
		pybind11::headers
		xtensor-python
)

# Installation library and symlink
include(GNUInstallDirs)
install(
	TARGETS ecole-py-ext-helper
	EXPORT "EcoleExtensionHelperTargets"
)

# Install CMake targets definition
install(
	EXPORT "EcoleExtensionHelperTargets"
	FILE "EcoleExtensionHelperTargets.cmake"
	NAMESPACE Ecole::
	COMPONENT Ecole_Python_Development
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/EcoleExtensionHelper"
)

# Install headers
install(
	DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/ecole"
	DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
	COMPONENT Ecole_Python_Development
	FILES_MATCHING PATTERN "*.hpp"
)

# Generate and install config and version files
include(CMakePackageConfigHelpers)
configure_package_config_file(
	"EcoleExtensionHelperConfig.cmake.in"
	"EcoleExtensionHelperConfig.cmake"
	INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/EcoleExtensionHelper"
)
write_basic_package_version_file(
	"EcoleExtensionHelperConfigVersion.cmake"
	VERSION "${Ecole_VERSION}"
	COMPATIBILITY SameMinorVersion
)
install(
	FILES
		"${CMAKE_CURRENT_BINARY_DIR}/EcoleExtensionHelperConfig.cmake"
		"${CMAKE_CURRENT_BINARY_DIR}/EcoleExtensionHelperConfigVersion.cmake"
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/EcoleExtensionHelper"
	COMPONENT Ecole_Python_Development
)

# Install the files to download dependencies (not mandatory but useful for users)
install(
	FILES "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/public.cmake"
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/EcoleExtensionHelper"
	COMPONENT Ecole_Python_Development
)
