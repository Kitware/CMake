
enable_language(C)

set(cfg_dir)
get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(_isMultiConfig)
  set(cfg_dir /Debug)
endif()
set(DUMP_EXE "${CMAKE_CURRENT_BINARY_DIR}${cfg_dir}/dump${CMAKE_EXECUTABLE_SUFFIX}")

add_executable(dump dump.c)

# ensure no temp file will be used
string(REPLACE "${CMAKE_START_TEMP_FILE}" "" CMAKE_C_CREATE_SHARED_LIBRARY "${CMAKE_C_CREATE_SHARED_LIBRARY}")
string(REPLACE "${CMAKE_END_TEMP_FILE}" "" CMAKE_C_CREATE_SHARED_LIBRARY "${CMAKE_C_CREATE_SHARED_LIBRARY}")

function (add_test_library target_name)
  add_library(${target_name} SHARED LinkOptionsLib.c)

  # use LAUNCH facility to dump linker command
  set_property(TARGET ${target_name} PROPERTY RULE_LAUNCH_LINK "\"${DUMP_EXE}\"")

  add_dependencies(${target_name} dump)
endfunction()

# Use LINKER alone
add_test_library(linker)
target_link_options(linker PRIVATE "LINKER:-foo,bar")

# Use LINKER with SHELL
add_test_library(linker_shell)
target_link_options(linker_shell PRIVATE "LINKER:SHELL:-foo bar")

# Nested LINKER: prefixes should be preserved as written, only the outermost LINKER: prefix removed
add_test_library(linker_nested)
target_link_options(linker_nested PRIVATE "LINKER:LINKER:-foo,-Xlinker=bar,-Xlinker,--baz,-Wl,/qux")

# Same with LINKER:SHELL:
add_test_library(linker_nested_shell SHARED LinkOptionsLib.c)
target_link_options(linker_nested_shell PRIVATE "LINKER:SHELL:LINKER:-foo -Xlinker=bar -Xlinker --baz -Wl,/qux")

# generate reference for LINKER flag
if (CMAKE_C_LINKER_WRAPPER_FLAG)
  set(linker_flag ${CMAKE_C_LINKER_WRAPPER_FLAG})
  list(GET linker_flag -1 linker_space)
  if (linker_space STREQUAL " ")
    list(REMOVE_AT linker_flag -1)
  else()
    set(linker_space)
  endif()
  list (JOIN linker_flag " " linker_flag)
  if (CMAKE_C_LINKER_WRAPPER_FLAG_SEP)
    set(linker_sep "${CMAKE_C_LINKER_WRAPPER_FLAG_SEP}")

    set(linker_flag_nested       "${linker_flag}${linker_space}LINKER:-foo${linker_sep}-Xlinker=bar${linker_sep}-Xlinker${linker_sep}--baz${linker_sep}-Wl${linker_sep}/qux")
    set(linker_flag_nested_shell "${linker_flag}${linker_space}LINKER:-foo${linker_sep}-Xlinker=bar${linker_sep}-Xlinker${linker_sep}--baz${linker_sep}-Wl,/qux")
    string (APPEND  linker_flag "${linker_space}" "-foo${linker_sep}bar")
  else()
    set(linker_prefix "${linker_flag}${linker_space}")

    set(linker_flag_nested       "${linker_prefix}LINKER:-foo ${linker_prefix}-Xlinker=bar ${linker_prefix}-Xlinker ${linker_prefix}--baz ${linker_prefix}-Wl ${linker_prefix}/qux")
    set(linker_flag_nested_shell "${linker_prefix}LINKER:-foo ${linker_prefix}-Xlinker=bar ${linker_prefix}-Xlinker ${linker_prefix}--baz ${linker_prefix}-Wl,/qux")
    set (linker_flag "${linker_prefix}-foo ${linker_prefix}bar")
  endif()
else()
  set(linker_flag_nested       "LINKER:-foo -Xlinker=bar -Xlinker --baz -Wl /qux")
  set(linker_flag_nested_shell "LINKER:-foo -Xlinker=bar -Xlinker --baz -Wl,/qux")
  set(linker_flag "-foo bar")
endif()
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/LINKER.txt" "${linker_flag}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/LINKER_NESTED.txt" "${linker_flag_nested}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/LINKER_NESTED_SHELL.txt" "${linker_flag_nested_shell}")
