ARG BASE_IMAGE
FROM $BASE_IMAGE

RUN zypper --non-interactive install sudo opencl-headers curl-devel

ENV LD_LIBRARY_PATH="/usr/local/lib64/:${LD_LIBRARY_PATH}" 

RUN mkdir -p /workspace

COPY . /workspace

WORKDIR /workspace

RUN make -C ./apps/test build

RUN mv ./suspend_resume_manager.sh /usr/bin/



