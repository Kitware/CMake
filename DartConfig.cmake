# Dashboard is opened for submissions for a 24 hour period starting at
# the specified NIGHLY_START_TIME. Time is specified in 24 hour format.
SET (NIGHTLY_START_TIME "22:00:00 EDT")

# Dart server to submit results (used by client)
SET (DROP_SITE "public.kitware.com")
SET (DROP_LOCATION "/incoming")
SET (DROP_SITE_USER "ftpuser")
SET (DROP_SITE_PASSWORD "public")
SET (TRIGGER_SITE 
       "http://${DROP_SITE}/cgi-bin/Submit-CMake-TestingResults.pl")

# Project Home Page
SET (PROJECT_URL "http://www.cmake.org/")

# Dart server configuration 
SET (ROLLUP_URL "http://${DROP_SITE}/cgi-bin/cmake-rollup-dashboard.sh")

SET (CVS_WEB_URL "http://${DROP_SITE}/cgi-bin/cvsweb.cgi/CMake/")
SET (CVS_WEB_CVSROOT "CMake")

SET (USE_DOXYGEN "On")
SET (DOXYGEN_URL "http://www.cmake.org/doc/nightly/html/" )
OPTION(BUILD_DOXYGEN "Build source documentation using doxygen" "Off")
SET (DOXYGEN_CONFIG "${PROJECT_BINARY_DIR}/doxygen.config" )

SET (USE_GNATS "Off")
SET (GNATS_WEB_URL "http://${DROP_SITE}/cgi-bin/gnatsweb.pl/CMake/")
