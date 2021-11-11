find_package(Python COMPONENTS Interpreter Development NumPy REQUIRED)

find_or_download_package(
	NAME pybind11
	URL https://github.com/pybind/pybind11/archive/v2.6.2.tar.gz
	URL_HASH SHA256=8ff2fff22df038f5cd02cea8af56622bc67f5b64534f1b83b9f133b8366acff2
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
