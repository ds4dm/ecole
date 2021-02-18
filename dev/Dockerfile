# Matrix built with different compilers (e.g. gcc9, clang10) and python versions.
ARG compiler=clang10
ARG python_version=3.6

FROM conanio/${compiler}
USER root
COPY conda.yaml conda.yaml

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
RUN conda env update -n base -f conda.yaml && \
    conda install --channel conda-forge python="${python_version}" && \
    conda clean --all --yes

WORKDIR /app
