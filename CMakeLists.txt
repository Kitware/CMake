PROJECT(CMake)

SUBDIRS(Source)

# Include the standard Dart testing module
INCLUDE (${CMAKE_ROOT}/Modules/Dart.cmake)

#
# Variables used for submitting testing results. These will change from
# project to project. Most likely, you will change DROP_SITE, 
# DROP_SITE_PASSWORD and TRIGGER_SITE.
# 
IF (BUILD_TESTING)
  ENABLE_TESTING()

  # Dart server to submit results (used by client)
  SET (DROP_SITE "public.kitware.com" CACHE INTERNAL 
       "FTP site for submitting testing results")
  SET (DROP_LOCATION "/incoming" CACHE INTERNAL 
       "Directory on FTP site to place testing results")
  SET (DROP_SITE_USER "anonymous" CACHE INTERNAL 
       "Username for FTP site for submitting testing results")
  SET (DROP_SITE_PASSWORD "cmake-tester@somewhere.com" CACHE INTERNAL 
       "Password for FTP site for submitting testing results")
  SET (TRIGGER_SITE 
       "http://${DROP_SITE}/cgi-bin/Submit-CMake-TestingResults.pl" 
       CACHE INTERNAL 
       "Web site to signal when testing results have been submitted")

  # Dart server configuration 
  SET (CVS_WEB_URL "http://${DROP_SITE}/cgi-bin/cmakecvsweb.cgi/CMake/" 
       CACHE INTERNAL "URL for revision control system")
  SET (CVS_WEB_CVSROOT "CMake" CACHE INTERNAL 
       "Symbolic name for the CVSROOT in cvsweb")
  SET (DOXYGEN_URL "http://${DROP_SITE}/CMake/Doxygen/html/" 
       CACHE INTERNAL "URL for source code documentation")
  SET (GNATS_WEB_URL "http://${DROP_SITE}/cgi-bin/gnatsweb.pl/CMake/" 
       CACHE INTERNAL "URL for bug tracker")

ENDIF (BUILD_TESTING)


