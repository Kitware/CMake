cmake_minimum_required(VERSION 3.18)
project(CSharpWin32GenEx CSharp)

add_executable(CSharpWin32GenEx csharpwin32genex.cs)
set_property(TARGET CSharpWin32GenEx PROPERTY WIN32_EXECUTABLE $<1:1>)
