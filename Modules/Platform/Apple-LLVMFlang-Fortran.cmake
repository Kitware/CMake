# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

# LLVM Flang requires Darwin linker flags to be passed through its linker
# wrapper rather than directly to the compiler driver.
set(CMAKE_Fortran_OSX_COMPATIBILITY_VERSION_FLAG "-Xlinker -compatibility_version -Xlinker ")
set(CMAKE_Fortran_OSX_CURRENT_VERSION_FLAG "-Xlinker -current_version -Xlinker ")
set(CMAKE_SHARED_MODULE_CREATE_Fortran_FLAGS "-Xlinker -bundle")
set(CMAKE_SHARED_LIBRARY_SONAME_Fortran_FLAG "-Xlinker -install_name -Xlinker ")
