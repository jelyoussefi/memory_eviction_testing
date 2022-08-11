ARG BASE_IMAGE
FROM $BASE_IMAGE

RUN zypper --non-interactive install opencl-headers
WORKDIR /tmp
RUN git clone https://github.com/jupp0r/prometheus-cpp && \
	cd prometheus-cpp && \
	git submodule init && git submodule update && \
	mkdir build && cd build && CXX=g++ \
	cmake .. -DBUILD_SHARED_LIBS=ON -DENABLE_PUSH=OFF -DENABLE_COMPRESSION=OFF .. && \
	make &&  make install && \
	rm -rf *

ENV LD_LIBRARY_PATH="/usr/local/lib64/:${LD_LIBRARY_PATH}" 

RUN mkdir -p /workspace
WORKDIR /workspace

COPY . /workspace

RUN mkdir -p /workspace/cl_cache

RUN touch ~/.bashrc 
RUN make build

