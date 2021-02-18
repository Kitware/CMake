include(FetchContent)

# Verify COMMAND keyword is recognised after various *_COMMAND options
FetchContent_Declare(multiCommand
  DOWNLOAD_COMMAND  "${CMAKE_COMMAND}" -E echo "download 1"
           COMMAND  "${CMAKE_COMMAND}" -E echo "download 2"
  UPDATE_COMMAND    "${CMAKE_COMMAND}" -E echo "update 1"
         COMMAND    "${CMAKE_COMMAND}" -E echo "update 2"
  PATCH_COMMAND     "${CMAKE_COMMAND}" -E echo "patch 1"
        COMMAND     "${CMAKE_COMMAND}" -E echo "patch 2"
)

# Force all steps to be re-run by removing timestamps, scripts, etc. from any
# previous run
file(REMOVE_RECURSE "${FETCHCONTENT_BASE_DIR}/multiCommand-subbuild")

set(FETCHCONTENT_QUIET FALSE)
FetchContent_MakeAvailable(multiCommand)
