enable_language(C)

find_package(foo REQUIRED CONFIG NO_DEFAULT_PATH)

add_executable(main main.c)
target_link_libraries(main PRIVATE foo-install::foo)

get_property(enable_exports TARGET foo-install::foo PROPERTY ENABLE_EXPORTS)
if (CMAKE_TAPI AND NOT enable_exports)
  message(SEND_ERROR "foo-install::foo: ENABLE_EXPORTS not set.")
endif()

get_property(implib TARGET foo-install::foo PROPERTY IMPORTED_IMPLIB_RELEASE)
if (CMAKE_TAPI AND NOT implib)
  message(SEND_ERROR "foo-install::foo: IMPORTED_IMPLIB_RELEASE not set.")
endif()
if (CMAKE_TAPI AND NOT implib MATCHES "Release/libfoo.tbd$")
  message(SEND_ERROR "foo-install::foo: ${implib}: wrong value for IMPORTED_IMPLIB_RELEASE.")
endif()

get_property(location TARGET foo-install::foo PROPERTY IMPORTED_LOCATION_RELEASE)
if (NOT location)
  message(SEND_ERROR "foo-install::foo: IMPORTED_LOCATION_RELEASE not set.")
endif()
if (NOT location MATCHES "Release/libfoo.dylib$")
  message(SEND_ERROR "foo-install::foo: ${location}: wrong value for IMPORTED_LOCATION_RELEASE.")
endif()


include(${foo_BUILD}/foo.cmake)

add_executable(main2 main.c)
target_link_libraries(main2 PRIVATE foo-build::foo)

get_property(enable_exports TARGET foo-build::foo PROPERTY ENABLE_EXPORTS)
if (CMAKE_TAPI AND NOT enable_exports)
  message(SEND_ERROR "foo-build::foo: ENABLE_EXPORTS not set.")
endif()

get_property(implib TARGET foo-build::foo PROPERTY IMPORTED_IMPLIB_RELEASE)
if (CMAKE_TAPI AND NOT implib)
  message(SEND_ERROR "foo-build::foo: IMPORTED_IMPLIB_RELEASE not set.")
endif()
if (CMAKE_TAPI AND NOT implib STREQUAL "${foo_BUILD}/libfoo.tbd")
  message(SEND_ERROR "foo-build::foo: ${implib}: wrong value for IMPORTED_IMPLIB_RELEASE.")
endif()

get_property(location TARGET foo-build::foo PROPERTY IMPORTED_LOCATION_RELEASE)
if (NOT location)
  message(SEND_ERROR "foo-build::foo: IMPORTED_LOCATION_RELEASE not set.")
endif()
if (NOT location STREQUAL "${foo_BUILD}/libfoo.dylib")
  message(SEND_ERROR "foo-build::foo: ${location}: wrong value for IMPORTED_LOCATION_RELEASE.")
endif()
