set(CMAKE_PKG_CONFIG_PC_LIB_DIRS)

set(ENV{PKG_CONFIG_PATH} ${CMAKE_CURRENT_LIST_DIR}/PackageRoot)

if(WIN32)
  set(sep ";")
else()
  set(sep ":")
endif()

set(ENV{PKG_CONFIG_LIBDIR} "Alpha${sep}Beta")
set(ENV{PKG_CONFIG_DISABLE_UNINSTALLED} Gamma)
set(ENV{PKG_CONFIG_SYSROOT_DIR} Delta)
set(ENV{PKG_CONFIG_TOP_BUILD_DIR} Epsilon)
set(ENV{PKG_CONFIG_SYSTEM_INCLUDE_PATH} "Zeta${sep}Eta")
set(ENV{PKG_CONFIG_SYSTEM_LIBRARY_PATH} "Theta${sep}Iota")
set(ENV{PKG_CONFIG_ALLOW_SYSTEM_CFLAGS} Kappa)
set(ENV{PKG_CONFIG_ALLOW_SYSTEM_LIBS} Lambda)

set(ENV{CPATH} "Mu${sep}Nu")
set(ENV{C_INCLUDE_PATH} "Xi${sep}Omnicron")
set(ENV{CPLUS_INCLUDE_PATH} "Pi${sep}Rho")

if(WIN32)
  set(ENV{OBJC_INCLUDE_PATH} Sigma)
  set(ENV{INCLUDE} Tau)
else()
  set(ENV{OBJC_INCLUDE_PATH} Sigma:Tau)
endif()

set(ENV{LIBRARY_PATH} "Upsilon${sep}Phi")

cmake_pkg_config(
  EXTRACT relocate
  ENV_MODE IGNORE
  PC_PATH ${CMAKE_CURRENT_LIST_DIR}/PackageRoot
  SYSTEM_INCLUDE_DIRS /Alpha
  SYSTEM_LIBRARY_DIRS /Beta
)

# Shouldn't mangle, ALLOW_SYSTEM_* should default to on under ENV IGNORE
message("Includes: ${CMAKE_PKG_CONFIG_INCLUDES}")
message("LibDirs: ${CMAKE_PKG_CONFIG_LIBDIRS}")

cmake_pkg_config(
  EXTRACT qux
  ENV_MODE IGNORE
  PC_PATH ${CMAKE_CURRENT_LIST_DIR}/PackageRoot
)

# Shouldn't find uninstalled package
message("Cflags: ${CMAKE_PKG_CONFIG_CFLAGS}")

cmake_pkg_config(
  EXTRACT foo
  ENV_MODE FDO
)

message("PC_LIB_DIRS: ${CMAKE_PKG_CONFIG_PC_LIB_DIRS}")
message("PC_PATH: ${CMAKE_PKG_CONFIG_PC_PATH}")
message("DISABLE_UNINSTALLED: ${CMAKE_PKG_CONFIG_DISABLE_UNINSTALLED}")
message("SYSROOT_DIR: ${CMAKE_PKG_CONFIG_SYSROOT_DIR}")
message("TOP_BUILD_DIR: ${CMAKE_PKG_CONFIG_TOP_BUILD_DIR}")
message("SYSTEM_INCLUDE_DIRS: ${CMAKE_PKG_CONFIG_SYS_INCLUDE_DIRS}")
message("SYSTEM_LIB_DIRS: ${CMAKE_PKG_CONFIG_SYS_LIB_DIRS}")
message("ALLOW_SYSTEM_INCLUDES: ${CMAKE_PKG_CONFIG_ALLOW_SYS_INCLUDES}")
message("ALLOW_SYSTEM_LIBRARIES: ${CMAKE_PKG_CONFIG_ALLOW_SYS_LIBS}")

cmake_pkg_config(
  EXTRACT foo
  ENV_MODE PKGCONF
)

message("PKGCONF_INCLUDES: ${CMAKE_PKG_CONFIG_PKGCONF_INCLUDES}")
message("PKGCONF_LIB_DIRS: ${CMAKE_PKG_CONFIG_PKGCONF_LIB_DIRS}")
