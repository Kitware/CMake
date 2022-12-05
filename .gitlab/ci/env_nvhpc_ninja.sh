export CC=nvc
export CXX=nvc++
export FC=nvfortran
export CUDACXX=nvcc
export CUDAHOSTCXX=nvc++
# FIXME(#24225): /opt/nvidia/hpc_sdk/Linux_x86_64/22.9/compilers/bin/nvcc
# selects a CUDA version 10.2 with host driver versions > 520.
# Manually select the preferred nvcc version.
export PATH=/opt/nvidia/hpc_sdk/Linux_x86_64/22.9/cuda/11.7/bin:$PATH
