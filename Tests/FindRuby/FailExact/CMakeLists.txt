cmake_minimum_required(VERSION 3.17)
project(TestRubyFailExact LANGUAGES NONE)

# Should always fail since there is NO ruby 1.9.9 (goes from 1.9.3 to 2.0.0)
find_package(Ruby 1.9.9 EXACT REQUIRED)
if (NOT Ruby_FOUND)
  message (FATAL_ERROR "Failed to find Ruby 1.9.9")
endif()
