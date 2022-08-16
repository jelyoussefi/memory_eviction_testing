FROM ge/intel/sles_dpcpp_compiler

RUN zypper --non-interactive install opencl-headers flex bison libzip-devel

RUN git config --global user.name "Jamal El Youssefi"
RUN git config --global user.email "jamal.elyoussefi@yahoo.fr"

WORKDIR /tmp

# Building igc

RUN git clone https://github.com/intel/intel-graphics-compiler igc
RUN git clone https://github.com/intel/vc-intrinsics vc-intrinsics
RUN git clone -b llvmorg-11.1.0 https://github.com/llvm/llvm-project llvm-project
RUN git clone -b ocl-open-110 https://github.com/intel/opencl-clang llvm-project/llvm/projects/opencl-clang
RUN git clone -b llvm_release_110 https://github.com/KhronosGroup/SPIRV-LLVM-Translator llvm-project/llvm/projects/llvm-spirv
RUN git clone https://github.com/KhronosGroup/SPIRV-Tools.git SPIRV-Tools
RUN git clone https://github.com/KhronosGroup/SPIRV-Headers.git SPIRV-Headers
RUN mkdir build
RUN cd build && \
	cmake ../igc && make -j`nproc` && make install

# Building gmmlib

RUN git clone https://github.com/intel/gmmlib
RUN cd gmmlib && mkdir build && cd ./build && \
	cmake .. && make -j`nproc` && make install	

# Building neo drivers

RUN git clone https://github.com/intel/compute-runtime neo
RUN cd neo && mkdir build && cd ./build && \
	cmake -DNEO_SKIP_UNIT_TESTS=1 ..  && make -j`nproc` && make install

RUN echo "export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH" >> ~/.bashrc

RUN rm -rf /tmp/*
