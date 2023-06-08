# kFreeBSD is a Debian GNU distribution with a kernel from FreeBSD,
# and should be marked as LINUX
include(Platform/Linux-Initialize)

set(CMAKE_LIBRARY_ARCHITECTURE_REGEX "[a-z0-9_]+(-[a-z0-9_]+)?-kfreebsd-gnu[a-z0-9_]*")
