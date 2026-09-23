enable_language(C)

# Pretend the platform supports install names, as Darwin does, so that the
# generator writes the install-name directory on every host.
set(CMAKE_PLATFORM_HAS_INSTALLNAME 1)
set(CMAKE_SHARED_LIBRARY_SONAME_C_FLAG "-install_name ")
set(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG "-Wl,-rpath,")
set(CMAKE_C_CREATE_SHARED_LIBRARY
  "<CMAKE_C_COMPILER> <SONAME_FLAG><TARGET_INSTALLNAME_DIR><TARGET_SONAME> -o <TARGET> <OBJECTS>")

add_library(rpath_dir SHARED hello.c)
set_property(TARGET rpath_dir PROPERTY AIX_SHARED_LIBRARY_ARCHIVE OFF)

add_library(custom_dir SHARED hello.c)
set_property(TARGET custom_dir PROPERTY INSTALL_NAME_DIR "/custom/dir")
set_property(TARGET custom_dir PROPERTY BUILD_WITH_INSTALL_NAME_DIR ON)
set_property(TARGET custom_dir PROPERTY AIX_SHARED_LIBRARY_ARCHIVE OFF)
