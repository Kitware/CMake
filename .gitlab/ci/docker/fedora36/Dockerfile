FROM fedora:36 as rvm-build
MAINTAINER Ben Boeckel <ben.boeckel@kitware.com>

COPY install_rvm.sh /root/install_rvm.sh
RUN sh /root/install_rvm.sh

FROM fedora:36 AS clang-tidy-headers
MAINTAINER Kyle Edwards <kyle.edwards@kitware.com>

COPY install_clang_tidy_headers.sh /root/install_clang_tidy_headers.sh
RUN sh /root/install_clang_tidy_headers.sh

FROM fedora:36
MAINTAINER Ben Boeckel <ben.boeckel@kitware.com>

COPY install_deps.sh /root/install_deps.sh
RUN sh /root/install_deps.sh

COPY --from=rvm-build /root/rvm.tar /root/rvm.tar
RUN tar -C /usr/local -xf /root/rvm.tar \
 && rm /root/rvm.tar
COPY --from=clang-tidy-headers /root/clang-tidy-headers.tar /root/clang-tidy-headers.tar
RUN tar -C /usr/include -xf /root/clang-tidy-headers.tar \
 && rm /root/clang-tidy-headers.tar
