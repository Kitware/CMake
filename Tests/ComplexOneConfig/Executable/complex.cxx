#include "cmTestConfigure.h"
#include "ExtraSources/file1.h"
#include "file2.h"
#include "sharedFile.h"
#include "cmStandardIncludes.h"
#include "cmSystemTools.h"

int passed = 0;
int failed = 0;

void Failed(const char* Message, const char* m2= "")
{
  std::cerr << "Failed: " << Message << m2 << "\n"; 
  failed++;
}

void Passed(const char* Message, const char* m2="")
{
  std::cout << "Passed: " << Message << m2 << "\n"; 
  passed++;
}

void TestAndRemoveFile(const char* filename) 
{
  if (!cmSystemTools::FileExists(filename))
    {
    Failed("Could not find file: ", filename);
    }
  else
    {
    if (!cmSystemTools::RemoveFile(filename))
      {
      Failed("Unable to remove file. It does not imply that this test failed, but it *will* be corrupted thereafter if this file is not removed: ", filename);
      }
    else
      {
      Passed("Find and remove file: ", filename);
      }
    }
}

void TestDir(const char* filename) 
{
  if (!cmSystemTools::FileExists(filename))
    {
    Failed("Could not find dir: ", filename);
    }
  else
    {
    if (!cmSystemTools::FileIsDirectory(filename))
      {
      Failed("Unable to check if file is a directory: ", filename);
      }
    else
      {
      Passed("Find dir: ", filename);
      }
    }
}


int main()
{
  if(sharedFunction() != 1)
    {
    Failed("Call to sharedFunction from shared library failed.");
    }
  else
    {
    Passed("Call to sharedFunction from shared library worked.");
    }
  
  if(file1() != 1)
    {
    Failed("Call to file1 function from library failed.");
    }
  else
    {
    Passed("Call to file1 function returned 1.");
    }

  if(file2() != 1)
    {
    Failed("Call to file2 function from library failed.");
    }
  else
    {
    Passed("Call to file2 function returned 1.");
    }

#ifndef CMAKE_IS_FUN
  Failed("CMake is not fun, so it is broken and should be fixed.");
#else
  Passed("CMAKE_IS_FUN is defined.");
#endif
  
#ifdef SHOULD_NOT_BE_DEFINED
  Failed("IF or SET is broken, SHOULD_NOT_BE_DEFINED is defined.");
#else
  Passed("SHOULD_NOT_BE_DEFINED is not defined.");
#endif
  
#ifndef SHOULD_BE_DEFINED
  Failed("IF or SET is broken, SHOULD_BE_DEFINED is not defined.\n");
#else
  Passed("SHOULD_BE_DEFINED is defined.");
#endif
  
#ifdef SHOULD_NOT_BE_DEFINED_AND
  Failed("IF or SET is broken, SHOULD_NOT_BE_DEFINED_AND is defined.");
#else
  Passed("SHOULD_NOT_BE_DEFINED_AND is not defined.");
#endif
  
#ifndef SHOULD_BE_DEFINED_AND
  Failed("IF or SET is broken, SHOULD_BE_DEFINED_AND is not defined.\n");
#else
  Passed("SHOULD_BE_DEFINED_AND is defined.");
#endif
  
#ifdef SHOULD_NOT_BE_DEFINED_OR
  Failed("IF or SET is broken, SHOULD_NOT_BE_DEFINED_OR is defined.");
#else
  Passed("SHOULD_NOT_BE_DEFINED_OR is not defined.");
#endif
  
#ifndef SHOULD_BE_DEFINED_OR
  Failed("IF or SET is broken, SHOULD_BE_DEFINED_OR is not defined.\n");
#else
  Passed("SHOULD_BE_DEFINED_OR is defined.");
#endif
  
#ifdef SHOULD_NOT_BE_DEFINED_MATCHES
  Failed("IF or SET is broken, SHOULD_NOT_BE_DEFINED_MATCHES is defined.");
#else
  Passed("SHOULD_NOT_BE_DEFINED_MATCHES is not defined.");
#endif
  
#ifndef SHOULD_BE_DEFINED_MATCHES
  Failed("IF or SET is broken, SHOULD_BE_DEFINED_MATCHES is not defined.\n");
#else
  Passed("SHOULD_BE_DEFINED_MATCHES is defined.");
#endif
  
#ifndef ONE_VAR
  Failed("cmakedefine is broken, ONE_VAR is not defined.");
#else
  Passed("ONE_VAR is defined.");
#endif
  
#ifndef ONE_VAR_IS_DEFINED
  Failed("cmakedefine, SET or VARIABLE_REQUIRES is broken, "
         "ONE_VAR_IS_DEFINED is not defined.");
#else
  Passed("ONE_VAR_IS_DEFINED is defined.");
#endif
  
#ifdef ZERO_VAR
  Failed("cmakedefine is broken, ZERO_VAR is defined.");
#else
  Passed("ZERO_VAR is not defined.");
#endif
  
#ifndef STRING_VAR
  Failed("the CONFIGURE_FILE command is broken, STRING_VAR is not defined.");
#else
  if(strcmp(STRING_VAR, "CMake is great") != 0)
    {
    Failed("the SET or CONFIGURE_FILE command is broken. STRING_VAR == ", 
           STRING_VAR);
    }
  else
    {
    Passed("STRING_VAR == ", STRING_VAR);
    }
#endif

#ifndef FOREACH_VAR1
  Failed("the FOREACH, SET or CONFIGURE_FILE command is broken, "
         "FOREACH_VAR1 is not defined.");
#else
  if(strcmp(FOREACH_VAR1, "VALUE1") != 0)
    {
    Failed("the FOREACH, SET or CONFIGURE_FILE command is broken, "
           "FOREACH_VAR1 == ", FOREACH_VAR1);
    }
  else
    {
    Passed("FOREACH_VAR1 == ", FOREACH_VAR1);
    }
#endif

#ifndef FOREACH_VAR2
  Failed("the FOREACH, SET or CONFIGURE_FILE command is broken, "
         "FOREACH_VAR2 is not defined.");
#else
  if(strcmp(FOREACH_VAR2, "VALUE2") != 0)
    {
    Failed("the FOREACH, SET or CONFIGURE_FILE command is broken, "
           "FOREACH_VAR2 == ", FOREACH_VAR2);
    }
  else
    {
    Passed("FOREACH_VAR2 == ", FOREACH_VAR2);
    }
#endif

#ifndef FILENAME_VAR_PATH_NAME
  Failed("the FIND_FILE or GET_FILENAME_COMPONENT command is broken, "
         "FILENAME_VAR_PATH_NAME is not defined.");
#else
  if(strcmp(FILENAME_VAR_PATH_NAME, "Complex") != 0)
    {
    Failed("the FIND_FILE or GET_FILENAME_COMPONENT command is broken, "
           "FILENAME_VAR_PATH_NAME == ", FILENAME_VAR_PATH_NAME);
    }
  else
    {
    Passed("FILENAME_VAR_PATH_NAME == ", FILENAME_VAR_PATH_NAME);
    }
#endif

#ifndef FILENAME_VAR_NAME
  Failed("the FIND_FILE or GET_FILENAME_COMPONENT command is broken, "
         "FILENAME_VAR_NAME is not defined.");
#else
  if(strcmp(FILENAME_VAR_NAME, "VarTests.cmake") != 0)
    {
    Failed("the FIND_FILE or GET_FILENAME_COMPONENT command is broken, "
           "FILENAME_VAR_NAME == ", FILENAME_VAR_NAME);
    }
  else
    {
    Passed("FILENAME_VAR_NAME == ", FILENAME_VAR_NAME);
    }
#endif

#ifndef FILENAME_VAR_EXT
  Failed("the FIND_FILE or GET_FILENAME_COMPONENT command is broken, "
         "FILENAME_VAR_EXT is not defined.");
#else
  if(strcmp(FILENAME_VAR_EXT, ".cmake") != 0)
    {
    Failed("the FIND_FILE or GET_FILENAME_COMPONENT command is broken, "
           "FILENAME_VAR_EXT == ", FILENAME_VAR_EXT);
    }
  else
    {
    Passed("FILENAME_VAR_EXT == ", FILENAME_VAR_EXT);
    }
#endif

#ifndef FILENAME_VAR_NAME_WE
  Failed("the FIND_FILE or GET_FILENAME_COMPONENT command is broken, "
         "FILENAME_VAR_NAME_WE is not defined.");
#else
  if(strcmp(FILENAME_VAR_NAME_WE, "VarTests") != 0)
    {
    Failed("the FIND_FILE or GET_FILENAME_COMPONENT command is broken, "
           "FILENAME_VAR_NAME_WE == ", FILENAME_VAR_NAME_WE);
    }
  else
    {
    Passed("FILENAME_VAR_NAME_WE == ", FILENAME_VAR_NAME_WE);
    }
#endif

#ifndef PATH_VAR_NAME
  Failed("the FIND_FILE or GET_FILENAME_COMPONENT command is broken, "
         "PATH_VAR_NAME is not defined.");
#else
  if(strcmp(PATH_VAR_NAME, "Complex") != 0)
    {
    Failed("the FIND_FILE or GET_FILENAME_COMPONENT command is broken, "
           "PATH_VAR_NAME == ", PATH_VAR_NAME);
    }
  else
    {
    Passed("PATH_VAR_NAME == ", PATH_VAR_NAME);
    }
#endif

#ifndef CACHE_TEST_VAR1
  Failed("the LOAD_CACHE or CONFIGURE_FILE command is broken, "
         "CACHE_TEST_VAR1 is not defined.");
#else
  if(strcmp(CACHE_TEST_VAR1, "foo") != 0)
    {
    Failed("the FOREACH, SET or CONFIGURE_FILE command is broken, "
           "CACHE_TEST_VAR1 == ", CACHE_TEST_VAR1);
    }
  else
    {
    Passed("CACHE_TEST_VAR1 == ", CACHE_TEST_VAR1);
    }
#endif

#ifndef CACHE_TEST_VAR2
  Failed("the LOAD_CACHE or CONFIGURE_FILE command is broken, "
         "CACHE_TEST_VAR2 is not defined.");
#else
  if(strcmp(CACHE_TEST_VAR2, "bar") != 0)
    {
    Failed("the FOREACH, SET or CONFIGURE_FILE command is broken, "
           "CACHE_TEST_VAR2 == ", CACHE_TEST_VAR2);
    }
  else
    {
    Passed("CACHE_TEST_VAR2 == ", CACHE_TEST_VAR2);
    }
#endif

#ifndef CACHE_TEST_VAR3
  Failed("the LOAD_CACHE or CONFIGURE_FILE command is broken, "
         "CACHE_TEST_VAR3 is not defined.");
#else
  if(strcmp(CACHE_TEST_VAR3, "1") != 0)
    {
    Failed("the FOREACH, SET or CONFIGURE_FILE command is broken, "
           "CACHE_TEST_VAR3 == ", CACHE_TEST_VAR3);
    }
  else
    {
    Passed("CACHE_TEST_VAR3 == ", CACHE_TEST_VAR3);
    }
#endif

  // A post-build custom-command has been attached to the lib (see Library/).
  // It run ${CREATE_FILE_EXE} which will create the file
  // ${Complex_BINARY_DIR}/Library/postbuild.txt.

  TestAndRemoveFile(BINARY_DIR "/Library/postbuild.txt");

  // A custom target has been created (see Library/).
  // It run ${CREATE_FILE_EXE} which will create the file
  // ${Complex_BINARY_DIR}/Library/custom_target1.txt.

  TestAndRemoveFile(BINARY_DIR "/Library/custom_target1.txt");

  // A directory has been created.

  TestDir(BINARY_DIR "/make_dir");

  std::cout << "Passed: " << passed << "\n";
  if(failed)
    {
    std::cout << "Failed: " << failed << "\n";
    return failed;
    }

  return 0;
}
