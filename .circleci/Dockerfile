FROM continuumio/miniconda

RUN for PY_VERSION in "3.6" "3.7" "3.8" ; do \
		conda create -n "ecole-py${PY_VERSION:?}" \
			--channel scipopt --channel conda-forge  \
			gxx_linux-64 clangxx cmake'>=3.15' make conan clang-tools'>=10' \
			scip pyscipopt'>=3.0.1' python="${PY_VERSION:?}" pybind11 xtensor-python numpy \
			pytest pytest-benchmark black \
			doxygen sphinx'>=3.0' breathe'>=4.15' sphinx_rtd_theme ; \
	done && \
	conda clean --all --yes

# Minimum required to use as CircleCI image
RUN apt-get update && \
	apt-get install -y --no-install-recommends \
		wget git openssh-client tar gzip ca-certificates && \
	apt-get clean

WORKDIR /app
