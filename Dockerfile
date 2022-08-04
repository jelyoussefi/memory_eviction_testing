FROM ge/intel/sles_dpcpp_compiler_host_level_zero

RUN zypper --non-interactive install opencl-headers

RUN mkdir -p /workspace
WORKDIR /workspace

COPY . /workspace

RUN touch ~/.bashrc 
RUN make build

