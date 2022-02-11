include(ExternalProject)

if(CMAKE_GENERATOR STREQUAL "Borland Makefiles" OR
   CMAKE_GENERATOR STREQUAL "Watcom WMake")
  set(fs_delay 3)
else()
  set(fs_delay 1.125)
endif()

# Given this setup, on the first build, both configure steps and both build
# steps will run. On a noop rebuild, only the build steps will run. Without
# CONFIGURE_HANDLED_BY_BUILD, the configure step of proj2 would also run on a
# noop rebuild.

ExternalProject_Add(proj1
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ""
  CONFIGURE_COMMAND ${CMAKE_COMMAND} -E echo "Doing something"
  # file(TIMESTAMP) gives back the timestamp in seconds so we sleep a second to
  # make sure we get a different timestamp on the stamp file
  BUILD_COMMAND ${CMAKE_COMMAND} -E sleep ${fs_delay}
  INSTALL_COMMAND ""
  BUILD_ALWAYS ON
  STAMP_DIR "stamp"
)
ExternalProject_Add(proj2
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ""
  CONFIGURE_COMMAND ${CMAKE_COMMAND} -E echo "Doing something"
  BUILD_COMMAND ${CMAKE_COMMAND} -E sleep ${fs_delay}
  INSTALL_COMMAND ""
  CONFIGURE_HANDLED_BY_BUILD ON
  DEPENDS proj1
  STAMP_DIR "stamp"
)
