# kFreeBSD looks just like Linux.
include(Platform/Linux)
unset(LINUX)

set(CMAKE_LIBRARY_ARCHITECTURE_REGEX "[a-z0-9_]+(-[a-z0-9_]+)?-kfreebsd-gnu[a-z0-9_]*")
