find_or_download_package(
	NAME xtl
	URL https://github.com/xtensor-stack/xtl/archive/0.7.2.tar.gz
	URL_HASH SHA256=95c221bdc6eaba592878090916383e5b9390a076828552256693d5d97f78357c
	CONFIGURE_ARGS -D BUILD_TESTS=OFF
)

find_or_download_package(
	NAME xsimd
	URL https://github.com/xtensor-stack/xsimd/archive/7.4.9.tar.gz
	URL_HASH SHA256=f6601ffb002864ec0dc6013efd9f7a72d756418857c2d893be0644a2f041874e
	CONFIGURE_ARGS -D BUILD_TESTS=OFF
)

find_or_download_package(
	NAME xtensor
	URL https://github.com/xtensor-stack/xtensor/archive/0.23.1.tar.gz
	URL_HASH SHA256=b9bceea49db240ab64eede3776d0103bb0503d9d1f3ce5b90b0f06a0d8ac5f08
	CONFIGURE_ARGS -D BUILD_TESTS=OFF
)

find_or_download_package(
	NAME span-lite
	URL https://github.com/martinmoene/span-lite/archive/v0.9.0.tar.gz
	URL_HASH SHA256=cdb5f86e5f5e679d63700a56de734c44fe22a574a17347d09dbaaef80619af91
	CONFIGURE_ARGS
		-D SPAN_LITE_OPT_BUILD_TESTS=OFF
		-D SPAN_LITE_OPT_BUILD_EXAMPLES=OFF
)
