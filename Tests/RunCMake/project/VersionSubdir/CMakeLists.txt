project(WithVersion LANGUAGES NONE VERSION 1.1.1.1)

project(SubdirNoVersion LANGUAGES NONE)

foreach(post "" "_MAJOR" "_MINOR" "_PATCH" "_TWEAK")
  if(NOT DEFINED PROJECT_VERSION${post})
    message(SEND_ERROR "PROJECT_VERSION${post} is not defined when previous project version was provided")
    continue()
  endif()

  if(NOT PROJECT_VERSION${post} STREQUAL "")
    message(SEND_ERROR "PROJECT_VERSION${post} has value '${PROJECT_VERSION${post}}' when empty string is expected")
  endif()
endforeach()

if(NOT CMAKE_PROJECT_VERSION STREQUAL "1.1.1.1")
  message(SEND_ERROR "CMAKE_PROJECT_VERSION has value '${CMAKE_PROJECT_VERION}' when 1.1.1.1 is expected")
endif()

foreach(post "_MAJOR" "_MINOR" "_PATCH" "_TWEAK")
  if(NOT CMAKE_PROJECT_VERSION${post} STREQUAL "1")
    message(SEND_ERROR "CMAKE_PROJECT_VERSION${post} has value '${CMAKE_PROJECT_VERSION${post}}' when 1 is expected")
  endif()
endforeach()
