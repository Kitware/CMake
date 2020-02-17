# Set Qt test version and include the Autogen test macros
set(QT_TEST_VERSION 5)
include("../QtAutogen/TestMacros.cmake")

# Common tests
include("../QtAutogen/Tests.cmake")

set(TEST_BUILD_DIRS "${TEST_BUILD_DIRS}" PARENT_SCOPE)
