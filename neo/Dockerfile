FROM ubuntu:20.04 as neo_builder

RUN rm /bin/sh && ln -s /bin/bash /bin/sh
ENV DEBIAN_FRONTEND noninteractive

RUN apt update && apt install -y cmake g++ git pkg-config software-properties-common wget
RUN add-apt-repository ppa:intel-opencl/intel-opencl
RUN apt update
RUN apt install -y libigdgmm-dev


# Building gmmlib
#----------------------
WORKDIR /tmp

RUN git clone https://github.com/intel/gmmlib
WORKDIR /tmp/gmmlib
RUN git checkout intel-gmmlib-21.2.1
RUN mkdir /tmp/gmmlib/build
WORKDIR /tmp/gmmlib/build
RUN cmake .. && make -j`nproc` && make install

# Installing igc
#----------------------

WORKDIR /tmp

RUN wget https://github.com/intel/intel-graphics-compiler/releases/download/igc-1.0.8365/intel-igc-opencl-devel_1.0.8365_amd64.deb
RUN wget https://github.com/intel/intel-graphics-compiler/releases/download/igc-1.0.8365/intel-igc-opencl_1.0.8365_amd64.deb
RUN wget https://github.com/intel/intel-graphics-compiler/releases/download/igc-1.0.8365/intel-igc-core_1.0.8365_amd64.deb
RUN wget https://github.com/intel/compute-runtime/releases/download/21.33.20678/intel-ocloc_21.33.20678_amd64.deb
RUN dpkg -i *.deb

# Building Neo
#----------------------

WORKDIR /tmp

RUN git clone https://github.com/intel/compute-runtime neo
WORKDIR /tmp/neo
RUN git config --global user.email "tester@intel.com"
RUN git config --global user.name "Tester"
RUN git checkout 21.34.20767
RUN git cherry-pick ac30102
RUN git cherry-pick 37af6a
RUN mkdir /tmp/neo/build
WORKDIR /tmp/neo/build
RUN cmake -DCMAKE_BUILD_TYPE=Release -DSKIP_UNIT_TESTS=1 ..
RUN make install
