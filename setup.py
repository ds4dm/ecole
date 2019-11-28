"""Setup script for building Ecole Python package.

This srcipt uses CMake to build the extensions. It was modified from:
  - https://github.com/pybind/cmake_example
  - https://github.com/raydouglass/cmake_setuptools
"""

import os
import sys
import subprocess
import shutil
from typing import Callable, List, Tuple
from pathlib import Path
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext


CURRENT_SRC_DIR = Path(__file__).parent.resolve()
CMAKE_EXE = os.environ.get("CMAKE_EXE", shutil.which("cmake"))


def cmake(*cmake_args: str, **subprocess_kwargs) -> None:
    """Execute a CMake command."""
    if not CMAKE_EXE:
        raise RuntimeError("CMake is required to build this package.")
    # check_call writes to stdout
    subprocess.check_call([CMAKE_EXE, *cmake_args], **subprocess_kwargs)


def check_cmake() -> None:
    """Check if CMake is available."""
    cmake("--version")


class CMakeExtension(Extension):
    """Setuptool extension class for CMake extensions."""

    def __init__(
        self, name: str, cmakelists: Path = Path("."), cmake_target: str = "all",
    ) -> None:
        """Initialize parent Extension class.

        Parameters
        ----------
        name:
            The name (with dots) of the module associated witht the extension.
        cmakelists:
            The path to the CMakeLists.txt used by cmake to generate the extension.
        cmake_target:
            The target used by CMake to generate the extension.

        """
        Extension.__init__(self, name, sources=[])
        self.cmakelists = cmakelists
        self.cmake_target = cmake_target


class CMakeBuild(build_ext):
    """Build class for CMake extensions."""

    def run(self):
        """Build all extensions using CMake."""
        check_cmake()
        for ext in self.extensions:
            self.build_extension(ext)

    def build_extension(self, ext):
        """Build a single exntension using CMake."""
        out_dir = Path(self.get_ext_fullpath(ext.name)).parent.resolve()
        cmake_build = Path(self.build_temp) / "cmake"
        configure_args = [
            f"-S{ext.cmakelists}",
            f"-B{cmake_build}",
            f"-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={out_dir}",
            f"-DPYTHON_EXECUTABLE={sys.executable}",
            "-DCMAKE_BUILD_TYPE={}".format("Debug" if self.debug else "Release"),
        ]
        build_args = ["--build", cmake_build, "--target", ext.cmake_target, "--", "-j"]

        cmake(*configure_args)
        cmake(*build_args)


def list_path(
    path: Path, globs: Tuple[str], filter_func: Callable[[Path], bool] = lambda p: True
) -> List[str]:
    """List paths as string in a glob."""
    all_paths = []
    for g in globs:
        all_paths.extend(str(p.resolve()) for p in path.glob(g) if filter_func(p))
    return all_paths


extension_names = ("base", "scip", "observation", "branching")
extensions = [
    CMakeExtension(
        name=f"ecole.{m}", cmakelists=CURRENT_SRC_DIR, cmake_target=f"py_{m}",
    )
    for m in extension_names
]


setup(
    name="ecole",
    author="Antoine Prouvost",
    version="0.0.1",
    url="https://ecole.ai",
    description="Extensible Combinatorial Optimization Learning Environments",
    license="BSD-3-Clause",
    package_dir={"": "python"},
    ext_modules=extensions,
    cmdclass=dict(build_ext=CMakeBuild),
    zip_safe=False,
)
