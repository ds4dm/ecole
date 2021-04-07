.. _contributing-reference:

Contribution Guidelines
=======================

Thank you for your interest in contributing to Ecole! 🌟
Contributions are more diverse than contributing new features.
Improving the documentation, reporting and reproducing bugs, discussing the direction of Ecole in
the discussions, helping other use Ecole.


Contribution acceptance
-----------------------
Not all code contributions are relevant for Ecole.
It does not mean that the idea is not good.
We try to balance added value with maintenance and complexity of a growing codebase.
For that reason, it is a good idea to communicate with the developpers first to be sure that we agree on
what should be added/modified.

.. important::

   Be sure to open an issue before sending a pull request.


Tour of the codebase
--------------------
- ``libecole`` is the Ecole C++ library.
  Ecole is mostly written in C++ so this is where you will find most features, rewards, observations...
- ``python`` contains some Python and C++ code to create bindings to Python.
  Ecole uses `PyBind <https://pybind11.readthedocs.io/>`_ to create the binding, these are all the C++ files
  in this directory.
  Sometimes, you may find Python code as well.
  Either this is because a feature is more natuarally in Python, or because we have accepted an early contribution
  that is not yet ported to C++.
- ``docs`` is the `Sphinx <https://www.sphinx-doc.org>`_ documentation written in reStructuredText.
- ``examples`` are practical examples showcasing how to use Ecole for certain tasks.


Dependencies with Conda
-----------------------
All dependencies required for building Ecole (including SCIP) can be resolved using a
`conda <https://docs.conda.io/en/latest/>`_ environment.
Install everything in a development (named ``ecole``) environment using

.. code-block:: bash

   conda env create -n ecole -f dev/conda.yaml

.. code-block:: bash

   conda activate ecole
   conda config --append channels conda-forge
   conda config --set channel_priority flexible

.. note::

   This environment contains tools to build ecole and scip, format code, test,
   generate documentation etc. These are more than the dependencies to only use Ecole.


Building
--------
CMake
^^^^^
`CMake <https://cmake.org>`_ is a meta-build tool, used for configuring other build tools
(*e.g.* Make) or IDE's.
The whole build of Ecole can be done with CMake.
A one-time configuration is necessary for CMake to find dependencies, detect system
information, *etc*.
CMake is made available in the ``ecole`` environment created earlier.
For the following, this environment always needs to be activated.

In the Ecole source repository, configure using

.. code-block:: bash

   cmake -B build/ -D ECOLE_DEVELOPER=ON

.. note::

   This is the time to pass optional build options, such as the build type and compiler
   choice. For instance ``-D CMAKE_BUILD_TYPE=Debug`` can be added to compile with debug
   information.

The definition ``-D ECOLE_DEVELOPER=ON`` changes the default settings (such as the build
type, static analysis, *etc.*) for added convenience.
Only the default settings are changed, this mode does not override any explicit setting.

Then, build Ecole with

.. code-block:: bash

   cmake --build build/ [--parallel [count]]

The Python package can finally be installed from the build directory

.. code-block:: bash

   python -m pip install -I build/python

.. important::

   Be sure to eliminate all warnings, as warnings. They will be considered as errors in the PR.

Compiler issues
^^^^^^^^^^^^^^^
If you encounter problem with your compiler (because it is too old for instance),
you can use the ones from ananconda.

.. code-block:: bash

   conda install cxx-compiler

And start again the configuring of Ecole.

.. code-block:: bash

   rm -r build/ && cmake -B build/ -D ECOLE_DEVELOPER=ON


Running the tests
-----------------
C++ tests
^^^^^^^^^
The C++ tests are build with `Catch2 <https://github.com/catchorg/Catch2>`_.
It produces a single standalone executable under ``build/libecole/tests/test-libecole``.
For test options, consult

.. code-block:: bash

   build/libecole/tests/test-libecole --help

Python tests
^^^^^^^^^^^^
Python tests are build with `PyTest <https://docs.pytest.org/en/latest/>`_.
As any Python project, running the tests requires that Python finds the correct Ecole
package.
It is therefore **not recommended** to manually install Ecole between every tests, as
it leaves room for running tests with an out of date installation.
The CMake configuration creates a Python virtual environment and installs the Ecole
Python package inside automatically.
To run the tests (or another operation with Python Ecole), use the virtual
environment

.. code-block:: bash

   ./build/venv/bin/python -m pytest python/tests/


Generating the documentation
----------------------------
The documentation is a CMake target that is not built by default.
When using ``-D ECOLE DEVELOPER=ON``, the documentation can be generated by providing
the target to the CMake build command:

.. code-block:: bash

   cmake --build build/ --target ecole-sphinx

The generated HTML files are located under ``build/doc/sphinx/html``.
In particular, ``build/doc/sphinx/html/index.html`` can be opened in your browser to
visualize the documentation.


Coding standards
----------------
The quality and conventions of the code are enforced automatically with various tools, for instance
to format the layout of the code and fix some C++ error-prone patterns.

Compilation database
^^^^^^^^^^^^^^^^^^^^
Some C++ tools need access to a *compilation database*.
This is a file called ``compile_commands.json`` that is created automatically by CMake when using
``-D ECOLE_DEVELOPER=ON``.
The file needs to be accessible at the root directory of the project, so you should symlink it like
so (assuming you set CMake to configure in the directory named ``build`` as shown before).

.. code-block:: bash

   ln -s build/compile_commands.json

.. tip::

   This file is also read by `clangd <https://clangd.llvm.org>`_, a C++ language server (already
   installed the conda environment).
   To get code completion, compile errors, go-to-definition and more, you can install a language
   server protocol plugin for your editor.

Pre-commit
^^^^^^^^^^
The tools are configured to run with `pre-commit <https://pre-commit.com/>`_, that is they can be
added to run automatically when making a commit, pushing, or on demand.
To have the tools run automatically, install the pre-commit hooks using

.. code-block:: bash

   pre-commit install

The tools are configured to run light tests only on the files that were changed during the commit,
so they should not run for long.
Installing the pre-commit hooks to run the tools is recommended.
Similar tests will be run online and pull requests *will* fail if the tools have not been run.

With ``pre-commit`` hooks, commits will be rejected by ``git`` if the tests ran by the tools fail.
If the tools can fix the issue for you, you will find some modifications that you can add to
your commit.

Sometimes when working locally, it can be useful not to run the tools.
You can tell ``git`` to ignore the ``pre-commit`` hooks by passing the ``--no-verify`` to any
``git`` command making commit, including ``commit``, ``merge``, ``rebase``, ``push``...

.. code-block:: bash

   git commit --no-verify

Other times you may wish to run the tools on all files unconditionally.
This can be done using

.. code-block:: bash

   pre-commit run --all-files


Running the continuous integration locally
------------------------------------------
The continuous integration runs on `CircleCI <https://circleci.com/>`_.
The file ``.circleci/config.yaml`` describe the steps, environment, `Docker <https://www.docker.com>`_ image,
*etc* that define the jobs.

To run a job locally, install Docker and the
`CircleCI command line interface <https://circleci.com/docs/2.0/local-cli/>`_.
To reproduce a job, say ``build_py38``, on your machine, execute


.. code-block:: bash

   circleci local execute --job build_py38


When things fail
----------------
If you cannot eliminate some warnings, code checks, errors, do not hesistate to ask questions in the
`Github Discussions <https://github.com/ds4dm/ecole/discussions>`_.

.. important::

   When you cannot figure things out, it's OK to send a failing pull request.
   We wish to grow as a community, and help others improve, not exclude and belittle. 🌈
