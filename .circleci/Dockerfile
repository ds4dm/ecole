FROM continuumio/miniconda AS scip-builder

ARG scipoptsuite_url

# Conda environment with build dependencies
RUN conda create -n scip -c conda-forge gcc_linux-64 gxx_linux-64 cmake'>=3.15' make

# Make following commands run in conda environment
SHELL ["conda", "run", "-n", "scip", "/bin/bash", "-c"]

WORKDIR /tmp/scipoptsuite
RUN wget -q "${scipoptsuite_url:?}"
RUN tar -xzf scipoptsuite-6.0.2.tgz
RUN cmake -B build -S scipoptsuite-6.0.2 \
		-D CMAKE_AR="${AR:?}" \
		-D CMAKE_BUILD_TYPE=Release \
		-D PARASCIP=1 \
		-D GMP=0 \
		-D ZIMPL=0 \
		-D GCG=0 \
		-D READLINE=0
RUN cmake --build build
RUN cmake --install build --prefix local


FROM continuumio/miniconda

ENV SCIP_HOME /opt/scipoptsuite
COPY --from=scip-builder /tmp/scipoptsuite/local ${SCIP_HOME}
ENV C_INCLUDE_PATH ${SCIP_HOME}/include:${C_INCLUDE_PATH}
ENV LIBRARY_PATH ${SCIP_HOME}/lib:${LIBRARY_PATH}
ENV LD_LIBRARY_PATH ${SCIP_HOME}/lib:${LD_LIBRARY_PATH}

RUN for PY_VERSION in "3.6" "3.7" "3.8" ; do \
		conda create --use-local -c conda-forge -n "ecole-py${PY_VERSION:?}" \
			gxx_linux-64 clangxx cmake'>=3.15' make conan clang-tools \
			python="${PY_VERSION:?}" pybind11 xtensor-python numpy \
			pytest black \
			doxygen sphinx'>=3.0' breathe'>=4.15' sphinx_rtd_theme ; \
	done && \
	conda clean --all --yes

# Minimum required to use as CircleCI image
RUN apt-get update && \
	apt-get install -y --no-install-recommends \
		wget git openssh-client tar gzip ca-certificates && \
	apt-get clean

WORKDIR /app
