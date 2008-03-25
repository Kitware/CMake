cmake_minimum_required (VERSION 2.6)
PROJECT(TestDriverTest)

SET(Extra_SRCS testExtraStuff.cxx testExtraStuff2.cxx )
SET(Extra_SRCS ${Extra_SRCS};testExtraStuff3.cxx )
INCLUDE_DIRECTORIES(${TestDriverTest_SOURCE_DIR})
CREATE_TEST_SOURCELIST(testSrcs 
                       TestDriverTest.cxx 
                       test1.cxx 
                       test2.cxx 
                       subdir/test3.cxx 
                       EXTRA_INCLUDE testArgs.h FUNCTION testProccessArgs)

ADD_EXECUTABLE(TestDriverTest ${testSrcs} ${Extra_SRCS})

