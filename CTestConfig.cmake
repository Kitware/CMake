set(CTEST_PROJECT_NAME "CMake")
set(CTEST_NIGHTLY_START_TIME "21:00:00 EDT")

set(CTEST_DROP_METHOD "http")
set(CTEST_DROP_SITE "www.cdash.org")
set(CTEST_DROP_LOCATION "/CDash/submit.php?project=CMake")
set(CTEST_DROP_SITE_CDASH TRUE)

# use old trigger stuff so that cmake 2.4 and below will not 
# get errors on trigger
SET (TRIGGER_SITE 
  "http://public.kitware.com/cgi-bin/Submit-CMake-TestingResults.cgi")
