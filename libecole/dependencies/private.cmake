find_or_download_package(
	NAME range-v3
	URL https://github.com/ericniebler/range-v3/archive/0.11.0.tar.gz
	URL_HASH SHA256=376376615dbba43d3bef75aa590931431ecb49eb36d07bb726a19f680c75e20c
	CONFIGURE_ARGS
		-D RANGE_V3_TESTS=OFF
		-D RANGE_V3_EXAMPLES=OFF
		-D RANGE_V3_PERF=OFF
		-D RANGE_V3_DOCS=OFF
)

find_or_download_package(
	NAME fmt
	URL https://github.com/fmtlib/fmt/archive/8.0.1.tar.gz
	URL_HASH SHA256=b06ca3130158c625848f3fb7418f235155a4d389b2abc3a6245fb01cb0eb1e01
	CONFIGURE_ARGS
		-D FMT_TEST=OFF
		-D FMT_DOC=OFF
		-D FMT_INSTALL=ON
		-D CMAKE_BUILD_TYPE=Release
		-D BUILD_SHARED_LIBS=OFF
		-D CMAKE_POSITION_INDEPENDENT_CODE=${CMAKE_POSITION_INDEPENDENT_CODE}
)

find_or_download_package(
	NAME robin_hood
	URL https://github.com/martinus/robin-hood-hashing/archive/refs/tags/3.11.2.tar.gz
	URL_HASH SHA256=148b4fbd4fbb30ba10cc97143dcbe385078801b9c9e329cd477c1ea27477cb73
	CONFIGURE_ARGS -D RH_STANDALONE_PROJECT=OFF
)
