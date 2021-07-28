FROM fedora:34 as rvm-build
MAINTAINER Ben Boeckel <ben.boeckel@kitware.com>

COPY install_rvm.sh /root/install_rvm.sh
RUN sh /root/install_rvm.sh

FROM fedora:34
MAINTAINER Ben Boeckel <ben.boeckel@kitware.com>

COPY install_deps.sh /root/install_deps.sh
RUN sh /root/install_deps.sh

COPY install_ispc.sh /root/install_ispc.sh
RUN sh /root/install_ispc.sh

COPY --from=rvm-build /root/rvm.tar /root/rvm.tar
RUN tar -C /usr/local -xf /root/rvm.tar \
 && rm /root/rvm.tar
