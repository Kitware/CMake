project(dumb)
project(dumber)
add_executable(just_silly silly.c)

if(DEFINED PROJECT_VERSION)
  message(SEND_ERROR "PROJECT_VERSION defined, but should not !")
endif()

project(Foo VERSION "1.2.3.4" C)

if(NOT Foo_VERSION STREQUAL "1.2.3.4")
  message(SEND_ERROR "Foo_VERSION set to ${Foo_VERSION} ! (expected 1.2.3.4 !")
endif()

if(NOT "${Foo_VERSION_MAJOR}.${Foo_VERSION_MINOR}.${Foo_VERSION_PATCH}.${Foo_VERSION_TWEAK}" STREQUAL "1.2.3.4")
  message(SEND_ERROR "Foo_VERSION set to \"${Foo_VERSION_MAJOR}.${Foo_VERSION_MINOR}.${Foo_VERSION_PATCH}.${Foo_VERSION_TWEAK}\"! (expected 1.2.3.4 !")
endif()

if(NOT Foo_VERSION STREQUAL PROJECT_VERSION)
  message(SEND_ERROR "Foo_VERSION differs from PROJECT_VERSION ! (${Foo_VERSION} != ${PROJECT_VERSION})")
endif()

project(Bar CXX)

if(DEFINED PROJECT_VERSION)
  message(SEND_ERROR "PROJECT_VERSION defined (\"${PROJECT_VERSION}\"), but should not !")
endif()

if(DEFINED Bar_VERSION)
  message(SEND_ERROR "Bar_VERSION defined, but should not !")
endif()


# test that PROJECT_VERSION is not reset if it hasn't been set by project(VERSION)

set(PROJECT_VERSION "5.6.7")

project(Blub)

if(NOT "${PROJECT_VERSION}" STREQUAL "5.6.7")
  message(SEND_ERROR "PROJECT_VERSION unset, but should not have been unset since it wasn't set by project(VERSION) before.")
endif()



project(Xyz VERSION "7.8.9")

set(CMAKE_PROJECT_VERSION_SET_BY_PROJECT_COMMAND FALSE)

set(PROJECT_VERSION "3.2.1")

project(Abc)

if(NOT "${PROJECT_VERSION}" STREQUAL "3.2.1")
  message(SEND_ERROR "PROJECT_VERSION set to \"${PROJECT_VERSION}\", but should still be 3.2.1, since unsetting was disabled .")
endif()
