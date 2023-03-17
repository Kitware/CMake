FROM kitware/nvidia-cuda:10.2-devel-ubuntu18.04
MAINTAINER Ben Boeckel <ben.boeckel@kitware.com>

COPY llvm.list /etc/apt/sources.list.d/llvm.list
COPY llvm-snapshot.gpg.key /root/llvm-snapshot.gpg.key
RUN apt-key add /root/llvm-snapshot.gpg.key

COPY install_deps.sh /root/install_deps.sh
RUN sh /root/install_deps.sh
