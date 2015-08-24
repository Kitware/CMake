cmake_minimum_required(VERSION 3.3)
project(TestCMP0065 CXX)
include(BuildTargetInSubProject.cmake)

BuildTargetInSubProject(TestPolicyCMP0065 FooOLDBad2 FALSE)
