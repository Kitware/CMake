cmake_minimum_required(VERSION 4.3)
project(cxx_modules_export_interface_config CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

add_library(export_interface_config STATIC)
target_sources(export_interface_config
  PUBLIC
    FILE_SET modules TYPE CXX_MODULES
      BASE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}/$<IF:$<CONFIG:Debug>,debug,release>"
      FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/$<IF:$<CONFIG:Debug>,debug,release>/module.cxx"
  )
target_compile_features(export_interface_config PUBLIC cxx_std_20)

install(TARGETS export_interface_config
  EXPORT CXXModules
  FILE_SET modules DESTINATION "lib/cxx/miu"
  )

export(EXPORT CXXModules
  NAMESPACE CXXModules::
  FILE "${CMAKE_CURRENT_BINARY_DIR}/export_interface_config-targets.cmake"
  CXX_MODULES_DIRECTORY "export_interface_config-cxx-modules"
  )
export(PACKAGE_INFO export_interface_config_cps
  EXPORT CXXModules
  CXX_MODULES_DIRECTORY "export_interface_config-cxx-modules-cps"
  )

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/export_interface_config-config.cmake"
  "include(\"\${CMAKE_CURRENT_LIST_DIR}/export_interface_config-targets.cmake\")\n"
  "set(\${CMAKE_FIND_PACKAGE_NAME}_FOUND 1)\n"
  )

set(generator
  -G "${CMAKE_GENERATOR}")
if (CMAKE_GENERATOR_TOOLSET)
  list(APPEND generator
    -T "${CMAKE_GENERATOR_TOOLSET}")
endif ()
if (CMAKE_GENERATOR_PLATFORM)
  list(APPEND generator
    -A "${CMAKE_GENERATOR_PLATFORM}")
endif ()

add_test(NAME export_interface_config_build
  COMMAND
    "${CMAKE_COMMAND}"
    "-Dexpected_source_dir=${CMAKE_CURRENT_SOURCE_DIR}"
    "-Dexpected_binary_dir=${CMAKE_CURRENT_BINARY_DIR}"
    "-DCMAKE_PREFIX_PATH=${CMAKE_CURRENT_BINARY_DIR}"
    ${generator}
    -S "${CMAKE_CURRENT_SOURCE_DIR}/test"
    -B "${CMAKE_CURRENT_BINARY_DIR}/test"
  )
