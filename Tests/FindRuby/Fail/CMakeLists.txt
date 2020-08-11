cmake_minimum_required(VERSION 3.17)
project(TestRubyFail LANGUAGES NONE)

# Should always fail since there is NO ruby 50.1.0 yet.
find_package(Ruby 50.1.0 REQUIRED)
