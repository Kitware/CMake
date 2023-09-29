#
# Splay the libraries and includes to emulate conda where
# things are split between the host and build prefix
#
# /usr/local/cuda/include/crt/ -> /tmp/cuda/include/crt
# /usr/local/cuda/lib64/stubs/ -> /tmp/cuda/stubs/
# /usr/local/cuda/lib64/libcudart* -> /tmp/cuda/libs/
#
# Also reduce to minimal subset of libraries by removing
# static libraries to emulate a minimal cuda install
mkdir -p /tmp/cuda/libs
mkdir -p /tmp/cuda/stubs
mkdir -p /tmp/cuda/include

mv /usr/local/cuda/lib64/libcuda* /tmp/cuda/libs
mv /usr/local/cuda/lib64/stubs/ /tmp/cuda/stubs/
mv /usr/local/cuda/include/crt/ /tmp/cuda/include/

# patch the nvcc.profile to handle the splayed layout
# which allows verification
mv /usr/local/cuda/bin/nvcc.profile /usr/local/cuda/bin/nvcc.profile.orig
echo "
TOP              = \$(_HERE_)/..

NVVMIR_LIBRARY_DIR = \$(TOP)/\$(_NVVM_BRANCH_)/libdevice

LD_LIBRARY_PATH += \$(TOP)/lib:
PATH            += \$(TOP)/\$(_NVVM_BRANCH_)/bin:\$(_HERE_):

INCLUDES        +=  \"-I\$(TOP)/\$(_TARGET_DIR_)/include\" \$(_SPACE_) \"-I/tmp/cuda/include\" \$(_SPACE_)

LIBRARIES        =+ \$(_SPACE_) \"-L\$(TOP)/\$(_TARGET_DIR_)/lib\$(_TARGET_SIZE_)\" \"-L/tmp/cuda/stubs/\" \"-L/tmp/cuda/libs\"

CUDAFE_FLAGS    +=
PTXAS_FLAGS     +=
" > /usr/local/cuda/bin/nvcc.profile
