include(ExternalProject)

# Verify COMMAND keyword is recognized after various *_COMMAND options
ExternalProject_Add(multiCommand
  DOWNLOAD_COMMAND  "${CMAKE_COMMAND}" -E echo "download 1"
           COMMAND  "${CMAKE_COMMAND}" -E echo "download 2"
  UPDATE_COMMAND    "${CMAKE_COMMAND}" -E echo "update 1"
         COMMAND    "${CMAKE_COMMAND}" -E echo "update 2"
  PATCH_COMMAND     "${CMAKE_COMMAND}" -E echo "patch 1"
        COMMAND     "${CMAKE_COMMAND}" -E echo "patch 2"
  CONFIGURE_COMMAND "${CMAKE_COMMAND}" -E echo "configure 1"
            COMMAND "${CMAKE_COMMAND}" -E echo "configure 2"
  BUILD_COMMAND     "${CMAKE_COMMAND}" -E echo "build 1"
        COMMAND     "${CMAKE_COMMAND}" -E echo "build 2"
  TEST_COMMAND      "${CMAKE_COMMAND}" -E echo "test 1"
       COMMAND      "${CMAKE_COMMAND}" -E echo "test 2"
  INSTALL_COMMAND   "${CMAKE_COMMAND}" -E echo "install 1"
          COMMAND   "${CMAKE_COMMAND}" -E echo "install 2"
)

# Force all steps to be re-run by removing timestamps from any previous run
ExternalProject_Get_Property(multiCommand STAMP_DIR)
file(REMOVE_RECURSE "${STAMP_DIR}")
file(MAKE_DIRECTORY "${STAMP_DIR}")
