# <ndk>/sources/cxx-stl/llvm-libc++/Android.mk
set(_ANDROID_STL_RTTI 1)
set(_ANDROID_STL_EXCEPTIONS 1)
macro(__android_stl_cxx lang filename)
  # Add the include directory.
  __android_stl_inc(${lang} "${CMAKE_ANDROID_NDK}/sources/cxx-stl/llvm-libc++/libcxx/include" 1)

  # Add a secondary include directory if it exists.
  __android_stl_inc(${lang} "${CMAKE_ANDROID_NDK}/sources/android/support/include" 0)

  # Add the library file.
  __android_stl_lib(${lang} "${CMAKE_ANDROID_NDK}/sources/cxx-stl/llvm-libc++/libs/${CMAKE_ANDROID_ARCH_ABI}/${filename}" 1)
endmacro()
