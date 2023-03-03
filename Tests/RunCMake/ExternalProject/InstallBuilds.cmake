include(ExternalProject)

ExternalProject_Add(InstallBuilds
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/InstallBuilds
  DOWNLOAD_COMMAND ""
  BUILD_COMMAND "${CMAKE_COMMAND}" -E echo "build command suppressed"
  )
