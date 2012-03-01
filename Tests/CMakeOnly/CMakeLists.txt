configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Test.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/Test.cmake @ONLY)

macro(add_CMakeOnly_test test)
  add_test(CMakeOnly.${test} ${CMAKE_CMAKE_COMMAND}
    -DTEST=${test}
    -P ${CMAKE_CURRENT_BINARY_DIR}/Test.cmake
    )
endmacro()

add_CMakeOnly_test(LinkInterfaceLoop)
set_property(TEST CMakeOnly.LinkInterfaceLoop PROPERTY TIMEOUT 90)

add_CMakeOnly_test(CheckSymbolExists)

add_CMakeOnly_test(CheckCXXSymbolExists)

add_CMakeOnly_test(CheckCXXCompilerFlag)

add_CMakeOnly_test(CheckLanguage)

add_CMakeOnly_test(AllFindModules)

add_CMakeOnly_test(TargetScope)

add_test(CMakeOnly.ProjectInclude ${CMAKE_CMAKE_COMMAND}
  -DTEST=ProjectInclude
  -DCMAKE_ARGS=-DCMAKE_PROJECT_ProjectInclude_INCLUDE=${CMAKE_CURRENT_SOURCE_DIR}/ProjectInclude/include.cmake
  -P ${CMAKE_CURRENT_BINARY_DIR}/Test.cmake
  )
