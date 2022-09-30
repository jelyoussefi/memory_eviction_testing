ARG BASE_IMAGE
FROM $BASE_IMAGE

RUN zypper --non-interactive install opencl-headers

ENV LD_LIBRARY_PATH="/usr/local/lib64/:${LD_LIBRARY_PATH}" 


RUN mkdir -p /workspace

COPY . /workspace

WORKDIR /workspace

RUN make 

ENTRYPOINT ["/usr/bin/memMonitor"]


