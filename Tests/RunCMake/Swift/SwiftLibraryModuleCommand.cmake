if(POLICY CMP0157)
  cmake_policy(SET CMP0157 NEW)
endif()

enable_language(Swift)

add_library(StaticLibrary STATIC L.swift)
add_library(DynamicLibrary SHARED L.swift)
set_target_properties(DynamicLibrary
  PROPERTIES
    Swift_MODULE_DIRECTORY "$<IF:$<CONFIG:Release>,release/modules,debug/modules>"
    INSTALL_NAME_DIR "@rpath")

add_library(DynamicLibrary2 SHARED L.swift)
set_target_properties(DynamicLibrary2
  PROPERTIES
    Swift_MODULE_DIRECTORY "Modules"
    INSTALL_NAME_DIR "@rpath")

add_executable(Executable E.swift)

add_dependencies(DynamicLibrary2 DynamicLibrary)
add_dependencies(DynamicLibrary StaticLibrary)
add_dependencies(Executable DynamicLibrary2)
