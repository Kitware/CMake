# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

# Produce an image with custom-built dependencies for portable CMake binaries.
# Build using the directory containing this file as its own build context.

ARG FROM_IMAGE_NAME=kitware/cmake:build-linux-aarch64-base-2026-11-20
ARG FROM_IMAGE_DIGEST=@sha256:257bf7c81eecbbd0505b6e207801cc2208d45a57821254a71e4eea8adc93ea36
ARG FROM_IMAGE=$FROM_IMAGE_NAME$FROM_IMAGE_DIGEST

# Qt
FROM $FROM_IMAGE AS qt
RUN : \
 && curl -OL https://github.com/ninja-build/ninja/releases/download/v1.13.2/ninja-linux-aarch64.zip \
 && sha256sum ninja-linux-aarch64.zip | \
    grep -q fd2cacc8050a7f12a16a2e48f9e06fca5c14fc4c2bee2babb67b58be17a607fc \
 && unzip ninja-linux-aarch64.zip \
 && rm ninja-linux-aarch64.zip \
 && chmod +x ninja \
 && mkdir -p /opt/ninja/bin \
 && mv ninja /opt/ninja/bin \
 && :
RUN : \
 && curl -OL https://github.com/Kitware/CMake/releases/download/v3.31.12/cmake-3.31.12-linux-aarch64.tar.gz \
 && sha256sum cmake-3.31.12-linux-aarch64.tar.gz | \
    grep -q 83f8fd91d2038a56556e1400390fcfe42f79602940c494f6c6f1cdae7f9e7f40 \
 && tar xzf cmake-3.31.12-linux-aarch64.tar.gz \
 && mv cmake-3.31.12-linux-aarch64 /opt/cmake \
 && rm cmake-3.31.12-linux-aarch64.tar.gz \
 && :
COPY qt-source.patch /opt/qt/src/
RUN : \
 && mkdir -p /opt/qt/src/qt-build \
 && cd /opt/qt/src \
 && curl -OL https://download.qt.io/archive/qt/6.8/6.8.5/submodules/qtbase-everywhere-opensource-src-6.8.5.tar.xz \
 && sha256sum qtbase-everywhere-opensource-src-6.8.5.tar.xz | \
    grep -q fc95010e94d8b1ec342eda9281fd278d5f096cf9ad8f1a0adfba9575438b0abc \
 && tar xJf qtbase-everywhere-opensource-src-6.8.5.tar.xz \
 && patch -p0 -i qt-source.patch \
 && cd qt-build \
 && source /opt/rh/gcc-toolset-11/enable \
 && export PATH=/opt/ninja/bin:$PATH \
 && export PATH=/opt/cmake/bin:$PATH \
 && ../qtbase-everywhere-src-6.8.5/configure \
      -prefix /opt/qt \
      -static \
      -release \
      -gui \
      -widgets \
      -xcb \
      -xkbcommon \
      -bundled-xcb-xinput \
      -fontconfig \
      -sql-sqlite \
      -qt-doubleconversion \
      -qt-libjpeg \
      -qt-libpng \
      -qt-pcre \
      -qt-sqlite \
      -qt-zlib \
      -system-freetype \
      -no-accessibility \
      -no-cups \
      -no-dbus \
      -no-directfb \
      -no-egl \
      -no-eglfs \
      -no-evdev \
      -no-gbm \
      -no-gif \
      -no-glib \
      -no-gtk \
      -no-harfbuzz \
      -no-icu \
      -no-journald \
      -no-libinput \
      -no-libproxy \
      -no-linuxfb \
      -no-ltcg \
      -no-mtdev \
      -no-opengl \
      -no-openssl \
      -no-sql-mysql \
      -no-sql-psql \
      -no-syslog \
      -no-system-proxies \
      -no-tslib \
 && ninja install \
 && cd /opt \
 && rm -rf qt/src \
 && tar cf /root/qt.tar qt \
 && :

# Curses
FROM $FROM_IMAGE AS ncurses
RUN : \
 && mkdir -p /opt/ncurses/src/ncurses-build \
 && cd /opt/ncurses/src \
 && curl -O https://ftp.gnu.org/pub/gnu/ncurses/ncurses-6.6.tar.gz \
 && sha256sum ncurses-6.6.tar.gz | \
    grep -q 355b4cbbed880b0381a04c46617b7656e362585d52e9cf84a67e2009b749ff11 \
 && tar xzf ncurses-6.6.tar.gz \
 && cd ncurses-build \
 && source /opt/rh/gcc-toolset-11/enable \
 && ../ncurses-6.6/configure \
      --prefix=/opt/ncurses \
      --with-terminfo-dirs=/etc/terminfo:/lib/terminfo:/usr/share/terminfo \
      --with-default-terminfo-dir=/usr/share/terminfo \
      --without-shared \
      --enable-overwrite \
 && make -j $(nproc) \
 && make install.libs install.includes \
 && cd /opt \
 && rm -rf ncurses/src \
 && tar cf /root/ncurses.tar ncurses \
 && :

# OpenSSL
FROM $FROM_IMAGE AS openssl
COPY openssl-source.patch /opt/openssl/src/
RUN : \
 && mkdir -p /opt/openssl/src \
 && cd /opt/openssl/src \
 && curl -OL https://github.com/openssl/openssl/releases/download/openssl-4.1.0/openssl-4.1.0.tar.gz \
 && sha256sum openssl-4.1.0.tar.gz | \
    grep -q 50c17baf54cb008f6ce42d50deb4b28050a1aad711636cabd4b241c7e90cc2fd \
 && tar xzf openssl-4.1.0.tar.gz \
 && cd openssl-4.1.0 \
 && patch -p1 -i ../openssl-source.patch \
 && source /opt/rh/gcc-toolset-11/enable \
 && ./Configure --prefix=/opt/openssl linux-elf no-asm no-shared -D_POSIX_C_SOURCE=199506L -D_POSIX_SOURCE=1 -D_SVID_SOURCE=1 -D_BSD_SOURCE=1 \
 && make install_dev -j $(nproc) \
 && cd /opt \
 && rm -rf openssl/src \
 && tar cf /root/openssl.tar openssl \
 && :

# CMake build environment
FROM $FROM_IMAGE
LABEL maintainer="Brad King <brad.king@kitware.com>"

RUN --mount=type=bind,from=qt,source=/root,target=/root \
    tar -C /opt -xf /root/qt.tar

RUN --mount=type=bind,from=ncurses,source=/root,target=/root \
    tar -C /opt -xf /root/ncurses.tar

RUN --mount=type=bind,from=openssl,source=/root,target=/root \
    tar -C /opt -xf /root/openssl.tar
