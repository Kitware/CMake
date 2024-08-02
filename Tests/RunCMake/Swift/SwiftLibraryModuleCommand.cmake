if(POLICY CMP0157)
  cmake_policy(SET CMP0157 NEW)
endif()

enable_language(Swift)

add_library(StaticLibrary STATIC L.swift)
add_library(DynamicLibrary SHARED L.swift)
set_target_properties(DynamicLibrary PROPERTIES INSTALL_NAME_DIR "@rpath")
add_executable(Executable E.swift)

add_dependencies(DynamicLibrary StaticLibrary)
add_dependencies(Executable DynamicLibrary)
