include(ExternalProject)
cmake_policy(SET CMP0114 NEW)

ExternalProject_Add(proj
  SOURCE_DIR "."
  DOWNLOAD_COMMAND ""
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  )
ExternalProject_Add_Step(proj custom
  DEPENDEES configure
  INDEPENDENT 1
  )
