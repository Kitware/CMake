include(Platform/Linux)
set(CMAKE_SHARED_LIBRARY_SONAME_C_FLAG "")
# RPath is useless on Android, because we can't determine the installation
# location ahead of time.
set(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG "")
