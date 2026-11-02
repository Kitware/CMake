# Show that the compiler warning about a deprecated symbol includes the
# message given to the deprecation macros.  The objects are only compiled, so
# the symbols need not be exported, and each is built in a separate step that
# checks the warnings.
add_library(deprecated_msg_libshared OBJECT EXCLUDE_FROM_ALL libshared.cpp)

add_library(deprecated_msg_libattributes OBJECT EXCLUDE_FROM_ALL
  libattributes.cpp)
set_property(TARGET deprecated_msg_libattributes PROPERTY CXX_STANDARD 14)

set(targets deprecated_msg_libshared deprecated_msg_libattributes)

# Only GCC and Clang support the GNU spelling that C++11 consumers get.
if(CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang|AppleClang)$")
  add_library(deprecated_msg_libattributes_cxx11 OBJECT EXCLUDE_FROM_ALL
    libattributes.cpp)
  set_property(TARGET deprecated_msg_libattributes_cxx11
    PROPERTY CXX_STANDARD 11)
  list(APPEND targets deprecated_msg_libattributes_cxx11)
endif()

# Undo -Wno-deprecated-declarations from the environment.
if(CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang|AppleClang)$")
  foreach(target IN LISTS targets)
    target_compile_options(${target} PRIVATE -Wdeprecated-declarations)
  endforeach()
endif()

# Messages are unsupported by some compilers, ignored by Oracle Developer
# Studio and not shown by OrangeC.
if(NOT COMPILER_HAS_DEPRECATED_MSG
    OR CMAKE_CXX_COMPILER_ID MATCHES "^(SunPro|OrangeC)$")
  set(targets "")
endif()
list(JOIN targets "\n" targets)
file(WRITE ${CMAKE_BINARY_DIR}/deprecated_msg_targets "${targets}")
