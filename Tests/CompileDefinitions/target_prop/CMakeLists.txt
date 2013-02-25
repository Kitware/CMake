
project(target_prop)

add_executable(target_prop_executable ../compiletest.cpp)

set_target_properties(target_prop_executable PROPERTIES COMPILE_DEFINITIONS CMAKE_IS_FUN)

set_property(TARGET target_prop_executable APPEND PROPERTY COMPILE_DEFINITIONS CMAKE_IS_REALLY="Very Fun" CMAKE_IS=Fun)
set_property(TARGET target_prop_executable APPEND PROPERTY COMPILE_DEFINITIONS CMAKE_IS_FUN CMAKE_IS_="Fun")

set_property(TARGET target_prop_executable APPEND PROPERTY COMPILE_DEFINITIONS
  TEST_GENERATOR_EXPRESSIONS
    "$<1:CMAKE_IS_DECLARATIVE>"
    "$<0:GE_NOT_DEFINED>"
    "$<1:ARGUMENT;LIST>"
    PREFIX_$<JOIN:DEF1;DEF2,;PREFIX_>
    LETTER_LIST1=\"$<JOIN:A;B;C;D,,>\"
    LETTER_LIST2=\"$<JOIN:A;B;C;D,,,>\"
    LETTER_LIST3=\"$<JOIN:A;B;C;D,,->\"
    LETTER_LIST4=\"$<JOIN:A;B;C;D,-,->\"
    LETTER_LIST5=\"$<JOIN:A;B;C;D,-,>\"
)

set_property(TARGET target_prop_executable APPEND PROPERTY COMPILE_DEFINITIONS
  BUILD_IS_DEBUG=$<CONFIG:Debug>
  BUILD_IS_NOT_DEBUG=$<NOT:$<CONFIG:Debug>>
  )
