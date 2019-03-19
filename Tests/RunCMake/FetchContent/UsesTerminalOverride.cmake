include(FetchContent)

set(FETCHCONTENT_QUIET NO)

FetchContent_Declare(
  t1
  DOWNLOAD_COMMAND ${CMAKE_COMMAND} -E echo "Logged from t1 download step"
  USES_TERMINAL_DOWNLOAD NO

)
FetchContent_Populate(t1)

FetchContent_Populate(
  t2
  DOWNLOAD_COMMAND ${CMAKE_COMMAND} -E echo "Logged from t2 download step"
  USES_TERMINAL_DOWNLOAD NO
)
