cmake_minimum_required(VERSION 3.3)
project(SwiftMix C Swift)

add_executable(SwiftMix CMain.c ObjCMain.m SwiftMain.swift ObjC-Swift.h)
set_property(TARGET SwiftMix PROPERTY XCODE_ATTRIBUTE_SWIFT_OBJC_BRIDGING_HEADER "ObjC-Swift.h")
target_compile_options(SwiftMix PRIVATE "$<$<COMPILE_LANGUAGE:C>:-Werror=objc-method-access>")
target_compile_definitions(SwiftMix PRIVATE "$<IF:$<COMPILE_LANGUAGE:Swift>,SWIFTCOND,CCOND=2>" FOO BAR=3)
