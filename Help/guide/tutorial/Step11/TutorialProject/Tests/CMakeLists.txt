add_executable(TestMathFunctions)

target_sources(TestMathFunctions
  PRIVATE
    TestMathFunctions.cxx
)

find_package(SimpleTest REQUIRED)

target_link_libraries(TestMathFunctions
  PRIVATE
    MathFunctions
    SimpleTest::SimpleTest
)

simpletest_discover_tests(TestMathFunctions)
