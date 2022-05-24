FROM rocm/dev-ubuntu-20.04:4.2
MAINTAINER Brad King <brad.king@kitware.com>

ENV PATH="/opt/rocm/bin:$PATH"

COPY install_deps.sh /root/install_deps.sh
RUN sh /root/install_deps.sh
