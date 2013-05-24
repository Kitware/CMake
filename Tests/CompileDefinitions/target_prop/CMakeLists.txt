
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
    "$<$<LINK_LANGUAGE:CXX>:LINK_CXX_DEFINE>"
    "$<$<LINK_LANGUAGE:C>:LINK_C_DEFINE>"
    "LINK_LANGUAGE_IS_$<LINK_LANGUAGE>"
)

set_property(TARGET target_prop_executable APPEND PROPERTY COMPILE_DEFINITIONS
  BUILD_IS_DEBUG=$<CONFIG:Debug>
  BUILD_IS_NOT_DEBUG=$<NOT:$<CONFIG:Debug>>
  )

add_executable(target_prop_c_executable ../compiletest.c)

set_property(TARGET target_prop_c_executable APPEND PROPERTY COMPILE_DEFINITIONS
    "$<$<LINK_LANGUAGE:CXX>:LINK_CXX_DEFINE>"
    "$<$<LINK_LANGUAGE:C>:LINK_C_DEFINE>"
    "LINK_LANGUAGE_IS_$<LINK_LANGUAGE>"
    )

# Resulting link language will be CXX
add_executable(target_prop_mixed_executable ../compiletest_mixed_c.c ../compiletest_mixed_cxx.cpp)

set_property(TARGET target_prop_mixed_executable APPEND PROPERTY COMPILE_DEFINITIONS
    "$<$<LINK_LANGUAGE:CXX>:LINK_CXX_DEFINE>"
    "$<$<LINK_LANGUAGE:C>:LINK_C_DEFINE>"
    "LINK_LANGUAGE_IS_$<LINK_LANGUAGE>"
    )
