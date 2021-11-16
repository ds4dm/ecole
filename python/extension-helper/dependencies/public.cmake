find_package(Python COMPONENTS Interpreter Development NumPy REQUIRED)

find_or_download_package(
	NAME pybind11
	URL https://github.com/pybind/pybind11/archive/v2.8.1.tar.gz
	URL_HASH SHA256=f1bcc07caa568eb312411dde5308b1e250bd0e1bc020fae855bf9f43209940cc
	CONFIGURE_ARGS
		-D PYBIND11_TEST=OFF
		-D "Python_EXECUTABLE=${Python_EXECUTABLE}"
		-D "PYTHON_EXECUTABLE=${Python_EXECUTABLE}"
)

find_or_download_package(
	NAME xtensor-python
	URL https://github.com/xtensor-stack/xtensor-python/archive/0.25.1.tar.gz
	URL_HASH SHA256=1e70db455a4dcba226c450bf9261a05a0c2fad513b84be35a3d139067356e6a1
	CONFIGURE_ARGS
		-D BUILD_TESTS=OFF
		-D "Python_EXECUTABLE=${Python_EXECUTABLE}"
		-D "PYTHON_EXECUTABLE=${Python_EXECUTABLE}"
)
