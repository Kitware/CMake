# Dashboard is opened for submissions for a 24 hour period starting at
# the specified NIGHLY_START_TIME. Time is specified in 24 hour format.
SET (NIGHTLY_START_TIME "21:00:00 EDT")

# Dart server to submit results (used by client)
IF(NOT DROP_METHOD)
  SET(DROP_METHOD http)
ENDIF(NOT DROP_METHOD)
IF(DROP_METHOD MATCHES http)
  SET (DROP_SITE "public.kitware.com")
  SET (DROP_LOCATION "/cgi-bin/HTTPUploadDartFile.cgi")
ELSE(DROP_METHOD MATCHES http)
  IF(DROP_METHOD MATCHES xmlrpc)
    SET (DROP_SITE "http://www.na-mic.org:8081")
    SET (DROP_LOCATION "CMake")
    SET (COMPRESS_SUBMISSION ON)
  ELSE(DROP_METHOD MATCHES xmlrpc)
    SET (DROP_SITE "public.kitware.com")
    SET (DROP_LOCATION "/incoming")
    SET (DROP_SITE_USER "ftpuser")
    SET (DROP_SITE_PASSWORD "public")
  ENDIF(DROP_METHOD MATCHES xmlrpc)
ENDIF(DROP_METHOD MATCHES http)

SET (TRIGGER_SITE 
  "http://${DROP_SITE}/cgi-bin/Submit-CMake-TestingResults.cgi")

# Project Home Page
SET (PROJECT_URL "http://www.cmake.org")

# Dart server configuration 
SET (ROLLUP_URL "http://${DROP_SITE}/cgi-bin/cmake-rollup-dashboard.sh")
SET (CVS_WEB_URL "http://${DROP_SITE}/cgi-bin/viewcvs.cgi/")
SET (CVS_WEB_CVSROOT "CMake")

OPTION(BUILD_DOXYGEN "Build source documentation using doxygen" "Off")
SET (DOXYGEN_CONFIG "${PROJECT_BINARY_DIR}/doxygen.config" )
MARK_AS_ADVANCED(BUILD_DOXYGEN)
SET (USE_DOXYGEN "On")
SET (DOXYGEN_URL "${PROJECT_URL}/doc/nightly/html/" )

SET (USE_GNATS "On")
SET (GNATS_WEB_URL "${PROJECT_URL}/Bug/query.php?projects=2&status%5B%5D=1&status%5B%5D=2&status%5B%5D=3&status%5B%5D=4&status%5B%5D=6&op=doquery")

# Continuous email delivery variables
SET (CONTINUOUS_FROM "cmake-dashboard@public.kitware.com")
SET (SMTP_MAILHOST "public.kitware.com")
SET (CONTINUOUS_MONITOR_LIST "cmake-dashboard@public.kitware.com")
SET (CONTINUOUS_BASE_URL "${PROJECT_URL}/Testing")

SET (DELIVER_BROKEN_BUILD_EMAIL_WITH_TEST_FAILURES ON)
SET (DELIVER_BROKEN_BUILD_EMAIL "Continuous Nightly")
SET (EMAIL_FROM "cmake-dashboard@public.kitware.com")
SET (DARTBOARD_BASE_URL "${PROJECT_URL}/Testing")

SET (DELIVER_BROKEN_BUILD_EMAIL_WITH_CONFIGURE_FAILURES 1)
SET (DELIVER_BROKEN_BUILD_EMAIL_WITH_BUILD_ERRORS 1)
SET (DELIVER_BROKEN_BUILD_EMAIL_WITH_BUILD_WARNINGS 1)
SET (DELIVER_BROKEN_BUILD_EMAIL_WITH_TEST_NOT_RUNS 1)
SET (DELIVER_BROKEN_BUILD_EMAIL_WITH_TEST_FAILURES 1)

