# Dart server to submit results (used by client)
SET (DROP_SITE "public.kitware.com")
SET (DROP_LOCATION "/incoming")
SET (DROP_SITE_USER "anonymous")
SET (DROP_SITE_PASSWORD "cmake-tester@somewhere.com")
SET (TRIGGER_SITE 
       "http://${DROP_SITE}/cgi-bin/Submit-CMake-TestingResults.pl")

# Dart server configuration 
SET (CVS_WEB_URL "http://${DROP_SITE}/cgi-bin/cvsweb.cgi/CMake/")
SET (CVS_WEB_CVSROOT "CMake")

OPTION(BUILD_DOXYGEN "Build source documentation using doxygen" "Off")
SET (DOXYGEN_URL "http://${DROP_SITE}/CMake/Doxygen/html/" )
SET (DOXYGEN_CONFIG "${PROJECT_BINARY_DIR}/doxygen.config" )

SET (USE_GNATS "Off")
SET (GNATS_WEB_URL "http://${DROP_SITE}/cgi-bin/gnatsweb.pl/CMake/")
