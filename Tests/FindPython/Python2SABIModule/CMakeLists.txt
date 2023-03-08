cmake_minimum_required(VERSION 3.5)

project(TestPython2SABIModule LANGUAGES C)

find_package(Python2 REQUIRED COMPONENTS Interpreter Development.Module Development.SABIModule)
