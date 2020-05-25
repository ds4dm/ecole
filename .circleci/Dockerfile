FROM continuumio/miniconda AS scip-builder

ENV CONDA_BLD_PATH /opt/scipoptsuite/channel
WORKDIR /opt/scipoptsuite

ARG scipoptsuite_url
COPY recipe recipe

RUN conda install conda-build conda-verify

RUN wget -q "${scipoptsuite_url:?}"
RUN tar -xzf scipoptsuite-7.0.0.tgz
RUN SCIP_DIR="${PWD}/scipoptsuite-7.0.0" conda build -c conda-forge recipe/scipoptsuite-7.0.0
RUN conda build -c file://${CONDA_BLD_PATH} -c conda-forge recipe/pyscipopt


FROM continuumio/miniconda

ENV SCIP_CHANNEL /opt/scipoptsuite/channel
COPY --from=scip-builder ${SCIP_CHANNEL} ${SCIP_CHANNEL}

RUN for PY_VERSION in "3.6" "3.7" "3.8" ; do \
		conda create -n "ecole-py${PY_VERSION:?}" \
			--use-local -c file://${SCIP_CHANNEL} -c conda-forge  \
			gxx_linux-64 clangxx cmake'>=3.15' make conan clang-tools'>=10' \
			scipoptsuite pyscipopt python="${PY_VERSION:?}" pybind11 xtensor-python numpy \
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
