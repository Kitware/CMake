PROJECT(TestDriverTest)

SET(Extra_SRCS testExtraStuff.cxx testExtraStuff2.cxx )
SET(Extra_SRCS ${Extra_SRCS};testExtraStuff3.cxx )

CREATE_TEST_SOURCELIST(testSrcs 
                       TestDriverTest 
                       test1 
                       test2 
	               subdir/test3)

ADD_EXECUTABLE(TestDriverTest testSrcs ${Extra_SRCS})

