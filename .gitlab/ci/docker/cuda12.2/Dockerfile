FROM kitware/nvidia-cuda:12.2.2-devel-ubuntu22.04
MAINTAINER Brad King <brad.king@kitware.com>

COPY llvm.list /etc/apt/sources.list.d/llvm.list
COPY llvm-snapshot.gpg.key /root/llvm-snapshot.gpg.key
RUN apt-key add /root/llvm-snapshot.gpg.key

COPY install_deps.sh /root/install_deps.sh
RUN sh /root/install_deps.sh
