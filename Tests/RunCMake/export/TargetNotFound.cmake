cmake_minimum_required(VERSION 2.8)

project(TargetNotFound)

export(TARGETS nonexistenttarget FILE somefile.cmake)
