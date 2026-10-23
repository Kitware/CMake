# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

# Produce an image with custom-built dependencies for portable CMake binaries.
# Build using the directory containing this file as its own build context.

ARG FROM_IMAGE_NAME=kitware/cmake:build-linux-aarch64-base-2026-01-13
ARG FROM_IMAGE_DIGEST=@sha256:8385631a7999a04d59aff527cf20302526c0d42e4d4c0d3db05e20c09d297fc5
ARG FROM_IMAGE=$FROM_IMAGE_NAME$FROM_IMAGE_DIGEST
FROM $FROM_IMAGE

# Qt
# Version 5.12.0 was the last to bundle xkbcommon.
COPY qt-install.patch /opt/qt/src/
RUN : \
 && mkdir -p /opt/qt/src/qt-build \
 && cd /opt/qt/src \
 && curl -OL https://download.qt.io/archive/qt/5.12/5.12.0/single/qt-everywhere-src-5.12.0.tar.xz \
 && sha512sum qt-everywhere-src-5.12.0.tar.xz | grep -q 0dd03d2645fb6dac5b58c8caf92b4a0a6900131f1ccfb02443a0df4702b5da0458f4c45e758d1b929ec709b0f4b36900df2fd60a058af9cc8c1a0748b6d57aae \
 && tar xJf qt-everywhere-src-5.12.0.tar.xz \
 && cd qt-build \
 && ../qt-everywhere-src-5.12.0/configure \
      -prefix /opt/qt \
      -static \
      -release \
      -c++std c++11 \
      -opensource -confirm-license \
      -gui \
      -widgets \
      -xcb \
      -fontconfig \
      -sql-sqlite \
      -qt-doubleconversion \
      -qt-libjpeg \
      -qt-libpng \
      -qt-pcre \
      -qt-sqlite \
      -qt-xcb \
      -qt-xkbcommon \
      -qt-zlib \
      -system-freetype \
      -no-accessibility \
      -no-compile-examples \
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
      -no-iconv \
      -no-icu \
      -no-journald \
      -no-kms \
      -no-libinput \
      -no-libproxy \
      -no-linuxfb \
      -no-ltcg \
      -no-mirclient \
      -no-mtdev \
      -no-opengl \
      -no-openssl \
      -no-pch \
      -no-sql-mysql \
      -no-sql-psql \
      -no-sql-sqlite2 \
      -no-syslog \
      -no-system-proxies \
      -no-tslib \
      -no-use-gold-linker \
      -skip declarative \
      -skip multimedia \
      -skip qtcanvas3d \
      -skip qtconnectivity \
      -skip qtdeclarative \
      -skip qtlocation \
      -skip qtmultimedia \
      -skip qtsensors \
      -skip qtserialport \
      -skip qtsvg \
      -skip qtwayland \
      -skip qtwebchannel \
      -skip qtwebengine \
      -skip qtwebsockets \
      -skip qtwinextras \
      -skip qtxmlpatterns \
      -nomake examples \
      -nomake tests \
 && make install -j $(nproc) \
 && cd /opt/qt \
 && patch -p1 -i src/qt-install.patch \
 && cd /opt \
 && rm -rf /opt/qt/src \
 && :

# Curses
RUN : \
 && mkdir -p /opt/ncurses/src/ncurses-build \
 && cd /opt/ncurses/src \
 && curl -O https://ftp.gnu.org/pub/gnu/ncurses/ncurses-6.6.tar.gz \
 && sha512sum ncurses-6.6.tar.gz | grep -q 02647baae53abc844fbadee5b0a2187ad073125c4e8950df6d1c4feb781cb74ba64fb838cedfee2c246c39932187f6775b1df124f18b99a4233f0d98c72191de \
 && tar xzf ncurses-6.6.tar.gz \
 && cd ncurses-build \
 && ../ncurses-6.6/configure \
      --prefix=/opt/ncurses \
      --with-terminfo-dirs=/etc/terminfo:/lib/terminfo:/usr/share/terminfo \
      --with-default-terminfo-dir=/usr/share/terminfo \
      --without-shared \
      --enable-overwrite \
 && make -j $(nproc) \
 && make install.libs install.includes \
 && cd /opt \
 && rm -rf /opt/ncurses/src \
 && :

# OpenSSL
COPY openssl-source.patch /opt/openssl/src/
RUN : \
 && mkdir -p /opt/openssl/src \
 && cd /opt/openssl/src \
 && curl -OL https://github.com/openssl/openssl/releases/download/openssl-3.6.5/openssl-3.6.5.tar.gz \
 && sha256sum openssl-3.6.5.tar.gz | grep -q a2157c2830efdec3788939b00c9b0638306d3f0bbb76dc4832ee503bb397df98 \
 && tar xzf openssl-3.6.5.tar.gz \
 && cd openssl-3.6.5 \
 && patch -p1 -i ../openssl-source.patch \
 && ./Configure --prefix=/opt/openssl linux-elf no-asm no-shared -D_POSIX_C_SOURCE=199506L -D_POSIX_SOURCE=1 -D_SVID_SOURCE=1 -D_BSD_SOURCE=1 \
 && make install_dev -j $(nproc) \
 && cd /opt \
 && rm -rf /opt/openssl/src \
 && :
