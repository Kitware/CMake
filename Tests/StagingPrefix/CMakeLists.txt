
cmake_minimum_required(VERSION 2.8.12)
project(StagingPrefix)

# Wipe out the install tree
add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/CleanupProject
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/Consumer
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/Producer
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/stage
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/prefix
  )
add_custom_target(CleanupTarget ALL DEPENDS ${CMAKE_BINARY_DIR}/CleanupProject)
set_property(
  SOURCE ${CMAKE_BINARY_DIR}/CleanupProject
  PROPERTY SYMBOLIC 1
  )

if(CMAKE_CONFIGURATION_TYPES)
  set(NESTED_CONFIG_TYPE -C "${CMAKE_CFG_INTDIR}")
else()
  if(CMAKE_BUILD_TYPE)
    set(NESTED_CONFIG_TYPE -C "${CMAKE_BUILD_TYPE}")
  else()
    set(NESTED_CONFIG_TYPE)
  endif()
endif()

macro(write_make_script name)
  file(WRITE "${CMAKE_BINARY_DIR}/CMakeFiles/Make${name}.sh"
    "#!/bin/sh\n${CMAKE_MAKE_PROGRAM} install > \"${CMAKE_BINARY_DIR}/make_${name}_output\"\n"
  )
  file(COPY "${CMAKE_BINARY_DIR}/CMakeFiles/Make${name}.sh"
    DESTINATION "${CMAKE_BINARY_DIR}"
    FILE_PERMISSIONS
      OWNER_READ OWNER_EXECUTE
  )
endmacro()

write_make_script(Producer)

# Build and install the producer.
add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/ProducerProject
  COMMAND ${CMAKE_CTEST_COMMAND} ${NESTED_CONFIG_TYPE}
    --build-and-test
    ${CMAKE_SOURCE_DIR}/Producer
    ${CMAKE_BINARY_DIR}/Producer
    --build-noclean
    --build-project Producer
    --build-generator ${CMAKE_GENERATOR}
    --build-generator-toolset "${CMAKE_GENERATOR_TOOLSET}"
    --build-makeprogram "${CMAKE_BINARY_DIR}/MakeProducer.sh"
    --build-options
      -DCMAKE_VERBOSE_MAKEFILE=1
      "-DCMAKE_STAGING_PREFIX=${CMAKE_BINARY_DIR}/stage"
      "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/prefix"
  VERBATIM
  )

add_custom_target(ProducerTarget ALL DEPENDS ${CMAKE_BINARY_DIR}/ProducerProject)
add_dependencies(ProducerTarget CleanupTarget)
set_property(
  SOURCE ${CMAKE_BINARY_DIR}/ProducerProject
  PROPERTY SYMBOLIC 1
  )

write_make_script(Consumer)

if(NOT WIN32)
  file(WRITE
    "${CMAKE_BINARY_DIR}/ignored/${CMAKE_BINARY_DIR}/stage/include/ignored.h"
    "#define IGNORED\n"
  )
endif()

# Build and install the consumer.
add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/ConsumerProject
  COMMAND ${CMAKE_CTEST_COMMAND} ${NESTED_CONFIG_TYPE}
   --build-and-test
   ${CMAKE_SOURCE_DIR}/Consumer
   ${CMAKE_BINARY_DIR}/Consumer
   --build-noclean
   --build-project Consumer
   --build-generator ${CMAKE_GENERATOR}
   --build-generator-toolset "${CMAKE_GENERATOR_TOOLSET}"
   --build-makeprogram "${CMAKE_BINARY_DIR}/MakeConsumer.sh"
   --build-options
      "-DCMAKE_FIND_ROOT_PATH=${CMAKE_BINARY_DIR}/ignored"
      "-DCMAKE_STAGING_PREFIX=${CMAKE_BINARY_DIR}/stage"
      "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/InstallationPrefix"
      -DCMAKE_VERBOSE_MAKEFILE=1
  VERBATIM
  )
add_custom_target(StagingPrefixTest ALL DEPENDS ${CMAKE_BINARY_DIR}/ConsumerProject)
add_dependencies(StagingPrefixTest ProducerTarget)
set_property(
  SOURCE ${CMAKE_BINARY_DIR}/ConsumerProject
  PROPERTY SYMBOLIC 1
  )

add_executable(StagingPrefix main.cpp)
target_compile_definitions(StagingPrefix
  PRIVATE
    "-DMAKE_OUTPUT=\"${CMAKE_BINARY_DIR}/make_Consumer_output\""
    "-DCOMPILER=\"${CMAKE_CXX_COMPILER}\""
)
add_dependencies(StagingPrefix StagingPrefixTest)
