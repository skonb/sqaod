SUFFIXES = .cu .cpp

noinst_LTLIBRARIES=libcuda.la
check_PROGRAMS=test

libcuda_la_SOURCES=cudafuncs.cpp DeviceMemoryStore.cpp Device.cpp DeviceStream.cpp DeviceCopy.cpp DeviceObjectAllocator.cpp DeviceMath.cpp DeviceKernels.cu #CUDAFormulas.cpp CUDADenseGraphBFSolver.cpp DeviceDenseGraphBatchSearch.cpp DeviceBipartiteGraphBatchSearch.cpp DeviceRandom.cu 
AM_CPPFLAGS=-I/usr/local/cuda/include -I$(abs_top_srcdir)/eigen -I$(abs_top_srcdir)/cub

# check
test_SOURCES=test.cpp
test_LDADD=libcuda.la $(top_builddir)/cpu/libcpu.la $(top_builddir)/common/libcommon.la
LIBS=-L/usr/local/cuda/lib64 -lcublas -lcudart

NVCC=/usr/local/cuda/bin/nvcc


.cu.o:
	$(NVCC) -c -arch=sm_61 $<  -o $@

.cu.lo:
	mkdir -p .libs
	$(NVCC) -c -std=c++11 -arch=sm_61 -Xcompiler "$(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS)"  $<  -o .libs/$*.o
	@echo "# Generated by libtool :  .lo file for CUDA objects." > $@
	@echo "pic_object='.libs/$*.o'" >> $@
	@echo "non_pic_object=none" >> $@ # FIXME: add static ver.

# https://stackoverflow.com/questions/23116674/build-cuda-and-c-using-autotools
