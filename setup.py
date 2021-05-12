import pathlib
import re
import sys
import os
import shlex

import skbuild

__dir__ = pathlib.Path(__file__).resolve().parent


def get_version(version_file: pathlib.Path) -> str:
    """Extract version from the Ecole VERSION file."""
    with open(version_file, "r") as f:
        lines = f.read()
    version_dict = re.search(
        "VERSION_MAJOR\s+(?P<major>\d+).*"
        "VERSION_MINOR\s+(?P<minor>\d+).*"
        "VERSION_PATCH\s+(?P<patch>\d+)",
        lines,
        re.DOTALL,
    ).groupdict()
    return "{major}.{minor}.{patch}".format(**version_dict)


install_requires = ["numpy>=1.4"]
if (sys.version_info.major == 3) and (sys.version_info.minor <= 6):
    install_requires += ["typing_extensions"]


skbuild.setup(
    name="ecole",
    author="Antoine Prouvost et al.",
    version=get_version(__dir__ / "VERSION"),
    url="https://www.ecole.ai",
    description="Extensible Combinatorial Optimization Learning Environments",
    license="BSD-3-Clause",
    packages=["ecole"],
    package_dir={"": "python/src"},
    package_data={"ecole": ["py.typed"]},
    cmake_languages=["CXX"],
    cmake_install_dir="python/src",
    # FIXME No way to pass cmake argument to scikit-build through pip (for now)
    # https://github.com/scikit-build/scikit-build/issues/479
    # So we read them from an environment variable
    cmake_args=shlex.split(os.environ.get("CMAKE_ARGS", "")),
    zip_safe=False,
    python_requires=">=3.6",
    install_requires=install_requires,
)
