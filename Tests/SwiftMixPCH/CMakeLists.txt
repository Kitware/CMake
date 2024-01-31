cmake_minimum_required(VERSION 3.15)
if(POLICY CMP0157)
  cmake_policy(SET CMP0157 NEW)
endif()

project(SwiftMixPCH C Swift)

# Swift on Windows only provides a release runtime.
set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreadedDLL)

add_executable(SwiftMixPCH CMain.c SwiftFunc.swift)
target_precompile_headers(SwiftMixPCH PRIVATE pch.h)

# We don't want swift to emit main()
target_compile_options(SwiftMixPCH PRIVATE "$<$<COMPILE_LANGUAGE:Swift>:-parse-as-library>")
