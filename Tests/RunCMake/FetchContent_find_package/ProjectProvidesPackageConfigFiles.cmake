include(FetchContent)

FetchContent_Declare(
  ConfigForm1
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/FatalIfAdded
  FIND_PACKAGE_ARGS 1.8 EXACT REQUIRED
)
file(WRITE ${CMAKE_FIND_PACKAGE_REDIRECTS_DIR}/ConfigForm1Config.cmake [[
set(ConfigForm1_FOUND TRUE)
message(STATUS "ConfigForm1 override successful")
]]
)
file(WRITE ${CMAKE_FIND_PACKAGE_REDIRECTS_DIR}/ConfigForm1ConfigVersion.cmake [[
set(PACKAGE_VERSION 1.8)
set(PACKAGE_VERSION_EXACT TRUE)
set(PACKAGE_VERSION_COMPATIBLE TRUE)
]]
)

FetchContent_Declare(
  ConfigForm2
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/FatalIfAdded
  FIND_PACKAGE_ARGS 1.8 REQUIRED
)
file(WRITE ${CMAKE_FIND_PACKAGE_REDIRECTS_DIR}/configform2-config.cmake [[
set(ConfigForm2_FOUND TRUE)
message(STATUS "ConfigForm2 override successful")
]]
)
file(WRITE ${CMAKE_FIND_PACKAGE_REDIRECTS_DIR}/configform2-config-version.cmake [[
set(PACKAGE_VERSION 1.9.7)
set(PACKAGE_VERSION_EXACT FALSE)
set(PACKAGE_VERSION_COMPATIBLE TRUE)
]]
)

FetchContent_MakeAvailable(ConfigForm1 ConfigForm2)

message(STATUS "ConfigForm1_VERSION = ${ConfigForm1_VERSION}")
message(STATUS "ConfigForm2_VERSION = ${ConfigForm2_VERSION}")
