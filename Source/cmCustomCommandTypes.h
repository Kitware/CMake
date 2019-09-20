/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCustomCommandTypes_h
#define cmCustomCommandTypes_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

/** Target custom command type */
enum class cmCustomCommandType
{
  PRE_BUILD,
  PRE_LINK,
  POST_BUILD
};

/** Where the command originated from. */
enum class cmCommandOrigin
{
  Project,
  Generator
};

/** How to handle custom commands for object libraries */
enum class cmObjectLibraryCommands
{
  Reject,
  Accept
};

/** Utility target output source file name.  */
struct cmUtilityOutput
{
  std::string Name;
  std::string NameCMP0049;
};

#endif
