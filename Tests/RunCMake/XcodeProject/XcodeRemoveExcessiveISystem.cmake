# This file is also included from '../XcodeProject-Device/XcodeRemoveExcessiveISystem.cmake'.

if(NOT "$ENV{CMAKE_OSX_ARCHITECTURES}" MATCHES "[;$]")
  set(USE_SWIFT 1)
else()
  set(USE_SWIFT 0)
endif()

if(IOS)
  set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED NO)
  set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
  if(XCODE_VERSION VERSION_LESS 9)
    set(USE_SWIFT 0)
  endif()
endif ()

enable_language (CXX)

if(USE_SWIFT)
  enable_language (Swift)
  if(NOT XCODE_VERSION VERSION_LESS 10.2)
    set(CMAKE_Swift_LANGUAGE_VERSION 5.0)
  elseif(NOT XCODE_VERSION VERSION_LESS 8.0)
    set(CMAKE_Swift_LANGUAGE_VERSION 3.0)
  endif()
endif()

# Try to find ZLIB in the SDK rather than in system locations.
set(CMAKE_FIND_USE_PACKAGE_ROOT_PATH FALSE)
set(CMAKE_FIND_USE_CMAKE_PATH FALSE)
set(CMAKE_FIND_USE_CMAKE_ENVIRONMENT_PATH FALSE)
set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH FALSE)
list(REMOVE_ITEM CMAKE_SYSTEM_PREFIX_PATH /usr/local /usr / /usr/X11R6 /usr/pkg /opt /sw /opt/local)

find_package(ZLIB REQUIRED)
add_library (framework_dependency STATIC)
target_sources (framework_dependency PRIVATE ${CMAKE_CURRENT_LIST_DIR}/use_cmath.cpp)
target_link_libraries(framework_dependency INTERFACE ZLIB::ZLIB)

add_library (framework_test SHARED ${CMAKE_CURRENT_LIST_DIR}/use_cmath.cpp)
if(USE_SWIFT)
  target_sources(framework_test PRIVATE ${CMAKE_CURRENT_LIST_DIR}/foo.swift)
endif()
target_link_libraries (framework_test PRIVATE framework_dependency)

set_target_properties (framework_test PROPERTIES
    FRAMEWORK TRUE
    FRAMEWORK_VERSION A
    MACOSX_FRAMEWORK_IDENTIFIER "framework.test"
    VERSION "1.0"
    SOVERSION 1.0
  )
