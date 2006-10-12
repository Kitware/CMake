MESSAGE("This is packaging script")
MESSAGE("It writes a file with all variables available in ${CMAKE_INSTALL_PREFIX}/AllVariables.txt")

FILE(WRITE ${CMAKE_INSTALL_PREFIX}/AllVariables.txt "")
GET_CMAKE_PROPERTY(res VARIABLES)
FOREACH(var ${res})
  FILE(APPEND ${CMAKE_INSTALL_PREFIX}/AllVariables.txt 
             "${var} \"${${var}}\"\n")
ENDFOREACH(var ${res})

