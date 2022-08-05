include(FetchContent)

set(FETCHCONTENT_QUIET NO)

FetchContent_Declare(SomeDep
  DOWNLOAD_COMMAND ${CMAKE_COMMAND} -E echo "Download command called"
)
FetchContent_MakeAvailable(SomeDep)
