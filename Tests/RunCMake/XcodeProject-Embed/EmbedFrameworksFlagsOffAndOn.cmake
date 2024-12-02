set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO")

try_compile(TESTLIB_FRAMEWORK_COMPILED
  PROJECT TestLib
  SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/TestLib
  BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/TestLib
  )
if(NOT TESTLIB_FRAMEWORK_COMPILED)
  message(FATAL_ERROR "TestLib.framework did not compile")
endif()
set(TestLib_framework "${CMAKE_CURRENT_BINARY_DIR}/TestLib/Debug/TestLib.framework")
if(NOT EXISTS "${TestLib_framework}/Headers/TestLib.h")
  message(FATAL_ERROR "TestLib.framework did not build with header")
endif()

add_executable(app1 MACOSX_BUNDLE main.m)
add_executable(app2 MACOSX_BUNDLE main.m)

set_target_properties(app1 PROPERTIES
  XCODE_EMBED_FRAMEWORKS "${CMAKE_CURRENT_BINARY_DIR}/TestLib/Debug/TestLib.framework"
  XCODE_EMBED_FRAMEWORKS_REMOVE_HEADERS_ON_COPY OFF
)

set_target_properties(app2 PROPERTIES
  XCODE_EMBED_FRAMEWORKS "${CMAKE_CURRENT_BINARY_DIR}/TestLib/Debug/TestLib.framework"
  XCODE_EMBED_FRAMEWORKS_REMOVE_HEADERS_ON_COPY ON
)
