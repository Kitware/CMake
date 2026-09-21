
enable_language(C)

if (NOT CMAKE_C_CREATE_PREPROCESSED_SOURCE)
  return()
endif()

if (NOT CMAKE_C_DEFINE_FLAG)
  set(CMAKE_C_DEFINE_FLAG "-D")
endif()

# clean-up the command line
string(REPLACE "${CMAKE_START_TEMP_FILE}" "" create_preprocessed_source "${CMAKE_C_CREATE_PREPROCESSED_SOURCE}")
string(REPLACE "${CMAKE_END_TEMP_FILE}" "" create_preprocessed_source "${create_preprocessed_source}")
string(REPLACE "<SOURCE>" "<INPUT>" create_preprocessed_source "${create_preprocessed_source}")
separate_arguments(create_preprocessed_source UNIX_COMMAND "${create_preprocessed_source}")


# Use file set configurator
function(fs_configurator rule target fileset outputFileset patterns)
  set(${patterns} "INPUT=$<PATH:NATIVE_PATH,<SOURCE>>;OUTPUT_DIR=<CURRENT_BINARY_DIR>/${target};CMAKE_C_COMPILER=${CMAKE_C_COMPILER};PREPROCESSED_SOURCE=$<PATH:NATIVE_PATH,<OUTPUT_DIR>/<BASE_NAME>.c>;INCLUDES=;DEFINES=${CMAKE_C_DEFINE_FLAG}RULE_PATTERN=1;FLAGS=" PARENT_SCOPE)
endfunction()

add_custom_rule(preprocess1 OUTPUT <OUTPUT_DIR>/<BASE_NAME>.c
  COMMAND "${CMAKE_COMMAND}" -E make_directory "<OUTPUT_DIR>"
  COMMAND ${create_preprocessed_source}
  CONFIGURATOR FOR_FILE_SET fs_configurator)

add_library(foo1 STATIC)

target_sources(foo1 PRIVATE FILE_SET fs TYPE preprocess1 FILES file1.c)

# Use source file configurator
function(src_configurator rule target fileset outputFileset source patterns)
  set(${patterns} "INPUT=$<PATH:NATIVE_PATH,<SOURCE>>;OUTPUT_DIR=<CURRENT_BINARY_DIR>/${target};CMAKE_C_COMPILER=${CMAKE_C_COMPILER};PREPROCESSED_SOURCE=$<PATH:NATIVE_PATH,<OUTPUT_DIR>/<BASE_NAME>.c>;INCLUDES=;DEFINES=${CMAKE_C_DEFINE_FLAG}RULE_PATTERN=1;FLAGS=" PARENT_SCOPE)
endfunction()

add_custom_rule(preprocess2 OUTPUT <OUTPUT_DIR>/<BASE_NAME>.c
  COMMAND "${CMAKE_COMMAND}" -E make_directory "<OUTPUT_DIR>"
  COMMAND ${create_preprocessed_source}
  CONFIGURATOR FOR_SOURCE src_configurator)

add_library(foo2 STATIC)

target_sources(foo2 PRIVATE FILE_SET fs TYPE preprocess2 FILES file1.c)


# source file configurator override file set configurator
function(fs_configurator2 rule target fileset outputFileset patterns)
  set(${patterns} "INPUT=$<PATH:NATIVE_PATH,<SOURCE>>;OUTPUT_DIR=<CURRENT_BINARY_DIR>/${target};CMAKE_C_COMPILER=${CMAKE_C_COMPILER};PREPROCESSED_SOURCE=$<PATH:NATIVE_PATH,<OUTPUT_DIR>/<BASE_NAME>.c>;INCLUDES=;DEFINES=${CMAKE_C_DEFINE_FLAG}WRONG=1;FLAGS=" PARENT_SCOPE)
endfunction()

function(src_configurator2 rule target fileset outputFileset source patterns)
  set(${patterns} "DEFINES=${CMAKE_C_DEFINE_FLAG}RULE_PATTERN=1" PARENT_SCOPE)
endfunction()

add_custom_rule(preprocess3 OUTPUT <OUTPUT_DIR>/<BASE_NAME>.c
  COMMAND "${CMAKE_COMMAND}" -E make_directory "<OUTPUT_DIR>"
  COMMAND ${create_preprocessed_source}
  CONFIGURATOR FOR_FILE_SET fs_configurator2 FOR_SOURCE src_configurator2)

add_library(foo3 STATIC)

target_sources(foo3 PRIVATE FILE_SET fs TYPE preprocess3 FILES file1.c)
