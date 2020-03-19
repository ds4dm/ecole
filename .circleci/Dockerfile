FROM continuumio/miniconda3

ARG scipoptsuite_url

RUN \
	apt-get update && \
	apt-get install -y --no-install-recommends \
		# Minimum required to use as CircleCI image
		wget git openssh-client tar gzip ca-certificates \
		# Install compiler tools
		build-essential && \
	apt-get clean

RUN \
	conda install -c conda-forge \
		python'>=3.6' \
		# C++ dev tools
		cmake'>=3.15' clangdev'>=3.4' conan \
		# Build library dependencies
		pybind11 xtensor-python numpy \
		# Python dev tools
		pytest black && \
	conda clean --all --yes

# Make RUN commands use the new environment:
SHELL ["conda", "run", "-n", "base", "/bin/bash", "-c"]


WORKDIR /tmp/scipoptsuite

# Download SCIP
RUN \
	wget -q "${scipoptsuite_url:?}" && \
	tar -xzf scipoptsuite-6.0.2.tgz && \
	rm scipoptsuite-6.0.2.tgz

# Build and install SCIP
RUN \
	cmake -B build -S scipoptsuite-6.0.2 \
		-D CMAKE_BUILD_TYPE=Release \
		-D PARASCIP=1 \
		-D GMP=0 \
		-D ZIMPL=0 \
		-D GCG=0 \
		-D READLINE=0 && \
	cmake --build build && \
	cmake --install build --prefix "${CONDA_PREFIX:?}" && \
	rm -rf /tmp/scipoptsuite

WORKDIR /app
