ARG BASE_IMAGE
FROM $BASE_IMAGE

RUN zypper --non-interactive install opencl-headers

RUN mkdir -p /workspace
WORKDIR /workspace

COPY . /workspace

RUN mkdir -p /workspace/cl_cache

RUN touch ~/.bashrc 
RUN make build

