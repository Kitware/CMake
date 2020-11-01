# Make sure CMP0104 isn't issued for CXX targets created prior to enabling CUDA. See #21341.
enable_language(CXX)
add_library(cxx main.cxx)

enable_language(CUDA)
add_library(cuda main.cu)
