cmake_minimum_required(VERSION 3.14)
project(embt_binutils C)
# The parent test reads these back to assert each tool was pinned.
foreach(t LINKER NM OBJDUMP STRIP DLLTOOL)
  message(STATUS "BINUTIL ${t}=[${CMAKE_${t}}]")
endforeach()
