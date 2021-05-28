cmake_minimum_required (VERSION 2.8.12)
project (CTestTestScheduler)
include (CTest)

add_executable (Sleep sleep.c)

foreach (time RANGE 1 4)
  add_test (TestSleep${time} Sleep ${time})
endforeach ()
