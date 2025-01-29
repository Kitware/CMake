# syntax=docker/dockerfile:1

ARG BASE_IMAGE=fedora:41

FROM ${BASE_IMAGE} AS dnf-cache
# Populate DNF cache w/ the fresh metadata and prefetch packages.
RUN --mount=type=bind,source=dnf.conf,target=/etc/dnf/dnf.conf \
    --mount=type=bind,source=deps_packages.lst,target=/root/deps_packages.lst \
    --mount=type=bind,source=iwyu_packages.lst,target=/root/iwyu_packages.lst \
    --mount=type=bind,source=rust_packages.lst,target=/root/rust_packages.lst \
    --mount=type=bind,source=rvm_packages.lst,target=/root/rvm_packages.lst \
    --mount=type=tmpfs,target=/var/log \
    --mount=type=tmpfs,target=/tmp \
    dnf install --downloadonly -y $(grep -h '^[^#]\+$' /root/*.lst)


FROM ${BASE_IMAGE} AS rust-build
LABEL maintainer="Ben Boeckel <ben.boeckel@kitware.com>"

RUN --mount=type=bind,source=dnf.conf,target=/etc/dnf/dnf.conf \
    --mount=type=bind,source=install_rust.sh,target=/root/install_rust.sh \
    --mount=type=bind,source=rust_packages.lst,target=/root/rust_packages.lst \
    --mount=type=cache,from=dnf-cache,source=/var/cache/libdnf5,target=/var/cache/libdnf5,sharing=private \
    --mount=type=tmpfs,target=/var/log \
    --mount=type=tmpfs,target=/tmp \
    sh /root/install_rust.sh

FROM ${BASE_IMAGE} AS rvm-build
LABEL maintainer="Ben Boeckel <ben.boeckel@kitware.com>"

RUN --mount=type=bind,source=dnf.conf,target=/etc/dnf/dnf.conf \
    --mount=type=bind,source=install_rvm.sh,target=/root/install_rvm.sh \
    --mount=type=bind,source=rvm_packages.lst,target=/root/rvm_packages.lst \
    --mount=type=cache,from=dnf-cache,source=/var/cache/libdnf5,target=/var/cache/libdnf5,sharing=private \
    --mount=type=tmpfs,target=/var/log \
    --mount=type=tmpfs,target=/tmp \
    sh /root/install_rvm.sh


FROM ${BASE_IMAGE} AS iwyu-build
LABEL maintainer="Kyle Edwards <kyle.edwards@kitware.com>"

RUN --mount=type=bind,source=dnf.conf,target=/etc/dnf/dnf.conf \
    --mount=type=bind,source=install_iwyu.sh,target=/root/install_iwyu.sh \
    --mount=type=bind,source=iwyu_packages.lst,target=/root/iwyu_packages.lst \
    --mount=type=cache,from=dnf-cache,source=/var/cache/libdnf5,target=/var/cache/libdnf5,sharing=private \
    --mount=type=tmpfs,target=/var/log \
    --mount=type=tmpfs,target=/tmp \
    sh /root/install_iwyu.sh


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

RUN --mount=type=bind,from=rust-build,source=/root,target=/root \
    tar -C /usr/local -xf /root/rust.tar

RUN --mount=type=bind,from=rvm-build,source=/root,target=/root \
    tar -C /usr/local -xf /root/rvm.tar

RUN --mount=type=bind,from=iwyu-build,source=/root,target=/root \
    tar -C / -xf /root/iwyu.tar
