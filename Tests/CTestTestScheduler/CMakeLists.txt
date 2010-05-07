CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
PROJECT (CTestTestScheduler)
INCLUDE (CTest)

ADD_EXECUTABLE (Sleep sleep.c)

FOREACH (time RANGE 1 4)
  ADD_TEST (TestSleep${time} Sleep ${time})
ENDFOREACH (time RANGE 1 4)
