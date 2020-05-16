cmake_minimum_required(VERSION 3.13)
project(ProjectSubdir LANGUAGES NONE)

set_property(GLOBAL PROPERTY FetchWithSubProject YES)
message(STATUS "Confirmation subproject has been added")
