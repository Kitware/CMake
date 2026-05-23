set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO")

add_library(MTestLib SHARED TestLib/TestLib.c TestLib/TestLib.h)
set_target_properties(MTestLib PROPERTIES
  FRAMEWORK 1
  PUBLIC_HEADER TestLib/TestLib.h
  )

add_executable(mapp1 MACOSX_BUNDLE main.m)
add_executable(mapp2 MACOSX_BUNDLE main.m)

set_target_properties(mapp1 PROPERTIES
  XCODE_EMBED_FRAMEWORKS MTestLib
  XCODE_EMBED_FRAMEWORKS_CODE_SIGN_ON_COPY ON
)

set_target_properties(mapp2 PROPERTIES
  XCODE_EMBED_FRAMEWORKS MTestLib
  XCODE_EMBED_FRAMEWORKS_CODE_SIGN_ON_COPY ON
)
