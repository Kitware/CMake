#include "cmTestConfigure.h"
#include "ExtraSources/file1.h"
#include "file2.h"
#include "sharedFile.h"
#include "cmStandardIncludes.h"
#include <sys/stat.h>
#include <stdio.h>

#if defined(_MSC_VER) || defined(__BORLANDC__)
#define _unlink unlink
#else
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#endif

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
  struct stat fs;
  if (stat(filename, &fs) != 0)
    {
    Failed("Could not find file: ", filename);
    }
  else
    {
    if (unlink(filename) != 0)
      {
      Failed("Unable to remove file. It does not imply that this test failed, but it *will* be corrupted thereafter if this file is not removed: ", filename);
      }
    else
      {
      Passed("Find and remove file: ", filename);
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
  if(strcmp(FILENAME_VAR_NAME, "VarTests.txt") != 0)
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
  if(strcmp(FILENAME_VAR_EXT, ".txt") != 0)
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

  // A post-build custom-command has been attached to the lib.
  // It run ${CREATE_FILE_EXE} which will create the file
  // ${Complex_BINARY_DIR}/postbuild.txt.

  TestAndRemoveFile(BINARY_DIR "/postbuild.txt");

  // A custom target has been created.
  // It run ${CREATE_FILE_EXE} which will create the file
  // ${Complex_BINARY_DIR}/custom_target1.txt.

  TestAndRemoveFile(BINARY_DIR "/custom_target1.txt");

  std::cout << "Passed: " << passed << "\n";
  if(failed)
    {
    std::cout << "Failed: " << failed << "\n";
    return failed;
    }

  return 0;
}
