FROM debian:10 as iwyu-build
MAINTAINER Ben Boeckel <ben.boeckel@kitware.com>

COPY install_iwyu.sh /root/install_iwyu.sh
RUN sh /root/install_iwyu.sh

FROM debian:10 as rvm-build
MAINTAINER Ben Boeckel <ben.boeckel@kitware.com>

COPY install_rvm.sh /root/install_rvm.sh
RUN sh /root/install_rvm.sh

FROM debian:10
MAINTAINER Ben Boeckel <ben.boeckel@kitware.com>

COPY install_deps.sh /root/install_deps.sh
RUN sh /root/install_deps.sh

COPY --from=iwyu-build /root/iwyu.tar.gz /root/iwyu.tar.gz
RUN tar -C / -xf /root/iwyu.tar.gz
RUN ln -s /usr/lib/llvm-6.0/bin/include-what-you-use /usr/bin/include-what-you-use-6.0

COPY --from=rvm-build /root/rvm.tar /root/rvm.tar
RUN tar -C /usr/local -xf /root/rvm.tar \
 && rm /root/rvm.tar
