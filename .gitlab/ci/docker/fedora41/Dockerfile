# syntax=docker/dockerfile:1

ARG BASE_IMAGE=fedora:41

FROM ${BASE_IMAGE} AS dnf-cache
# Populate DNF cache w/ the fresh metadata and prefetch packages.
RUN --mount=type=bind,source=dnf.conf,target=/etc/dnf/dnf.conf \
    --mount=type=bind,source=deps_packages.lst,target=/root/deps_packages.lst \
    --mount=type=bind,source=iwyu_packages.lst,target=/root/iwyu_packages.lst \
    --mount=type=bind,source=rbenv_packages.lst,target=/root/rbenv_packages.lst \
    --mount=type=bind,source=rust_packages.lst,target=/root/rust_packages.lst \
    --mount=type=bind,source=rvm_packages.lst,target=/root/rvm_packages.lst \
    --mount=type=tmpfs,target=/var/log \
    --mount=type=tmpfs,target=/tmp \
    dnf install --downloadonly -y $(grep -h '^[^#]\+$' /root/*.lst)


FROM ${BASE_IMAGE} AS rust-build-env
LABEL maintainer="Ben Boeckel <ben.boeckel@kitware.com>"
# Pre-install prerequisites to build Rust projects.
RUN --mount=type=bind,source=dnf.conf,target=/etc/dnf/dnf.conf \
    --mount=type=bind,source=rust_packages.lst,target=/root/rust_packages.lst \
    --mount=type=cache,from=dnf-cache,source=/var/cache/libdnf5,target=/var/cache/libdnf5,sharing=private \
    --mount=type=tmpfs,target=/var/log \
    --mount=type=tmpfs,target=/tmp \
    dnf install -y $(grep '^[^#]\+$' /root/rust_packages.lst)


FROM rust-build-env AS rust-build
LABEL maintainer="Ben Boeckel <ben.boeckel@kitware.com>"
# Build the needed Rust packages.
# https://doc.rust-lang.org/cargo/guide/cargo-home.html?highlight=.cargo#caching-the-cargo-home-in-ci
RUN --mount=type=bind,source=build_rust.sh,target=/root/build_rust.sh \
    --mount=type=cache,target=/root/.cargo/registry/index \
    --mount=type=cache,target=/root/.cargo/registry/cache \
    --mount=type=tmpfs,target=/tmp \
    sh /root/build_rust.sh


FROM ${BASE_IMAGE} AS rvm-build-env
LABEL maintainer="Ben Boeckel <ben.boeckel@kitware.com>"
# Pre-install prerequisites for RVM.
RUN --mount=type=bind,source=dnf.conf,target=/etc/dnf/dnf.conf \
    --mount=type=bind,source=rvm_packages.lst,target=/root/rvm_packages.lst \
    --mount=type=cache,from=dnf-cache,source=/var/cache/libdnf5,target=/var/cache/libdnf5,sharing=private \
    --mount=type=tmpfs,target=/var/log \
    --mount=type=tmpfs,target=/tmp \
    dnf install -y $(grep '^[^#]\+$' /root/rvm_packages.lst)


FROM rvm-build-env AS rvm-build
LABEL maintainer="Ben Boeckel <ben.boeckel@kitware.com>"
# Build Ruby with RVM.
RUN --mount=type=bind,source=build_rvm.sh,target=/root/build_rvm.sh \
    --mount=type=cache,target=/usr/local/rvm/archives \
    --mount=type=cache,target=/usr/local/rvm/gem-cache   \
    --mount=type=cache,target=/usr/local/rvm/src \
    --mount=type=tmpfs,target=/tmp \
    sh /root/build_rvm.sh


FROM ${BASE_IMAGE} AS rbenv
# Pre-install prerequisites for `rbenv`.
RUN --mount=type=bind,source=dnf.conf,target=/etc/dnf/dnf.conf \
    --mount=type=bind,source=rbenv_packages.lst,target=/root/rbenv_packages.lst \
    --mount=type=cache,from=dnf-cache,source=/var/cache/libdnf5,target=/var/cache/libdnf5,sharing=private \
    --mount=type=tmpfs,target=/var/log \
    --mount=type=tmpfs,target=/tmp \
    dnf install -y $(grep '^[^#]\+$' /root/rbenv_packages.lst)


FROM rbenv AS rbenv-build
ENV RBENV_ROOT=/opt/rbenv
ENV RBENV_BUILD_ROOT=/root/.cache/rbenv-build
ENV RUBY_BUILD_CACHE_PATH=/root/.cache/rbenv
# Build Ruby with `rbenv`.
RUN --mount=type=cache,target=/root/.cache \
    --mount=type=bind,source=build_rbenv.sh,target=/root/build_rbenv.sh \
    --mount=type=tmpfs,target=/tmp \
    sh /root/build_rbenv.sh


FROM ${BASE_IMAGE} AS iwyu-build-env
LABEL maintainer="Kyle Edwards <kyle.edwards@kitware.com>"
# Pre-install prerequisites to build IWYU.
RUN --mount=type=bind,source=dnf.conf,target=/etc/dnf/dnf.conf \
    --mount=type=bind,source=iwyu_packages.lst,target=/root/iwyu_packages.lst \
    --mount=type=cache,from=dnf-cache,source=/var/cache/libdnf5,target=/var/cache/libdnf5,sharing=private \
    --mount=type=tmpfs,target=/var/log \
    --mount=type=tmpfs,target=/tmp \
    dnf install -y $(grep '^[^#]\+$' /root/iwyu_packages.lst)


FROM iwyu-build-env AS iwyu-build
LABEL maintainer="Kyle Edwards <kyle.edwards@kitware.com>"
# Build IWYU.
RUN --mount=type=bind,source=build_iwyu.sh,target=/root/build_iwyu.sh \
    --mount=type=cache,target=/root/include-what-you-use \
    --mount=type=tmpfs,target=/tmp \
    sh /root/build_iwyu.sh


FROM ${BASE_IMAGE} AS p4-dl
# Download Perforce.
# NOTE `curl` is pre-installed in the base image.
RUN curl -C- -L https://www.perforce.com/downloads/perforce/r21.2/bin.linux26x86_64/helix-core-server.tgz \
  | tar -C /usr/local/bin -xvzf - -- p4 p4d


FROM ${BASE_IMAGE}
LABEL maintainer="Ben Boeckel <ben.boeckel@kitware.com>"

ENV RBENV_ROOT=/opt/rbenv

COPY --from=p4-dl --chown=root:root /usr/local/bin/p4 /usr/local/bin/p4d /usr/local/bin

RUN --mount=type=bind,source=dnf.conf,target=/etc/dnf/dnf.conf \
    --mount=type=bind,source=install_deps.sh,target=/root/install_deps.sh \
    --mount=type=bind,source=deps_packages.lst,target=/root/deps_packages.lst \
    --mount=type=cache,from=dnf-cache,source=/var/cache/libdnf5,target=/var/cache/libdnf5,sharing=private \
    --mount=type=tmpfs,target=/var/log \
    --mount=type=tmpfs,target=/tmp \
    sh /root/install_deps.sh

RUN --mount=type=bind,from=iwyu-build,source=/root,target=/root \
    tar -C / -xf /root/iwyu.tar

RUN --mount=type=bind,from=rust-build,source=/root,target=/root \
    tar -C /usr/local -xf /root/rust.tar

RUN --mount=type=bind,from=rvm-build,source=/root,target=/root \
    tar -C /usr/local -xf /root/rvm.tar

RUN --mount=type=bind,from=rbenv-build,source=/root,target=/root \
    tar -C / -xf /root/rbenv.tar && rbenv global 3.1.2
