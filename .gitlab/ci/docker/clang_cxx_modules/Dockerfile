FROM fedora:37
MAINTAINER Ben Boeckel <ben.boeckel@kitware.com>

# Install build dependencies for packages.
COPY install_deps.sh /root/install_deps.sh
RUN sh /root/install_deps.sh

COPY install_llvm.sh /root/install_llvm.sh
RUN sh /root/install_llvm.sh

# Install build dependencies for CMake's CI.
COPY install_cmake_deps.sh /root/install_cmake_deps.sh
RUN sh /root/install_cmake_deps.sh
