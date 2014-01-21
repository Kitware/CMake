
include(Platform/QNX)

set(CMAKE_INCLUDE_SYSTEM_FLAG_C "-Wp,-isystem,")
set(CMAKE_DEPFILE_FLAGS_C "-Wc,-MMD,<DEPFILE>,-MT,<OBJECT>,-MF,<DEPFILE>")
