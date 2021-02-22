include(FetchContent)

# Do nothing for an update because it would result in always re-running the
# patch step. We want to test that a patch step that only depends on the
# download step is not re-run unnecessarily.
FetchContent_Declare(customCommands
  PREFIX            ${CMAKE_CURRENT_BINARY_DIR}
  DOWNLOAD_COMMAND  "${CMAKE_COMMAND}" -E echo "download executed"
  UPDATE_COMMAND    ""
  PATCH_COMMAND     "${CMAKE_COMMAND}" -E echo "patch executed"
)

set(FETCHCONTENT_QUIET FALSE)
FetchContent_MakeAvailable(customCommands)
