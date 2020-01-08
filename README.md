# Ecole

[![CircleCI](https://circleci.com/gh/ds4dm/ecole.svg?style=svg)](https://circleci.com/gh/ds4dm/ecole)

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
#### Conda
  All dependencies required for building can be resolved using a
  [conda](https://docs.conda.io/en/latest/) environment.
  Install everything in a development (named `ecole`) environment using
  ```bash
  conda env create -n ecole -f conda-dev.yml
  ```
  For the following, the `ecole` environment always needs to be activated
  ```bash
  conda activate ecole
  ```
  *Note: this environment contains tools to build ecole and scip, format code, test,
  generate documentation etc. These are more than the dependencies to only use Ecole.*
  
#### CMake
  [CMake](https://cmake.org/) is a meta-build tool, configuring other build tools
  (_e.g._ Make) or IDE's.
  The whole build of Ecole can be done with CMake.
  A one-time configuration is necessary for CMake to find dependencies, detect system
  information, _etc_.
  This is the time to pass optional build options, such as the build type and compiler
  choice. For instance `-D CMAKE_BUILD_TYPE=debug` can be added to compile with debug
  information.
  Using `cmake`, we recommend building out of source using `cmake -B build/` to
  configure, and `cmake --build build/` to compile.
  CMake is made available in the `ecole` environment created earlier.

### Runtime dependencies
  * **SCIP** - Head to the [download page](https://scip.zib.de/index.php#download),
    select a version greater than 6.0 and download the `scipoptsuite-x.y.z.tgz` file
    (where `x.y.z` is to be replaced with the selected version).
    Extract it using
    ```bash
    tar -xz -f scipoptsuite-x.y.z.tgz
    ```
    In the extracted folder, configure and build with CMake
    ```bash
    cmake -B build/ -D PARASCIP=1 -D GMP=0 -D ZIMPL=0 -D GCG=0 -D READLINE=0
    cmake --build build/ --parallel
    ```
    Finally, install SCIP in the `ecole` conda environment
    ```bash
    cmake --install build/ --prefix $(conda run -n ecole echo '$CONDA_PREFIX')
    ```

### Building
  Building Ecole is very similar to building SCIP, as they both use CMake.
  In the Ecole source repository, configure using
  ```bash
  cmake -B build
  ```
  It is possible to add `-D CMAKE_BUILD_TYPE=debug` to this previous command to
  build with debugging information.
  Then, build Ecole with
  ```bash
  cmake --build build/
  ```
  The Python package can be installed from the build directory
  ```bash
  python -m pip install -I build/python
  ```

### Formatting
  Automatic tools are used to format the code. Pull request will be denied if the
  code is not formatted properly.

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
