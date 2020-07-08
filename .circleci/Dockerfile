# Matrix built with different compilers (e.g. gcc9, clang10).
ARG compiler=clang9

# --------------------

# Build and runtime dependencies in a image with different compiler and different python version
FROM conanio/${compiler} AS builder
USER root
ARG python_version=3.6

# Install minimal dependencies for a CircleCI image, install miniconda, and install Ecole
# dependencies.
RUN apt-get update && \
    apt-get install -y --no-install-recommends wget git openssh-client tar gzip ca-certificates && \
    apt-get clean && \
    pyenv install miniconda3-latest && \
    pyenv global miniconda3-latest
# We could get compilers from conda, but we want to avoid having compile flags set by conda as it
# can interfer with coverage and sanitizers.
# Still getting cmake... to make sure it finds scip and other conda packages
RUN conda install --channel scipopt --channel conda-forge \
        cmake make conan \
        scip pyscipopt python="${python_version}" pybind11 numpy xtensor-python \
        pytest && \
    conda clean --all --yes
WORKDIR /app

# --------------------

# Code analysis dependencies
FROM builder AS checker
RUN apt-get update && \
    apt-get install -y --no-install-recommends clang-format clang-tidy && \
    apt-get clean && \
    pip install --no-cache-dir pre-commit

# --------------------

# Documentation generator dependencies
FROM builder AS documenter
RUN conda install -c conda-forge doxygen sphinx'>=3.0' breathe'>=4.15' sphinx_rtd_theme && \
    conda clean --all --yes
