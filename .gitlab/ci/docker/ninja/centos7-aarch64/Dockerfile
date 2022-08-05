FROM kitware/cmake:build-linux-aarch64-base-2020-12-21
MAINTAINER Brad King <brad.king@kitware.com>

ARG GIT_TAG=v1.11.0

COPY build_ninja.sh /root/build_ninja.sh
RUN scl enable devtoolset-7 -- sh /root/build_ninja.sh $GIT_TAG
