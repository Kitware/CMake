# - This module determines the thread library of the system.
# The following variables are set
#  CMAKE_THREAD_LIBS_INIT     - the thread library
#  CMAKE_USE_SPROC_INIT       - are we using sproc?
#  CMAKE_USE_WIN32_THREADS_INIT - using WIN32 threads?
#  CMAKE_USE_PTHREADS_INIT    - are we using pthreads
#  CMAKE_HP_PTHREADS_INIT     - are we using hp pthreads

INCLUDE (CheckIncludeFiles)
INCLUDE (CheckLibraryExists)

# Do we have sproc?
IF(CMAKE_SYSTEM MATCHES IRIX)
  CHECK_INCLUDE_FILES("sys/types.h;sys/prctl.h"  CMAKE_HAVE_SPROC_H)
ENDIF(CMAKE_SYSTEM MATCHES IRIX)

IF(CMAKE_HAVE_SPROC_H)
  # We have sproc
  SET(CMAKE_USE_SPROC_INIT 1)
ELSE(CMAKE_HAVE_SPROC_H)
  # Do we have pthreads?
  CHECK_INCLUDE_FILES("pthread.h" CMAKE_HAVE_PTHREAD_H)
  IF(CMAKE_HAVE_PTHREAD_H)
    # We have pthread.h
    # Let's check for the library now.
    SET(CMAKE_HAVE_THREADS_LIBRARY)
    IF(NOT THREADS_HAVE_PTHREAD_ARG)
      # Do we have -lpthreads
      CHECK_LIBRARY_EXISTS(pthreads pthread_create "" CMAKE_HAVE_PTHREADS_CREATE)
      IF(CMAKE_HAVE_PTHREADS_CREATE)
        SET(CMAKE_THREAD_LIBS_INIT "-lpthreads")
        SET(CMAKE_HAVE_THREADS_LIBRARY 1)
      ENDIF(CMAKE_HAVE_PTHREADS_CREATE)
      # Ok, how about -lpthread
      CHECK_LIBRARY_EXISTS(pthread pthread_create "" CMAKE_HAVE_PTHREAD_CREATE)
      IF(CMAKE_HAVE_PTHREAD_CREATE)
        SET(CMAKE_THREAD_LIBS_INIT "-lpthread")
        SET(CMAKE_HAVE_THREADS_LIBRARY 1)
      ENDIF(CMAKE_HAVE_PTHREAD_CREATE)
      IF(CMAKE_SYSTEM MATCHES "SunOS.*")
        # On sun also check for -lthread
        CHECK_LIBRARY_EXISTS(thread thr_create "" CMAKE_HAVE_THR_CREATE)
        IF(CMAKE_HAVE_THR_CREATE)
          SET(CMAKE_THREAD_LIBS_INIT "-lthread")
          SET(CMAKE_HAVE_THREADS_LIBRARY 1)
        ENDIF(CMAKE_HAVE_THR_CREATE)
      ENDIF(CMAKE_SYSTEM MATCHES "SunOS.*")
    ENDIF(NOT THREADS_HAVE_PTHREAD_ARG)

    IF(NOT CMAKE_HAVE_THREADS_LIBRARY)
      # If we did not found -lpthread, -lpthread, or -lthread, look for -pthread
      IF("THREADS_HAVE_PTHREAD_ARG" MATCHES "^THREADS_HAVE_PTHREAD_ARG")
        MESSAGE(STATUS "Check if compiler accepts -pthread")
        TRY_RUN(THREADS_PTHREAD_ARG THREADS_HAVE_PTHREAD_ARG
          ${CMAKE_BINARY_DIR}
          ${CMAKE_ROOT}/Modules/CheckForPthreads.c
          CMAKE_FLAGS -DLINK_LIBRARIES:STRING=-pthread
          COMPILE_OUTPUT_VARIABLE OUTPUT)
        IF(THREADS_HAVE_PTHREAD_ARG)
          IF(THREADS_PTHREAD_ARG MATCHES "^2$")
            MESSAGE(STATUS "Check if compiler accepts -pthread - yes")
          ELSE(THREADS_PTHREAD_ARG MATCHES "^2$")
            MESSAGE(STATUS "Check if compiler accepts -pthread - no")
            FILE(APPEND 
              ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log 
              "Determining if compiler accepts -pthread returned ${THREADS_PTHREAD_ARG} instead of 2. The compiler had the following output:\n${OUTPUT}\n\n")
          ENDIF(THREADS_PTHREAD_ARG MATCHES "^2$")
        ELSE(THREADS_HAVE_PTHREAD_ARG)
          MESSAGE(STATUS "Check if compiler accepts -pthread - no")
          FILE(APPEND 
            ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log 
            "Determining if compiler accepts -pthread failed with the following output:\n${OUTPUT}\n\n")
        ENDIF(THREADS_HAVE_PTHREAD_ARG)
      ENDIF("THREADS_HAVE_PTHREAD_ARG" MATCHES "^THREADS_HAVE_PTHREAD_ARG")
      IF(THREADS_HAVE_PTHREAD_ARG)
        SET(CMAKE_THREAD_LIBS_INIT "-pthread")
      ENDIF(THREADS_HAVE_PTHREAD_ARG)
    ENDIF(NOT CMAKE_HAVE_THREADS_LIBRARY)
  ENDIF(CMAKE_HAVE_PTHREAD_H)
ENDIF(CMAKE_HAVE_SPROC_H)

IF(CMAKE_THREAD_LIBS_INIT)
  SET(CMAKE_USE_PTHREADS_INIT 1)
ENDIF(CMAKE_THREAD_LIBS_INIT)

IF(CMAKE_SYSTEM MATCHES "Windows")
  SET(CMAKE_USE_WIN32_THREADS_INIT 1)
ENDIF(CMAKE_SYSTEM MATCHES "Windows")

IF(CMAKE_USE_PTHREADS_INIT)
  IF(CMAKE_SYSTEM MATCHES "HP-UX-*")
    # Use libcma if it exists and can be used.  It provides more
    # symbols than the plain pthread library.  CMA threads
    # have actually been deprecated:
    #   http://docs.hp.com/en/B3920-90091/ch12s03.html#d0e11395
    #   http://docs.hp.com/en/947/d8.html
    # but we need to maintain compatibility here.
    # The CMAKE_HP_PTHREADS setting actually indicates whether CMA threads
    # are available.
    CHECK_LIBRARY_EXISTS(cma pthread_attr_create "" CMAKE_HAVE_HP_CMA)
    IF(CMAKE_HAVE_HP_CMA)
      SET(CMAKE_THREAD_LIBS_INIT "-lcma")
      SET(CMAKE_HP_PTHREADS_INIT 1)
    ENDIF(CMAKE_HAVE_HP_CMA)
    SET(CMAKE_USE_PTHREADS_INIT 1)
  ENDIF(CMAKE_SYSTEM MATCHES "HP-UX-*")

  IF(CMAKE_SYSTEM MATCHES "OSF1-V*")
    SET(CMAKE_USE_PTHREADS_INIT 0)
    SET(CMAKE_THREAD_LIBS_INIT )
  ENDIF(CMAKE_SYSTEM MATCHES "OSF1-V*")

  IF(CMAKE_SYSTEM MATCHES "CYGWIN_NT*")
    SET(CMAKE_USE_PTHREADS_INIT 1)
    SET(CMAKE_THREAD_LIBS_INIT )
    SET(CMAKE_USE_WIN32_THREADS_INIT 0)
  ENDIF(CMAKE_SYSTEM MATCHES "CYGWIN_NT*")
ENDIF(CMAKE_USE_PTHREADS_INIT)

