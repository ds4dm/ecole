find_or_download_package(
	NAME Catch2
	URL https://github.com/catchorg/Catch2/archive/v2.13.4.tar.gz
	URL_HASH SHA256=e7eb70b3d0ac2ed7dcf14563ad808740c29e628edde99e973adad373a2b5e4df
	CONFIGURE_ARGS -D CMAKE_BUILD_TYPE=Release
)
