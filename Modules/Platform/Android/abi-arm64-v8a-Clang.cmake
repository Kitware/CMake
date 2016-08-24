# <ndk>/build/core/toolchains/aarch64-linux-android-clang/setup.mk
set(_ANDROID_ABI_CLANG_TARGET "aarch64-none-linux-android")

string(APPEND _ANDROID_ABI_INIT_CFLAGS
  " -fpic"
  )

include(Platform/Android/abi-common-Clang)
