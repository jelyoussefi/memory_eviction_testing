ARG BASE_IMAGE
FROM $BASE_IMAGE

RUN zypper --non-interactive install sudo opencl-headers curl-devel

ENV LD_LIBRARY_PATH="/usr/local/lib64/:${LD_LIBRARY_PATH}" 

RUN mkdir -p /workspace

COPY ./apps/test /workspace

COPY ./suspend_resume_manager.sh ./lp_entry_point.sh /usr/bin/

WORKDIR /workspace

RUN make  build





