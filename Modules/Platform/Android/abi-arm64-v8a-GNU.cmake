# <ndk>/build/core/toolchains/aarch64-linux-android-4.9/setup.mk
string(APPEND _ANDROID_ABI_INIT_CFLAGS
  " -fpic"
  )

include(Platform/Android/abi-common-GNU)
