FROM debian:10 AS iwyu-build
MAINTAINER Ben Boeckel <ben.boeckel@kitware.com>

COPY install_iwyu.sh /root/install_iwyu.sh
RUN sh /root/install_iwyu.sh

FROM debian:10 AS rvm-build
MAINTAINER Ben Boeckel <ben.boeckel@kitware.com>

COPY install_rvm.sh /root/install_rvm.sh
RUN sh /root/install_rvm.sh

FROM debian:10
MAINTAINER Ben Boeckel <ben.boeckel@kitware.com>

COPY install_deps.sh /root/install_deps.sh
RUN sh /root/install_deps.sh

RUN --mount=type=bind,from=iwyu-build,source=/root,target=/root \
    tar -C / -xf /root/iwyu.tar \
 && ln -s /usr/lib/llvm-6.0/bin/include-what-you-use /usr/bin/include-what-you-use-6.0

RUN --mount=type=bind,from=rvm-build,source=/root,target=/root \
    tar -C /usr/local -xf /root/rvm.tar
