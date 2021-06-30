Example: How to Contribute an Observation Function
==================================================

To contribute an observation (or reward) function, there are a few files to modify.
For the purpose of example, let us call our observation `Cookie`.
We recommend looking, at every step, to other observation functions as examples.

.. note::
   Be sure to read the :ref:`contribution guidelines <contributing-reference>` to figure out how to get started and
   running the tests.

Create the Observation
----------------------
The C++ code is typically separated into `headers <https://en.wikipedia.org/wiki/Include_directive>`_
and source files.

Headers care not compiled and should only contains the public
`declaration <https://docs.microsoft.com/en-us/cpp/cpp/declarations-and-definitions-cpp>`_
or classes/functions signature (except for tempalted code).
They should ``#include`` the minimal headers to be self contained.

 - Create the header file ``libecole/include/ecole/observation/cookie.hpp``, and add the observation function declaration.

Source files contain the definition of the functions, _i.e._ their implementation.

 - Create the source file ``libecole/src/observation/cookie.cpp``,
 - Add the inclusion of your header ``#include "ecole/observation/cookie.hpp``
 - Add the definition of your observation function (you can also add helper functions/classes here),
 - Explicitly add the source file in CMake, in ``libecole/CMakeLists.txt``.

Test Your Code
--------------
Tests are not part of a library, so they only need a source file.

 - Create the test file ``libecole/tests/src/observation/test-cookie.cpp``,
 - Add unit tests to ensure the observation function abides to the required interface,
 - Add functional tests to ensure the observation function is correct,
 - Explicitly add the test file in CMake, in ``libecole/tests/CMakeLists.txt``.


Bind Code to Python
-------------------
To expose the code in Python, we are using `PyBind <https://pybind11.readthedocs.io>`_ directly from C++.

 - Edit ``python/src/ecole/core/observation.cpp``, and bind the class using ``py::class_``,
 - Add the docstring.

.. warning::
   Due to some discrepencies between C++ and Python, not all bindings are straightforward.
   More complex types need to be handled on a case-by-case basis.

Test the Bindings
-----------------
We need to make sure nothing is forgotten or raises runtime errors when used from Python.

 - Edit ``python/tests/test_observation.py``, test the interface, and the return types.

Reference the Observation in the Documentation
----------------------------------------------
Documentation from docstring is automatically read by Sphinx, so we only need to tell it where to display it.

 - Add the observation function in the list in ``docs/reference/observation.rst``.

.. note::
   Remember to run the tests and code checks before pushing.
