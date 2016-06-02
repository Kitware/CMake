# <ndk>/build/core/toolchains/mipsel-linux-android-clang/setup.mk
set(_ANDROID_ABI_CLANG_TARGET "mipsel-none-linux-android")

string(APPEND _ANDROID_ABI_INIT_CFLAGS
  " -fpic"
  )

include(Platform/Android/abi-common-Clang)
