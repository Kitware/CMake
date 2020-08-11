# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

# Produce an image with custom-built dependencies for portable CMake binaries.
# Build using the directory containing this file as its own build context.

ARG FROM_IMAGE_NAME=kitware/cmake:build-linux-x86_64-base-2019-08-09
ARG FROM_IMAGE_DIGEST=@sha256:d2c13617f01181a3143a069e4496d6b78eafffa19d181c42be196d5dfd588151
ARG FROM_IMAGE=$FROM_IMAGE_NAME$FROM_IMAGE_DIGEST
FROM $FROM_IMAGE

# Sphinx
RUN : \
 && source /opt/rh/rh-python36/enable \
 && pip install sphinx==2.1.2 \
 && :

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
 && source /opt/rh/devtoolset-6/enable \
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
 && curl -O https://ftp.gnu.org/pub/gnu/ncurses/ncurses-6.1.tar.gz \
 && sha512sum ncurses-6.1.tar.gz | grep -q e308af43f8b7e01e98a55f4f6c4ee4d1c39ce09d95399fa555b3f0cdf5fd0db0f4c4d820b4af78a63f6cf6d8627587114a40af48cfc066134b600520808a77ee \
 && tar xzf ncurses-6.1.tar.gz \
 && cd ncurses-build \
 && source /opt/rh/devtoolset-6/enable \
 && ../ncurses-6.1/configure \
      --prefix=/opt/ncurses \
      --with-terminfo-dirs=/etc/terminfo:/lib/terminfo:/usr/share/terminfo \
      --with-default-terminfo-dir=/usr/share/terminfo \
      --without-shared \
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
 && curl -O https://www.openssl.org/source/openssl-1.1.1f.tar.gz \
 && sha512sum openssl-1.1.1f.tar.gz | grep -q b00bd9b5ad5298fbceeec6bb19c1ab0c106ca5cfb31178497c58bf7e0e0cf30fcc19c20f84e23af31cc126bf2447d3e4f8461db97bafa7bd78f69561932f000c \
 && tar xzf openssl-1.1.1f.tar.gz \
 && cd openssl-1.1.1f \
 && patch -p1 -i ../openssl-source.patch \
 && source /opt/rh/devtoolset-6/enable \
 && ./Configure --prefix=/opt/openssl linux-elf no-asm no-shared -D_POSIX_C_SOURCE=199506L -D_POSIX_SOURCE=1 -D_SVID_SOURCE=1 -D_BSD_SOURCE=1 \
 && make install_dev -j $(nproc) \
 && cd /opt \
 && rm -rf /opt/openssl/src \
 && :
