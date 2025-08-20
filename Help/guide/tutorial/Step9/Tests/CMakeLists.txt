add_executable(TestMathFunctions)

target_sources(TestMathFunctions
  PRIVATE
    TestMathFunctions.cxx
)

target_link_libraries(TestMathFunctions
  PRIVATE
    MathFunctions
)

function(MathFunctionTest op)
  add_test(
    NAME ${op}
    COMMAND TestMathFunctions ${op}
  )
endfunction()

MathFunctionTest(add)
MathFunctionTest(mul)
MathFunctionTest(sqrt)
MathFunctionTest(sub)
