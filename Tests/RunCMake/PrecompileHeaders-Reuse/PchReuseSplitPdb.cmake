cmake_policy(SET CMP0141 NEW)
set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT ProgramDatabase)
enable_language(C)

file(WRITE ${CMAKE_BINARY_DIR}/pch.h "")
file(WRITE ${CMAKE_BINARY_DIR}/creator.c "int creator(void) { return 0; }\n")
file(WRITE ${CMAKE_BINARY_DIR}/uses_pch.c "int uses_pch(void) { return 0; }\n")
file(WRITE ${CMAKE_BINARY_DIR}/skips_pch.c "int skips_pch(void) { return 0; }\n")
set_property(SOURCE ${CMAKE_BINARY_DIR}/skips_pch.c
  PROPERTY SKIP_PRECOMPILE_HEADERS ON)

add_library(creator STATIC ${CMAKE_BINARY_DIR}/creator.c)
target_precompile_headers(creator PRIVATE ${CMAKE_BINARY_DIR}/pch.h)

# Created with CMP0235 unset.
add_library(reuse_old STATIC
  ${CMAKE_BINARY_DIR}/uses_pch.c
  ${CMAKE_BINARY_DIR}/skips_pch.c
  )
cmake_policy(SET CMP0235 NEW)
foreach(name IN ITEMS reuse_default reuse_named reuse_legacy)
  add_library(${name} STATIC
    ${CMAKE_BINARY_DIR}/uses_pch.c
    ${CMAKE_BINARY_DIR}/skips_pch.c
    )
endforeach()

foreach(name IN ITEMS reuse_old reuse_default reuse_named reuse_legacy)
  target_precompile_headers(${name} REUSE_FROM creator)
  set_property(TARGET ${name} PROPERTY
    COMPILE_PDB_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${name})
endforeach()
set_property(TARGET reuse_old PROPERTY COMPILE_PDB_NAME named_old)
set_property(TARGET reuse_named PROPERTY COMPILE_PDB_NAME named)
# CMake 3.16 to 4.1 required this value.
set_property(TARGET reuse_legacy PROPERTY COMPILE_PDB_NAME creator)
