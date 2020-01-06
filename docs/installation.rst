Installation
============

PyPI
----
We plan to deploy builds on PyPI.

Anaconda
--------
We plan to deploy builds on Anaconda.

From Source
-----------
Runtime dependencies
^^^^^^^^^^^^^^^^^^^^
* `Scip <https://scip.zib.de/>`_ >= 6.0 needs to be independently installed with CMake.
  Configure with ``-D PARASCIP=true`` and make an installation.
  For instance, in ``scipoptsuite`` root directory, this could look like

  .. code-block:: bash

    cmake -B build/ -D PARASCIP=true
    cmake --build build/ --parallel
    cmake --install build/

Building
^^^^^^^^
Depending on what you are building (the C++ library, the Python bindings, the tests,
the documentation) a number of tools are required.
A superset of these tools are listed in the `Conda <https://docs.conda.io/>`_ environment
file ``conda-dev.yml``.

All components can be build with `CMake <https://cmake.org/>`_. For instance

  .. code-block:: bash

    cmake -B build/ -D CMAKE_BUILD_TYPE=release
    cmake --build build/ --parallel

The python package can then be installed from the build directory

  .. code-block:: bash

    pip install build/python/
