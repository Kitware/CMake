# TODO: In the future this should be a policy test, but for now verify version
# variables remain undefined for various configurations.

project(NoVersion LANGUAGES NONE)

foreach(pre "NoVersion_" "PROJECT_" "CMAKE_PROJECT_")
  foreach(post "" "_MAJOR" "_MINOR" "_PATCH" "_TWEAK")
    if(DEFINED ${pre}VERSION${post})
      message(SEND_ERROR "${pre}VERSION${post} is defined when no project version was provided")
    endif()
  endforeach()
endforeach()

add_subdirectory(VersionSubdir)
