cmake_policy(SET CMP0141 NEW)
enable_language(C)

# Only cl writes compiler PDBs.  Clang and IntelLLVM ignore /Fd, and the
# GNU-like Clang front-end does not support the ProgramDatabase format.
if(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT ProgramDatabase)
  set(check_compile_pdb 1)
else()
  set(check_compile_pdb 0)
endif()

# Exercise the owning target context for both properties, with and without
# a configuration-specific override.  Distinct target names must produce
# distinct PDB directories.
foreach(kind IN ITEMS default config)
  add_executable(exe_${kind} main.c)
  add_library(shared_${kind} SHARED lib.c)
  foreach(type IN ITEMS exe shared)
    set(tgt ${type}_${kind})
    set_target_properties(${tgt} PROPERTIES
      PDB_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/pdb/$<TARGET_PROPERTY:NAME>"
      COMPILE_PDB_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/cpdb/$<TARGET_PROPERTY:NAME>"
      PDB_NAME "${tgt}-link"
      COMPILE_PDB_NAME "${tgt}-compile"
      )
    if(kind STREQUAL "config")
      set_target_properties(${tgt} PROPERTIES
        PDB_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/pdb-debug/$<TARGET_PROPERTY:NAME>"
        COMPILE_PDB_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/cpdb-debug/$<TARGET_PROPERTY:NAME>"
        )
    endif()
    if(MSVC)
      # The path check is independent of TARGET_PDB_FILE's path computation.
      # Debug builds must emit real linker PDBs, and compiler PDBs where the
      # compiler writes them, at these paths.
      add_custom_command(TARGET ${tgt} POST_BUILD
        COMMAND ${CMAKE_COMMAND}
          "-Dartifact_path=${CMAKE_CURRENT_BINARY_DIR}"
          "-Dtarget_name=${tgt}"
          "-Ddirectory_kind=${kind}"
          "-Dcheck_compile_pdb=${check_compile_pdb}"
          -P "${CMAKE_CURRENT_SOURCE_DIR}/checkPDBOutputDirs.cmake"
        VERBATIM
        )
    endif()
  endforeach()
endforeach()
