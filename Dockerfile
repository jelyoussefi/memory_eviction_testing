ARG BASE_IMAGE
FROM $BASE_IMAGE

RUN zypper --non-interactive install sudo opencl-headers

ENV LD_LIBRARY_PATH="/usr/local/lib64/:${LD_LIBRARY_PATH}" 

RUN mkdir -p /workspace
WORKDIR /workspace

COPY . /workspace

RUN mkdir -p /workspace/cl_cache

RUN touch ~/.bashrc 
RUN make prometheus-cpp build

