# <ndk>/build/core/toolchains/x86-clang/setup.mk
set(_ANDROID_ABI_CLANG_TARGET "i686-none-linux-android")

string(APPEND _ANDROID_ABI_INIT_CFLAGS
  " -fPIC"
  )

include(Platform/Android/abi-common-Clang)
