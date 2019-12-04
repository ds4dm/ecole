# Ecole

Ecole (pronounced [ekɔl]) stands for _Extensible Combinatorial Optimization Learning
Environments_ and aims to expose a number of combinatorial optimization tasks as Markov
Decision Processes (_i.e._, Reinforcement Learning environments).
Rather than trying to solve such tasks directly, the philosophy behind Ecole is to work
in cooperation with state-of-the-art Mixed Integer Linear Programming solver.

The underlying solver used is [SCIP](https://scip.zib.de/), and the user facing API is
meant to mimic the [OpenAi Gym](https://gym.openai.com/) API (as much as possible).

## User Documentation
Please refer to the (upcoming) documentation for tutorials, examples, and installation
instructions.

## Developer Notes
### Build dependencies
  All dependencies required for building can be resolved using a
  [conda](https://docs.conda.io/en/latest/) environment.
  Install everything using
  ```bash
  conda env create -f conda-dev.yml
  ```

### Runtime dependencies
  * An installation of [SCIP](https://scip.zib.de/) >= 6.0, installed with CMake and
    `-D PARASCIP=true`.

### Formatting
  C++ code is formatted using
  [clang-format](https://clang.llvm.org/docs/ClangFormat.html).
  `clang-format` is available in the conda environment, and all files can be formatted
  using
  ```bash
  find libecole python \( -name '*.cpp' -o -name '*.hpp' \) -exec clang-format --style=file -i {} \;
  ```

  Python code is formatted using [Black](https://black.readthedocs.io).
  `black` is available in the conda environment, and all files can be formatted using
  ```bash
  black python/
  ```

### Building
  [CMake](https://cmake.org/) is a meta-build tool, configuring other build tools
  (_e.g._ Make) or IDE's.
  A one-time configuration is necessary for CMake to find dependencies, detect system
  information, _etc_.
  This is the time to pass optional build options, such as the build type and compiler
  choice. For instance `-D CMAKE_BUILD_TYPE=debug` can be added to compile with debug
  information.
  Using `cmake`, we recommend building out of source using `cmake -B build/` to
  configure, and `cmake --build build/` to compile.
  For instance, a complete install could look like
  ```bash
  cmake -B build -S .
  cmake --build build
  pip install build/python/
  ```
