#include "cmTestConfigure.h"
#include "ExtraSources/file1.h"
#include "file2.h"
#include "sharedFile.h"
#include "cmStandardIncludes.h"
#include "cmSystemTools.h"

int cm_passed = 0;
int cm_failed = 0;

// Here is a stupid function that tries to use std::string methods
// so that the dec cxx compiler will instantiate the stuff that
// we are using from the CMakeLib library....

void ForceStringUse()
{
  std::vector<std::string> v;
  std::vector<std::string> v2;
  v = v2;
  std::string cachetest = CACHE_TEST_VAR_INTERNAL;
  v.push_back(cachetest);
  v2 = v;
  std::string x(5,'x');  
  char buff[5];
  x.copy(buff, 1, 0);
  std::string::size_type pos = 0;
  x.replace(pos, pos, pos, 'x');
  std::string copy = cachetest;
  cachetest.find("bar");
  cachetest.rfind("bar");
  copy.append(cachetest);
  copy = cachetest.substr(0, cachetest.size());
}

// ======================================================================

void cmFailed(const char* Message, const char* m2= "")
{
  std::cerr << "Failed: " << Message << m2 << "\n"; 
  cm_failed++;
}

// ======================================================================

void cmPassed(const char* Message, const char* m2="")
{
  std::cout << "Passed: " << Message << m2 << "\n"; 
  cm_passed++;
}

// ======================================================================

void TestAndRemoveFile(const char* filename) 
{
  if (!cmSystemTools::FileExists(filename))
    {
    cmFailed("Could not find file: ", filename);
    }
  else
    {
    if (!cmSystemTools::RemoveFile(filename))
      {
      cmFailed("Unable to remove file. It does not imply that this test failed, but it *will* be corrupted thereafter if this file is not removed: ", filename);
      }
    else
      {
      cmPassed("Find and remove file: ", filename);
      }
    }
}

// ======================================================================

void TestDir(const char* filename) 
{
  if (!cmSystemTools::FileExists(filename))
    {
    cmFailed("Could not find dir: ", filename);
    }
  else
    {
    if (!cmSystemTools::FileIsDirectory(filename))
      {
      cmFailed("Unable to check if file is a directory: ", filename);
      }
    else
      {
      cmPassed("Find dir: ", filename);
      }
    }
}

// ======================================================================

int main()
{
  if(sharedFunction() != 1)
    {
    cmFailed("Call to sharedFunction from shared library failed.");
    }
  else
    {
    cmPassed("Call to sharedFunction from shared library worked.");
    }
  
  if(file1() != 1)
    {
    cmFailed("Call to file1 function from library failed.");
    }
  else
    {
    cmPassed("Call to file1 function returned 1.");
    }

  if(file2() != 1)
    {
    cmFailed("Call to file2 function from library failed.");
    }
  else
    {
    cmPassed("Call to file2 function returned 1.");
    }

  // ----------------------------------------------------------------------
  // Test ADD_DEFINITIONS

#ifndef CMAKE_IS_FUN
  cmFailed("CMake is not fun, so it is broken and should be fixed.");
#else
  cmPassed("CMAKE_IS_FUN is defined.");
#endif
  
  // ----------------------------------------------------------------------
  // Test SET, VARIABLE_REQUIRES

#ifdef SHOULD_NOT_BE_DEFINED
  cmFailed("IF or SET is broken, SHOULD_NOT_BE_DEFINED is defined.");
#else
  cmPassed("SHOULD_NOT_BE_DEFINED is not defined.");
#endif
  
#ifndef SHOULD_BE_DEFINED
  cmFailed("IF or SET is broken, SHOULD_BE_DEFINED is not defined.\n");
#else
  cmPassed("SHOULD_BE_DEFINED is defined.");
#endif
  
#ifndef ONE_VAR
  cmFailed("cmakedefine is broken, ONE_VAR is not defined.");
#else
  cmPassed("ONE_VAR is defined.");
#endif
  
#ifndef ONE_VAR_IS_DEFINED
  cmFailed("cmakedefine, SET or VARIABLE_REQUIRES is broken, "
         "ONE_VAR_IS_DEFINED is not defined.");
#else
  cmPassed("ONE_VAR_IS_DEFINED is defined.");
#endif
  
#ifdef ZERO_VAR
  cmFailed("cmakedefine is broken, ZERO_VAR is defined.");
#else
  cmPassed("ZERO_VAR is not defined.");
#endif
  
#ifndef STRING_VAR
  cmFailed("the CONFIGURE_FILE command is broken, STRING_VAR is not defined.");
#else
  if(strcmp(STRING_VAR, "CMake is great") != 0)
    {
    cmFailed("the SET or CONFIGURE_FILE command is broken. STRING_VAR == ", 
           STRING_VAR);
    }
  else
    {
    cmPassed("STRING_VAR == ", STRING_VAR);
    }
#endif

  // ----------------------------------------------------------------------
  // Test various IF/ELSE combinations

#ifdef SHOULD_NOT_BE_DEFINED_NOT
  cmFailed("IF or SET is broken, SHOULD_NOT_BE_DEFINED_NOT is defined.");
#else
  cmPassed("SHOULD_NOT_BE_DEFINED_NOT is not defined.");
#endif
  
#ifndef SHOULD_BE_DEFINED_NOT
  cmFailed("IF or SET is broken, SHOULD_BE_DEFINED_NOT is not defined.\n");
#else
  cmPassed("SHOULD_BE_DEFINED_NOT is defined.");
#endif
  
#ifdef SHOULD_NOT_BE_DEFINED_NOT2
  cmFailed("IF or SET is broken, SHOULD_NOT_BE_DEFINED_NOT2 is defined.");
#else
  cmPassed("SHOULD_NOT_BE_DEFINED_NOT2 is not defined.");
#endif
  
#ifndef SHOULD_BE_DEFINED_NOT2
  cmFailed("IF or SET is broken, SHOULD_BE_DEFINED_NOT2 is not defined.\n");
#else
  cmPassed("SHOULD_BE_DEFINED_NOT2 is defined.");
#endif
  
#ifdef SHOULD_NOT_BE_DEFINED_AND
  cmFailed("IF or SET is broken, SHOULD_NOT_BE_DEFINED_AND is defined.");
#else
  cmPassed("SHOULD_NOT_BE_DEFINED_AND is not defined.");
#endif
  
#ifndef SHOULD_BE_DEFINED_AND
  cmFailed("IF or SET is broken, SHOULD_BE_DEFINED_AND is not defined.\n");
#else
  cmPassed("SHOULD_BE_DEFINED_AND is defined.");
#endif
  
#ifdef SHOULD_NOT_BE_DEFINED_AND2
  cmFailed("IF or SET is broken, SHOULD_NOT_BE_DEFINED_AND2 is defined.");
#else
  cmPassed("SHOULD_NOT_BE_DEFINED_AND2 is not defined.");
#endif
  
#ifndef SHOULD_BE_DEFINED_AND2
  cmFailed("IF or SET is broken, SHOULD_BE_DEFINED_AND2 is not defined.\n");
#else
  cmPassed("SHOULD_BE_DEFINED_AND2 is defined.");
#endif
  
#ifdef SHOULD_NOT_BE_DEFINED_OR
  cmFailed("IF or SET is broken, SHOULD_NOT_BE_DEFINED_OR is defined.");
#else
  cmPassed("SHOULD_NOT_BE_DEFINED_OR is not defined.");
#endif
  
#ifndef SHOULD_BE_DEFINED_OR
  cmFailed("IF or SET is broken, SHOULD_BE_DEFINED_OR is not defined.\n");
#else
  cmPassed("SHOULD_BE_DEFINED_OR is defined.");
#endif
  
#ifdef SHOULD_NOT_BE_DEFINED_OR2
  cmFailed("IF or SET is broken, SHOULD_NOT_BE_DEFINED_OR2 is defined.");
#else
  cmPassed("SHOULD_NOT_BE_DEFINED_OR2 is not defined.");
#endif
  
#ifndef SHOULD_BE_DEFINED_OR2
  cmFailed("IF or SET is broken, SHOULD_BE_DEFINED_OR2 is not defined.\n");
#else
  cmPassed("SHOULD_BE_DEFINED_OR2 is defined.");
#endif
  
#ifdef SHOULD_NOT_BE_DEFINED_MATCHES
  cmFailed("IF or SET is broken, SHOULD_NOT_BE_DEFINED_MATCHES is defined.");
#else
  cmPassed("SHOULD_NOT_BE_DEFINED_MATCHES is not defined.");
#endif
  
#ifndef SHOULD_BE_DEFINED_MATCHES
  cmFailed("IF or SET is broken, SHOULD_BE_DEFINED_MATCHES is not defined.\n");
#else
  cmPassed("SHOULD_BE_DEFINED_MATCHES is defined.");
#endif
  
#ifdef SHOULD_NOT_BE_DEFINED_MATCHES2
  cmFailed("IF or SET is broken, SHOULD_NOT_BE_DEFINED_MATCHES2 is defined.");
#else
  cmPassed("SHOULD_NOT_BE_DEFINED_MATCHES2 is not defined.");
#endif
  
#ifndef SHOULD_BE_DEFINED_MATCHES2
  cmFailed("IF or SET is broken, SHOULD_BE_DEFINED_MATCHES2 is not defined.\n");
#else
  cmPassed("SHOULD_BE_DEFINED_MATCHES2 is defined.");
#endif
  
#ifdef SHOULD_NOT_BE_DEFINED_COMMAND
  cmFailed("IF or SET is broken, SHOULD_NOT_BE_DEFINED_COMMAND is defined.");
#else
  cmPassed("SHOULD_NOT_BE_DEFINED_COMMAND is not defined.");
#endif
  
#ifndef SHOULD_BE_DEFINED_COMMAND
  cmFailed("IF or SET is broken, SHOULD_BE_DEFINED_COMMAND is not defined.\n");
#else
  cmPassed("SHOULD_BE_DEFINED_COMMAND is defined.");
#endif
  
#ifdef SHOULD_NOT_BE_DEFINED_COMMAND2
  cmFailed("IF or SET is broken, SHOULD_NOT_BE_DEFINED_COMMAND2 is defined.");
#else
  cmPassed("SHOULD_NOT_BE_DEFINED_COMMAND2 is not defined.");
#endif
  
#ifndef SHOULD_BE_DEFINED_COMMAND2
  cmFailed("IF or SET is broken, SHOULD_BE_DEFINED_COMMAND2 is not defined.\n");
#else
  cmPassed("SHOULD_BE_DEFINED_COMMAND2 is defined.");
#endif
  
#ifdef SHOULD_NOT_BE_DEFINED_EXISTS
  cmFailed("IF or SET is broken, SHOULD_NOT_BE_DEFINED_EXISTS is defined.");
#else
  cmPassed("SHOULD_NOT_BE_DEFINED_EXISTS is not defined.");
#endif
  
#ifndef SHOULD_BE_DEFINED_EXISTS
  cmFailed("IF or SET is broken, SHOULD_BE_DEFINED_EXISTS is not defined.\n");
#else
  cmPassed("SHOULD_BE_DEFINED_EXISTS is defined.");
#endif
  
#ifdef SHOULD_NOT_BE_DEFINED_EXISTS2
  cmFailed("IF or SET is broken, SHOULD_NOT_BE_DEFINED_EXISTS2 is defined.");
#else
  cmPassed("SHOULD_NOT_BE_DEFINED_EXISTS2 is not defined.");
#endif
  
#ifndef SHOULD_BE_DEFINED_EXISTS2
  cmFailed("IF or SET is broken, SHOULD_BE_DEFINED_EXISTS2 is not defined.\n");
#else
  cmPassed("SHOULD_BE_DEFINED_EXISTS2 is defined.");
#endif
  
  // ----------------------------------------------------------------------
  // Test FOREACH

#ifndef FOREACH_VAR1
  cmFailed("the FOREACH, SET or CONFIGURE_FILE command is broken, "
         "FOREACH_VAR1 is not defined.");
#else
  if(strcmp(FOREACH_VAR1, "VALUE1") != 0)
    {
    cmFailed("the FOREACH, SET or CONFIGURE_FILE command is broken, "
           "FOREACH_VAR1 == ", FOREACH_VAR1);
    }
  else
    {
    cmPassed("FOREACH_VAR1 == ", FOREACH_VAR1);
    }
#endif

#ifndef FOREACH_VAR2
  cmFailed("the FOREACH, SET or CONFIGURE_FILE command is broken, "
         "FOREACH_VAR2 is not defined.");
#else
  if(strcmp(FOREACH_VAR2, "VALUE2") != 0)
    {
    cmFailed("the FOREACH, SET or CONFIGURE_FILE command is broken, "
           "FOREACH_VAR2 == ", FOREACH_VAR2);
    }
  else
    {
    cmPassed("FOREACH_VAR2 == ", FOREACH_VAR2);
    }
#endif

  // ----------------------------------------------------------------------
  // Test FIND_FILE, FIND_PATH and various GET_FILENAME_COMPONENT combinations

#ifndef FILENAME_VAR_PATH_NAME
  cmFailed("the FIND_FILE or GET_FILENAME_COMPONENT command is broken, "
         "FILENAME_VAR_PATH_NAME is not defined.");
#else
  if(strcmp(FILENAME_VAR_PATH_NAME, "Complex") != 0)
    {
    cmFailed("the FIND_FILE or GET_FILENAME_COMPONENT command is broken, "
           "FILENAME_VAR_PATH_NAME == ", FILENAME_VAR_PATH_NAME);
    }
  else
    {
    cmPassed("FILENAME_VAR_PATH_NAME == ", FILENAME_VAR_PATH_NAME);
    }
#endif

#ifndef FILENAME_VAR_NAME
  cmFailed("the FIND_FILE or GET_FILENAME_COMPONENT command is broken, "
         "FILENAME_VAR_NAME is not defined.");
#else
  if(strcmp(FILENAME_VAR_NAME, "VarTests.cmake") != 0)
    {
    cmFailed("the FIND_FILE or GET_FILENAME_COMPONENT command is broken, "
           "FILENAME_VAR_NAME == ", FILENAME_VAR_NAME);
    }
  else
    {
    cmPassed("FILENAME_VAR_NAME == ", FILENAME_VAR_NAME);
    }
#endif

#ifndef FILENAME_VAR_EXT
  cmFailed("the FIND_FILE or GET_FILENAME_COMPONENT command is broken, "
         "FILENAME_VAR_EXT is not defined.");
#else
  if(strcmp(FILENAME_VAR_EXT, ".cmake") != 0)
    {
    cmFailed("the FIND_FILE or GET_FILENAME_COMPONENT command is broken, "
           "FILENAME_VAR_EXT == ", FILENAME_VAR_EXT);
    }
  else
    {
    cmPassed("FILENAME_VAR_EXT == ", FILENAME_VAR_EXT);
    }
#endif

#ifndef FILENAME_VAR_NAME_WE
  cmFailed("the FIND_FILE or GET_FILENAME_COMPONENT command is broken, "
         "FILENAME_VAR_NAME_WE is not defined.");
#else
  if(strcmp(FILENAME_VAR_NAME_WE, "VarTests") != 0)
    {
    cmFailed("the FIND_FILE or GET_FILENAME_COMPONENT command is broken, "
           "FILENAME_VAR_NAME_WE == ", FILENAME_VAR_NAME_WE);
    }
  else
    {
    cmPassed("FILENAME_VAR_NAME_WE == ", FILENAME_VAR_NAME_WE);
    }
#endif

#ifndef PATH_VAR_NAME
  cmFailed("the FIND_FILE or GET_FILENAME_COMPONENT command is broken, "
         "PATH_VAR_NAME is not defined.");
#else
  if(strcmp(PATH_VAR_NAME, "Complex") != 0)
    {
    cmFailed("the FIND_FILE or GET_FILENAME_COMPONENT command is broken, "
           "PATH_VAR_NAME == ", PATH_VAR_NAME);
    }
  else
    {
    cmPassed("PATH_VAR_NAME == ", PATH_VAR_NAME);
    }
#endif

  // ----------------------------------------------------------------------
  // Test LOAD_CACHE

#ifndef CACHE_TEST_VAR1
  cmFailed("the LOAD_CACHE or CONFIGURE_FILE command is broken, "
         "CACHE_TEST_VAR1 is not defined.");
#else
  if(strcmp(CACHE_TEST_VAR1, "foo") != 0)
    {
    cmFailed("the LOAD_CACHE or CONFIGURE_FILE command is broken, "
           "CACHE_TEST_VAR1 == ", CACHE_TEST_VAR1);
    }
  else
    {
    cmPassed("CACHE_TEST_VAR1 == ", CACHE_TEST_VAR1);
    }
#endif

#ifndef CACHE_TEST_VAR2
  cmFailed("the LOAD_CACHE or CONFIGURE_FILE command is broken, "
         "CACHE_TEST_VAR2 is not defined.");
#else
  if(strcmp(CACHE_TEST_VAR2, "bar") != 0)
    {
    cmFailed("the LOAD_CACHE or CONFIGURE_FILE command is broken, "
           "CACHE_TEST_VAR2 == ", CACHE_TEST_VAR2);
    }
  else
    {
    cmPassed("CACHE_TEST_VAR2 == ", CACHE_TEST_VAR2);
    }
#endif

#ifndef CACHE_TEST_VAR3
  cmFailed("the LOAD_CACHE or CONFIGURE_FILE command is broken, "
         "CACHE_TEST_VAR3 is not defined.");
#else
  if(strcmp(CACHE_TEST_VAR3, "1") != 0)
    {
    cmFailed("the LOAD_CACHE or CONFIGURE_FILE command is broken, "
           "CACHE_TEST_VAR3 == ", CACHE_TEST_VAR3);
    }
  else
    {
    cmPassed("CACHE_TEST_VAR3 == ", CACHE_TEST_VAR3);
    }
#endif

#ifdef CACHE_TEST_VAR_EXCLUDED
  cmFailed("the LOAD_CACHE or CONFIGURE_FILE command or cmakedefine is broken, "
         "CACHE_TEST_VAR_EXCLUDED is defined (should not have been loaded).");
#else
  cmPassed("CACHE_TEST_VAR_EXCLUDED is not defined.");
#endif

#ifndef CACHE_TEST_VAR_INTERNAL
  cmFailed("the LOAD_CACHE or CONFIGURE_FILE command is broken, "
         "CACHE_TEST_VAR_INTERNAL is not defined.");
#else
  std::string cachetest = CACHE_TEST_VAR_INTERNAL;
  if(cachetest != "bar")
    {
    cmFailed("the LOAD_CACHE or CONFIGURE_FILE command is broken, "
           "CACHE_TEST_VAR_INTERNAL == ", CACHE_TEST_VAR_INTERNAL);
    }
  else
    {
    cmPassed("CACHE_TEST_VAR_INTERNAL == ", CACHE_TEST_VAR_INTERNAL);
    }
#endif

  // ----------------------------------------------------------------------
  // A post-build custom-command has been attached to the lib (see Library/).
  // It runs ${CREATE_FILE_EXE} which will create a file.
  //
  // WARNING: if you run 'complex' manually, this *will* fail, because
  // the file was removed the last time 'complex' was run, and it is
  // only created during a build.

  TestAndRemoveFile(BINARY_DIR "/Library/postbuild.txt");

  // ----------------------------------------------------------------------
  // A custom target has been created (see Library/).
  // It runs ${CREATE_FILE_EXE} which will create a file.
  //
  // WARNING: if you run 'complex' manually, this *will* fail, because
  // the file was removed the last time 'complex' was run, and it is
  // only created during a build.

  TestAndRemoveFile(BINARY_DIR "/Library/custom_target1.txt");

  // ----------------------------------------------------------------------
  // A directory has been created.

  TestDir(BINARY_DIR "/make_dir");

  // ----------------------------------------------------------------------
  // Test OUTPUT_REQUIRED_FILES
  // The files required by 'complex' have been output to a file.
  // The contents of this file is not tested (absolute paths).
  //
  // WARNING: if you run 'complex' manually, this *will* fail, because
  // the file was removed the last time 'complex' was run, and it is
  // only created during a build.

  TestAndRemoveFile(BINARY_DIR "/Executable/Temp/complex-required.txt");

  // ----------------------------------------------------------------------
  // Summary

  std::cout << "Passed: " << cm_passed << "\n";
  if(cm_failed)
    {
    std::cout << "Failed: " << cm_failed << "\n";
    return cm_failed;
    }

  return 0;
}
