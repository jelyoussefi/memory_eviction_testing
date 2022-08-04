FROM ge/intel/sles_dpcpp_compiler

RUN zypper --non-interactive install opencl-headers

RUN mkdir -p /workspace
WORKDIR /workspace

COPY . /workspace

RUN touch ~/.bashrc 
RUN make build

