cmake_minimum_required(VERSION 3.29)

# This is different to the usual RunCMake test pattern. We are specifically
# testing the scenario where enable_language() is called before project().
include(${RunCMake_TEST}.cmake)

project(${RunCMake_TEST} NONE)
