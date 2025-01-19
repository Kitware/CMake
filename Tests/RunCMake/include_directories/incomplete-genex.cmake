enable_language(CXX)

add_library(somelib empty.cpp)

# This test ensures that some internal mechanisms of cmGeneratorExpression
# do not segfault (#14410).

# Test that cmGeneratorExpression::Preprocess(StripAllGeneratorExpressions)
# does not segfault
target_include_directories(somelib PUBLIC
  "/include;/include/$<BUILD_INTERFACE:subdir"
)

# Test that cmGeneratorExpression::Preprocess(BuildInterface) does not segfault
export(TARGETS somelib FILE somelibTargets.cmake)

install(TARGETS somelib EXPORT someExport DESTINATION prefix)
# Test that cmGeneratorExpression::Preprocess(InstallInterface)
# and cmGeneratorExpression::Split do not segfault
install(EXPORT someExport DESTINATION prefix)
