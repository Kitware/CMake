cmake_minimum_required (VERSION 2.6)
PROJECT(CTestTestResourceLock)
INCLUDE(CTest)

ADD_EXECUTABLE (LockFile lockFile.c)

ADD_TEST (TestLockedFile1.1 LockFile locked1.txt)
ADD_TEST (TestLockedFile1.2 LockFile locked1.txt)
SET_TESTS_PROPERTIES(TestLockedFile1.1 TestLockedFile1.2 PROPERTIES RESOURCE_LOCK "locked1.txt")

ADD_TEST (TestLockedFile2.1 LockFile locked2.txt)
ADD_TEST (TestLockedFile2.2 LockFile locked2.txt)
SET_TESTS_PROPERTIES(TestLockedFile2.1 TestLockedFile2.2 PROPERTIES RESOURCE_LOCK "locked2.txt")
