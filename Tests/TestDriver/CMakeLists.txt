PROJECT(TestDriverTest)

CREATE_TEST_SOURCELIST(testSrcs 
                       TestDriverTest 
                       test1 
                       test2 
	               subdir/test3)

ADD_EXECUTABLE(TestDriverTest testSrcs)

