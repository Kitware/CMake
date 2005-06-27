# a simple test case
project (simple)

add_executable (simple simple.cxx)

add_library (simpleLib STATIC 
  simpleLib.cxx 
  simpleCLib.c 
  simpleWe.cpp
  )

target_link_libraries (simple simpleLib)
