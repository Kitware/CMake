enable_language(C)

set(CMAKE_LINK_LIBRARIES_ONLY_TARGETS 1)

# Use imported interface library to name toolchain-provided libraries.
add_library(toolchain::m INTERFACE IMPORTED)
set_property(TARGET toolchain::m PROPERTY IMPORTED_LIBNAME "m")

# Linking directly warns.
add_executable(exe main.c)
target_link_libraries(exe PRIVATE
  -lflag_in_exe     # accepted
  /abs/path/in_exe  # accepted
  rel/path/in_exe   # accepted
  toolchain::m      # accepted
  non_target_in_exe # rejected
  )

# Link interfaces warn.
add_library(iface INTERFACE)
target_link_libraries(iface INTERFACE
  -lflag_in_iface     # accepted
  /abs/path/in_iface  # accepted
  rel/path/in_iface   # accepted
  non_target_in_iface # rejected
  )

# Imported target link interfaces warn if explicitly enabled.
add_library(iface_imported_checked INTERFACE IMPORTED)
target_link_libraries(iface_imported_checked INTERFACE
  -lflag_iface_imported_checked        # accepted
  /abs/path/in_iface_imported_checked  # accepted
  rel/path/in_iface_imported_checked   # accepted
  non_target_in_iface_imported_checked # rejected
  )
set_property(TARGET iface_imported_checked PROPERTY LINK_LIBRARIES_ONLY_TARGETS 1)

# Linking directly does not warn if explicitly disabled.
add_executable(exe_not_checked main.c)
target_link_libraries(exe_not_checked PRIVATE
  non_target_in_exe_not_checked
  )
set_property(TARGET exe_not_checked PROPERTY LINK_LIBRARIES_ONLY_TARGETS 0)

# Link interfaces do not warn if explicitly disabled.
add_library(iface_not_checked INTERFACE)
target_link_libraries(iface_not_checked INTERFACE
  non_target_in_iface_not_checked
  )
set_property(TARGET iface_not_checked PROPERTY LINK_LIBRARIES_ONLY_TARGETS 0)

# Imported target link interfaces do not warn if not explicitly enabled.
add_library(iface_imported_default INTERFACE IMPORTED)
target_link_libraries(iface_imported_default INTERFACE
  non_target_in_iface_imported_default
  )
