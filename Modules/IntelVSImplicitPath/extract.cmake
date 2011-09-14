file(STRINGS env.txt LIB REGEX "^LIB=.*$")
string(REPLACE "LIB=" "" LIB "${LIB}" )
# change LIB from a string to a ; separated list of paths
set(LIB ${LIB})
# look at each path and try to find ifconsol.lib
foreach( dir ${LIB})
  file(TO_CMAKE_PATH "${dir}" dir)
  if(EXISTS "${dir}/ifconsol.lib")
    file(WRITE implict_link.txt ${dir})
    return()
  endif()
endforeach()
