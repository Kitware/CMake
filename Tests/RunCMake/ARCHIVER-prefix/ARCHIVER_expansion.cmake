
enable_language(C)

set(cfg_dir)
get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(_isMultiConfig)
  set(cfg_dir /Debug)
endif()

set(DUMP_EXE "${CMAKE_CURRENT_BINARY_DIR}${cfg_dir}/dump${CMAKE_EXECUTABLE_SUFFIX}")

add_executable(dump dump.c)

# Overwrite archive rule to enable command line dump
set(CMAKE_C_CREATE_STATIC_LIBRARY "\"${DUMP_EXE}\" create_archive <LINK_FLAGS>")

function(add_test_library target_name options)
  add_library(${target_name} STATIC lib.c)
  set_property(TARGET ${target_name} PROPERTY STATIC_LIBRARY_OPTIONS "${options}")

  add_dependencies(${target_name} dump)
endfunction()

# Use ARCHIVER alone
add_test_library(archiver "ARCHIVER:-foo,bar")

# Use ARCHIVER with SHELL
add_test_library(archiver_shell "ARCHIVER:SHELL:-foo bar")

# Nested ARCHIVER: prefixes should be preserved as written, only the outermost ARCHIVER: prefix removed
add_test_library(archiver_nested "ARCHIVER:ARCHIVER:-foo,bar")

# Same with ARCHIVER:SHELL:
add_test_library(archiver_nested_shell "ARCHIVER:SHELL:ARCHIVER:-foo bar")

# generate reference for ARCHIVER flag
if (CMAKE_C_ARCHIVER_WRAPPER_FLAG)
  set(archiver_flag ${CMAKE_C_ARCHIVER_WRAPPER_FLAG})
  list(GET archiver_flag -1 archiver_space)
  if (archiver_space STREQUAL " ")
    list(REMOVE_AT archiver_flag -1)
  else()
    set(archiver_space)
  endif()
  list (JOIN archiver_flag " " archiver_flag)
  if (CMAKE_C_ARCHIVER_WRAPPER_FLAG_SEP)
    set(archiver_sep "${CMAKE_C_ARCHIVER_WRAPPER_FLAG_SEP}")

    set(archiver_flag_nested       "${archiver_flag}${archiver_space}ARCHIVER:-foo${archiver_sep}bar")
    set(archiver_flag_nested_shell "${archiver_flag}${archiver_space}ARCHIVER:-foo${archiver_sep}bar")
    string (APPEND  archiver_flag "${archiver_space}" "-foo${archiver_sep}bar")
  else()
    set(archiver_prefix "${archiver_flag}${archiver_space}")

    set(archiver_flag_nested       "${archiver_prefix}ARCHIVER:-foo ${archiver_prefix}bar")
    set(archiver_flag_nested_shell "${archiver_prefix}ARCHIVER:-foo ${archiver_prefix}bar")
    set (archiver_flag "${archiver_prefix}-foo ${archiver_prefix}bar")
  endif()
else()
  set(archiver_flag_nested       "ARCHIVER:-foo bar")
  set(archiver_flag_nested_shell "ARCHIVER:-foo bar")
  set(archiver_flag "-foo bar")
endif()
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/ARCHIVER.txt" "${archiver_flag}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/ARCHIVER_NESTED.txt" "${archiver_flag_nested}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/ARCHIVER_NESTED_SHELL.txt" "${archiver_flag_nested_shell}")
