.. _installation:

Installation
============

Conda
-----
.. image:: https://img.shields.io/conda/vn/conda-forge/ecole?label=version&logo=conda-forge
   :alt: Conda-Forge version
.. image:: https://img.shields.io/conda/pn/conda-forge/ecole?logo=conda-forge
   :alt: Conda-Forge platforms

.. code-block:: bash

   conda install -c conda-forge ecole

All dependencies are resolved by conda, no compiler is required.

`PyScipOpt <https://github.com/SCIP-Interfaces/PySCIPOpt>`_ is not required but is the main SCIP
interface to develop new Ecole components from Python

.. code-block:: bash

   conda install -c conda-forge ecole pyscipopt

Currenlty, conda packages are only available for Linux and MacOS.

Pip wheel (binary)
------------------
Currently unavailable.

Pip source
-----------
.. image:: https://img.shields.io/pypi/v/ecole?logo=python
   :target: https://pypi.org/project/ecole/
   :alt: PyPI version

Building from source requires:
 - A `C++17 compiler <https://en.cppreference.com/w/cpp/compiler_support>`_,
 - A `SCIP <https://www.scipopt.org/>`_ installation.

For the stable `PyPI version <https://pypi.org/project/ecole/>`_:

.. code-block:: bash

   pip install ecole

To specify the where to find SCIP (or any CMake parameters):

.. code-block:: bash

   CMAKE_ARGS="-DSCIP_DIR=path/to/lib/cmake/scip -DCMAKE_INSTALL_RPATH_USE_LINK_PATH=ON" pip install ecole

For the latest Github version:

.. code-block:: bash

   pip install git+https://github.com/ds4dm/ecole
