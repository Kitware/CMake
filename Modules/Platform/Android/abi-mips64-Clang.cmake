# <ndk>/build/core/toolchains/mips64el-linux-android-clang/setup.mk
set(_ANDROID_ABI_CLANG_TARGET "mips64el-none-linux-android")

string(APPEND _ANDROID_ABI_INIT_CFLAGS
  " -fpic"
  )

include(Platform/Android/abi-common-Clang)
