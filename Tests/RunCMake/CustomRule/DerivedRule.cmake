
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

add_custom_rule(preprocess1 OUTPUT <OUTPUT_DIR>/<BASE_NAME>.c
  COMMAND "${CMAKE_COMMAND}" -E make_directory "<OUTPUT_DIR>"
  COMMAND ${create_preprocessed_source})

set_property(RULE preprocess1 PROPERTY COMPILE_DEFINITIONS RULE_PATTERN=1)

# derived rule take snapshot of properties
add_custom_rule(derived_preprocess1 FROM_RULE preprocess1)

add_library(foo1 STATIC)

target_sources(foo1 PRIVATE FILE_SET fs TYPE derived_preprocess1 FILES file1.c)

set_property(FILE_SET fs TARGET foo1 PROPERTY RULE_PATTERNS
  "INPUT=$<PATH:NATIVE_PATH,<SOURCE>>;OUTPUT_DIR=<CURRENT_BINARY_DIR>/<TARGET>;CMAKE_C_COMPILER=${CMAKE_C_COMPILER};PREPROCESSED_SOURCE=$<PATH:NATIVE_PATH,<OUTPUT_DIR>/<BASE_NAME>.c>;INCLUDES=;DEFINES=$<LIST:TRANSFORM,<COMPILE_DEFINITIONS>,PREPEND,${CMAKE_C_DEFINE_FLAG}>;FLAGS=")

# Update root rule with wrong compile definition
# so preprocess rule is no longer usable but derived_preprocess rule is OK
set_property(RULE preprocess1 PROPERTY COMPILE_DEFINITIONS WRONG=1)


# Use derived rule configurator to restore correct behavior
function(fs_configurator1 rule target fileset patterns)
  set_property(FILE_SET ${fileset} TARGET ${target} PROPERTY COMPILE_DEFINITIONS RULE_PATTERN=1)
endfunction()

add_custom_rule(derived_preprocess2 FROM_RULE preprocess1
  CONFIGURATOR FOR_FILE_SET fs_configurator1)

add_library(foo2 STATIC)

target_sources(foo2 PRIVATE FILE_SET fs TYPE derived_preprocess2 FILES file1.c)

set_property(FILE_SET fs TARGET foo2 PROPERTY RULE_PATTERNS
  "INPUT=$<PATH:NATIVE_PATH,<SOURCE>>;OUTPUT_DIR=<CURRENT_BINARY_DIR>/<TARGET>;CMAKE_C_COMPILER=${CMAKE_C_COMPILER};PREPROCESSED_SOURCE=$<PATH:NATIVE_PATH,<OUTPUT_DIR>/<BASE_NAME>.c>;INCLUDES=;DEFINES=$<LIST:TRANSFORM,<COMPILE_DEFINITIONS>,PREPEND,${CMAKE_C_DEFINE_FLAG}>;FLAGS=")


# Check OVERRIDE behavior
function(fs_configurator2 rule target fileset patterns)
  set_property(FILE_SET ${fileset} TARGET ${target} PROPERTY COMPILE_DEFINITIONS WRONG=1)
endfunction()

add_custom_rule(preprocess2 OUTPUT <OUTPUT_DIR>/<BASE_NAME>.c
  COMMAND "${CMAKE_COMMAND}" -E make_directory "<OUTPUT_DIR>"
  COMMAND ${create_preprocessed_source}
  CONFIGURATOR FOR_FILE_SET fs_configurator2)


function(fs_configurator3 rule target fileset outputFileset patterns)
  set_property(FILE_SET ${fileset} TARGET ${target} APPEND PROPERTY COMPILE_DEFINITIONS DEF=1)
endfunction()

add_custom_rule(derived_preprocess3 FROM_RULE preprocess2
  CONFIGURATOR FOR_FILE_SET fs_configurator3 OVERRIDE)

add_library(foo3 STATIC)

target_sources(foo3 PRIVATE FILE_SET fs TYPE derived_preprocess3 FILES file2.c)

set_property(FILE_SET fs TARGET foo3 PROPERTY RULE_PATTERNS
  "INPUT=$<PATH:NATIVE_PATH,<SOURCE>>;OUTPUT_DIR=<CURRENT_BINARY_DIR>/<TARGET>;CMAKE_C_COMPILER=${CMAKE_C_COMPILER};PREPROCESSED_SOURCE=$<PATH:NATIVE_PATH,<OUTPUT_DIR>/<BASE_NAME>.c>;INCLUDES=;DEFINES=$<LIST:TRANSFORM,<COMPILE_DEFINITIONS>,PREPEND,${CMAKE_C_DEFINE_FLAG}>;FLAGS=")


# Check CHAIN behavior
function(fs_configurator4 rule target fileset outputFileset patterns)
  set_property(FILE_SET ${fileset} TARGET ${target} PROPERTY COMPILE_DEFINITIONS DEF1=1)
endfunction()

add_custom_rule(preprocess3 OUTPUT <OUTPUT_DIR>/<BASE_NAME>.c
  COMMAND "${CMAKE_COMMAND}" -E make_directory "<OUTPUT_DIR>"
  COMMAND ${create_preprocessed_source}
  CONFIGURATOR FOR_FILE_SET fs_configurator4)


function(fs_configurator5 rule target fileset outputFileset patterns)
  set_property(FILE_SET ${fileset} TARGET ${target} APPEND PROPERTY COMPILE_DEFINITIONS DEF2=1)
endfunction()

add_custom_rule(derived_preprocess4 FROM_RULE preprocess3
  CONFIGURATOR FOR_FILE_SET fs_configurator5 CHAIN)

add_library(foo4 STATIC)

target_sources(foo4 PRIVATE FILE_SET fs TYPE derived_preprocess4 FILES file3.c)

set_property(FILE_SET fs TARGET foo4 PROPERTY RULE_PATTERNS
  "INPUT=$<PATH:NATIVE_PATH,<SOURCE>>;OUTPUT_DIR=<CURRENT_BINARY_DIR>/<TARGET>;CMAKE_C_COMPILER=${CMAKE_C_COMPILER};PREPROCESSED_SOURCE=$<PATH:NATIVE_PATH,<OUTPUT_DIR>/<BASE_NAME>.c>;INCLUDES=;DEFINES=$<LIST:TRANSFORM,<COMPILE_DEFINITIONS>,PREPEND,${CMAKE_C_DEFINE_FLAG}>;FLAGS=")
