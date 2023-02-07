FROM fedora:37 AS rvm-build
MAINTAINER Ben Boeckel <ben.boeckel@kitware.com>

COPY install_rvm.sh /root/install_rvm.sh
RUN sh /root/install_rvm.sh

FROM fedora:37 AS clang-tidy-headers
MAINTAINER Kyle Edwards <kyle.edwards@kitware.com>

COPY install_clang_tidy_headers.sh /root/install_clang_tidy_headers.sh
RUN sh /root/install_clang_tidy_headers.sh

FROM fedora:37 AS iwyu-build
MAINTAINER Kyle Edwards <kyle.edwards@kitware.com>

COPY install_iwyu.sh /root/install_iwyu.sh
RUN sh /root/install_iwyu.sh

FROM fedora:37
MAINTAINER Ben Boeckel <ben.boeckel@kitware.com>

COPY install_deps.sh /root/install_deps.sh
RUN sh /root/install_deps.sh

RUN --mount=type=bind,from=rvm-build,source=/root,target=/root \
    tar -C /usr/local -xf /root/rvm.tar

RUN --mount=type=bind,from=clang-tidy-headers,source=/root,target=/root \
    tar -C /usr/include -xf /root/clang-tidy-headers.tar

RUN --mount=type=bind,from=iwyu-build,source=/root,target=/root \
    tar -C / -xf /root/iwyu.tar
