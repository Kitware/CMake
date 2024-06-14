set(CMAKE_Swift_SYSROOT_FLAG "-sdk")

# Linker Selections
if("${CMAKE_GENERATOR}" STREQUAL Xcode)
  # Xcode always uses clang to link, regardless of what the cmake link language
  # is. Pass the clang flags when linking with Xcode.
  set(CMAKE_Swift_USING_LINKER_APPLE_CLASSIC "-fuse-ld=ld" "LINKER:-ld_classic")
  set(CMAKE_Swift_USING_LINKER_LLD "-fuse-ld=lld")
  set(CMAKE_Swift_USING_LINKER_SYSTEM "-fuse-ld=ld")
else()
  set(CMAKE_Swift_USING_LINKER_APPLE_CLASSIC "-use-ld=ld" "LINKER:-ld_classic")
  set(CMAKE_Swift_USING_LINKER_LLD "-use-ld=lld")
  set(CMAKE_Swift_USING_LINKER_SYSTEM "-use-ld=ld")
endif()
