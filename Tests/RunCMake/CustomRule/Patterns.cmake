
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

add_custom_rule(preprocess OUTPUT <OUTPUT_DIR>/<BASE_NAME>.c
  COMMAND "${CMAKE_COMMAND}" -E make_directory "<OUTPUT_DIR>"
  COMMAND ${create_preprocessed_source})


# Use RULE_PATTERNS file set property
add_library(foo1 STATIC)

target_sources(foo1 PRIVATE FILE_SET fs TYPE preprocess FILES file1.c)

set_property(FILE_SET fs TARGET foo1 PROPERTY RULE_PATTERNS
  "INPUT=$<PATH:NATIVE_PATH,<SOURCE>>;OUTPUT_DIR=<CURRENT_BINARY_DIR>/<TARGET>;CMAKE_C_COMPILER=${CMAKE_C_COMPILER};PREPROCESSED_SOURCE=$<PATH:NATIVE_PATH,<OUTPUT_DIR>/<BASE_NAME>.c>;INCLUDES=;DEFINES=${CMAKE_C_DEFINE_FLAG}RULE_PATTERN=1;FLAGS=")


# Use <RULE>_PATTERNS source file property
add_library(foo2 STATIC)

target_sources(foo2 PRIVATE FILE_SET fs TYPE preprocess FILES file1.c)

set_property(SOURCE file1.c TARGET_DIRECTORY foo2 PROPERTY preprocess_PATTERNS
  "INPUT=$<PATH:NATIVE_PATH,<SOURCE>>;OUTPUT_DIR=<CURRENT_BINARY_DIR>/<TARGET>;CMAKE_C_COMPILER=${CMAKE_C_COMPILER};PREPROCESSED_SOURCE=$<PATH:NATIVE_PATH,<OUTPUT_DIR>/<BASE_NAME>.c>;INCLUDES=;DEFINES=${CMAKE_C_DEFINE_FLAG}RULE_PATTERN=1;FLAGS=")


# <RULE>_PATTERNS source file property override RULE_PATTERNS file set property
add_library(foo3 STATIC)

target_sources(foo3 PRIVATE FILE_SET fs TYPE preprocess FILES file1.c)

set_property(FILE_SET fs TARGET foo3 PROPERTY RULE_PATTERNS
  "DEFINES=${CMAKE_C_DEFINE_FLAG}WRONG=1")
