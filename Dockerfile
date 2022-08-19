ARG BASE_IMAGE
FROM $BASE_IMAGE

RUN zypper --non-interactive install sudo opencl-headers

ENV LD_LIBRARY_PATH="/usr/local/lib64/:${LD_LIBRARY_PATH}" 

RUN mkdir -p /workspace

COPY . /workspace

WORKDIR /workspace

RUN mv ./suspend_resume_manager.sh /usr/bin/

RUN make build


