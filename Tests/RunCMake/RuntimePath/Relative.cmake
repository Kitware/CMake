enable_language(C)

set(cfg_up)
set(cfg_slash /)
if(cfg_dir)
  set(cfg_up /..)
  set(cfg_slash)
endif()

if(NOT CMAKE_SHARED_LIBRARY_RPATH_ORIGIN_TOKEN)
  if(CMAKE_C_PLATFORM_ID STREQUAL "Linux")
    # Sanity check for platform that is definitely known to support $ORIGIN.
    message(FATAL_ERROR "Platform fails to report relative RPATH support")
  else()
    message(STATUS "Platform does not support relative RPATHs, skipping")
  endif()
  return()
endif()
set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)

function(CheckRpath target rpath)
  add_custom_command(
    TARGET ${target}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -Dfile=$<TARGET_FILE:${target}> -Drpath=${rpath}
            -P "${CMAKE_CURRENT_SOURCE_DIR}/RelativeCheck.cmake"
    VERBATIM
  )
endfunction()

if(CMAKE_C_COMPILER_ID STREQUAL "XL" AND CMAKE_BINARY_DIR MATCHES " ")
  # XL 16.1.0.0 fails building the library if the output path contains a space.
  set(externDir)
  message(STATUS "Skipping external library test because of a toolchain bug")
else()
  get_filename_component(externDir "${CMAKE_BINARY_DIR}" DIRECTORY)
  set(externDir "${externDir}/Relative-extern")
endif()

add_library(utils SHARED A.c)
add_library(utils-sub SHARED A.c)
set_property(TARGET utils-sub PROPERTY LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/libs)
if(externDir)
  add_library(utils-extern SHARED A.c)
  set_property(TARGET utils-extern PROPERTY LIBRARY_OUTPUT_DIRECTORY ${externDir})
endif()

add_executable(main main.c)
target_link_libraries(main utils)
CheckRpath(main "\$ORIGIN")

add_executable(main-norel main.c)
target_link_libraries(main-norel utils)
set_property(TARGET main-norel PROPERTY BUILD_RPATH_USE_ORIGIN OFF)
CheckRpath(main-norel "${CMAKE_CURRENT_BINARY_DIR}${cfg_dir}")

add_executable(mainsub main.c)
target_link_libraries(mainsub utils)
set_property(TARGET mainsub PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
CheckRpath(mainsub "\$ORIGIN${cfg_up}/..${cfg_dir}${cfg_slash}")

add_executable(main-sub main.c)
target_link_libraries(main-sub utils-sub)
CheckRpath(main-sub "\$ORIGIN${cfg_up}/libs${cfg_dir}")

add_executable(mainsub-sub main.c)
target_link_libraries(mainsub-sub utils-sub)
set_property(TARGET mainsub-sub PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
CheckRpath(mainsub-sub "\$ORIGIN${cfg_up}/../libs${cfg_dir}")

if(externDir)
  # Binaries linking to libraries outside the build tree should have an absolute RPATH.
  add_executable(main-extern main.c)
  target_link_libraries(main-extern utils-extern)
  CheckRpath(main-extern "${externDir}${cfg_dir}")
endif()
