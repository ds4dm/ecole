import pathlib
import re
import sys
import os
import shlex
import platform

from typing import List

import skbuild

__dir__ = pathlib.Path(__file__).resolve().parent


def get_file(file: pathlib.Path) -> str:
    """Extract all lines from a file."""
    with open(file, "r") as f:
        return f.read()


def get_version(version_file: pathlib.Path) -> str:
    """Extract version from the Ecole VERSION file according to PEP440."""
    lines = get_file(version_file)
    major = re.search(r"VERSION_MAJOR\s+(\d+)", lines).group(1)
    minor = re.search(r"VERSION_MINOR\s+(\d+)", lines).group(1)
    patch = re.search(r"VERSION_PATCH\s+(\d+)", lines).group(1)
    pre = re.search(r"VERSION_PRE\s+([\.\w]*)", lines).group(1)
    post = re.search(r"VERSION_POST\s+([\.\w]*)", lines).group(1)
    dev = re.search(r"VERSION_DEV\s+([\.\w]*)", lines).group(1)
    return f"{major}.{minor}.{patch}{pre}{post}{dev}"


def get_env_cmake_args() -> List[str]:
    """Return the list of extra CMake arguments from the environment.

    When called through conda-build (environment variable `CONDA_BUILD` is set), the `CMAKE_INSTALL_<>`
    are filtered out as they
    """
    cmake_args = shlex.split(os.environ.get("CMAKE_ARGS", ""))
    if "CONDA_BUILD" in os.environ:
        install_re = re.compile(r"-D\s*CMAKE_INSTALL.*")
        cmake_args = [a for a in cmake_args if not install_re.search(a)]
    return cmake_args


def get_cmake_install_args() -> List[str]:
    """Return default installation settings."""
    if "CONDA_BUILD" in os.environ:
        return get_cmake_out_package_install_args()
    else:
        return get_cmake_in_package_install_args()


def get_cmake_in_package_install_args() -> List[str]:
    """Return default installation settings for installing libecole in the package."""
    system = platform.system()
    if system == "Linux":
        origin = r"\${ORIGIN}"
    elif system == "Darwin":
        origin = "@loader_path"
    else:
        raise NotImplementedError(f"OS {system} is not supported")
    return [
        "-DBUILD_SHARED_LIBS=ON",
        "-DCMAKE_INSTALL_LIBDIR=lib",
        "-DCMAKE_INSTALL_BINDIR=bin",
        "-DCMAKE_INSTALL_INCLUDEDIR=include",
        "-DECOLE_PY_EXT_INSTALL_LIBDIR='.'",
        "-DECOLE_PY_EXT_INSTALL_RPATH={origin}/lib".format(origin=origin),
    ]


def get_cmake_out_package_install_args() -> List[str]:
    """Return default installation settings for an extrenal libecole installation."""
    return ["-DECOLE_BUILD_LIB=OFF", "-DCMAKE_INSTALL_RPATH_USE_LINK_PATH=ON"]


install_requires = ["numpy>=1.4"]
if (sys.version_info.major == 3) and (sys.version_info.minor <= 6):
    install_requires += ["typing_extensions"]


skbuild.setup(
    name="ecole",
    author="Antoine Prouvost et al.",
    version=get_version(__dir__ / "VERSION"),
    url="https://www.ecole.ai",
    description="Extensible Combinatorial Optimization Learning Environments",
    long_description=get_file(__dir__ / "README.rst"),
    long_description_content_type="text/x-rst",
    license="BSD-3-Clause",
    packages=["ecole"],
    package_dir={"": "python/ecole/src"},
    package_data={"ecole": ["py.typed"]},
    cmake_languages=["CXX"],
    cmake_install_dir="python/ecole/src/ecole",  # Must match package_dir layout
    # FIXME No way to pass cmake argument to scikit-build through pip (for now)
    # https://github.com/scikit-build/scikit-build/issues/479
    # So we read them from an environment variable
    cmake_args=get_cmake_install_args() + get_env_cmake_args(),
    zip_safe=False,
    python_requires=">=3.6",
    install_requires=install_requires,
)
