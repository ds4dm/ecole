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
