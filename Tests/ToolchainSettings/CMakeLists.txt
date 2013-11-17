cmake_minimum_required(VERSION 2.8.12)
cmake_policy(SET CMP0025 NEW)
project(ToolchainSettings)

# Wipe out the install tree
add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/CleanupProject
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/Consumer
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/Producer
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/sysroot
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/stage
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

execute_process(COMMAND
  "${CMAKE_CXX_COMPILER}" -dumpmachine
  OUTPUT_VARIABLE triple
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

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
      -Dtriple=${triple}
      -Dsysroot=${CMAKE_BINARY_DIR}/sysroot
      -DCMAKE_VERBOSE_MAKEFILE=1
  VERBATIM
  )

add_custom_target(ProducerTarget ALL DEPENDS ${CMAKE_BINARY_DIR}/ProducerProject)
add_dependencies(ProducerTarget CleanupTarget)
set_property(
  SOURCE ${CMAKE_BINARY_DIR}/ProducerProject
  PROPERTY SYMBOLIC 1
  )

configure_file(${CMAKE_SOURCE_DIR}/${CMAKE_CXX_COMPILER_ID}-Toolchain.cmake.in
               ${CMAKE_BINARY_DIR}/${CMAKE_CXX_COMPILER_ID}-Toolchain.cmake @ONLY)

write_make_script(Consumer)

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
      -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/InstallationPrefix
      -DCMAKE_TOOLCHAIN_FILE=${CMAKE_BINARY_DIR}/${CMAKE_CXX_COMPILER_ID}-Toolchain.cmake
      -Dtriple=${triple}
      -DCMAKE_VERBOSE_MAKEFILE=1
  VERBATIM
  )
add_custom_target(ToolchainSettingsTest ALL DEPENDS ${CMAKE_BINARY_DIR}/ConsumerProject)
add_dependencies(ToolchainSettingsTest ProducerTarget)
set_property(
  SOURCE ${CMAKE_BINARY_DIR}/ConsumerProject
  PROPERTY SYMBOLIC 1
  )

add_executable(ToolchainSettings main.cpp)
target_compile_definitions(ToolchainSettings
  PRIVATE
    "-DMAKE_OUTPUT=\"${CMAKE_BINARY_DIR}/make_Consumer_output\""
    "-DCOMPILER=\"${CMAKE_CXX_COMPILER}\""
)
add_dependencies(ToolchainSettings ToolchainSettingsTest)
