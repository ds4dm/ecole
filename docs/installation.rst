Installation
============

Conda
-----
.. code-block:: bash

   conda install -c scipopt -c conda-forge ecole

`PyScipOpt <https://github.com/SCIP-Interfaces/PySCIPOpt>`_ is not required but is the main SCIP
interface to develop new Ecole components from Python

.. code-block:: bash

   conda install -c scipopt -c conda-forge ecole pyscipopt

Currenlty, conda packages are only available for Linux and MacOS.

Pip
---
Currently unavailable.

From Source
-----------
Source builds currently require ``conda`` to fetch the dependencies.

.. code-block:: bash

   conda env create -n ecole -f dev/conda.yaml
   conda activate ecole
   cmake -B build/
   cmake --build build/ --parallel
   python -m pip install build/python


.. warning::

   This mode of installation is not mature.
   In particular, the scip library may not be found when installed outside of the ``ecole`` environemnt.
