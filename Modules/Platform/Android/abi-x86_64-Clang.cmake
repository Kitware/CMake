# <ndk>/build/core/toolchains/x86_64-clang/setup.mk
set(_ANDROID_ABI_CLANG_TARGET "x86_64-none-linux-android")

string(APPEND _ANDROID_ABI_INIT_CFLAGS
  " -fPIC"
  )

include(Platform/Android/abi-common-Clang)
