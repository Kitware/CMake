/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmIDEFlagTable_h
#define cmIDEFlagTable_h

#include <string>

// This is a table mapping XML tag IDE names to command line options
struct cmIDEFlagTable
{
  std::string IDEName;     // name used in the IDE xml file
  std::string commandFlag; // command line flag
  std::string comment;     // comment
  std::string value;       // string value
  unsigned int special;    // flags for special handling requests
  enum
  {
    UserValue = (1 << 0),           // flag contains a user-specified value
    UserIgnored = (1 << 1),         // ignore any user value
    UserRequired = (1 << 2),        // match only when user value is non-empty
    Continue = (1 << 3),            // continue looking for matching entries
    SemicolonAppendable = (1 << 4), // a flag that if specified multiple times
                                    // should have its value appended to the
                                    // old value with semicolons (e.g.
                                    // /NODEFAULTLIB: =>
                                    // IgnoreDefaultLibraryNames)
    UserFollowing = (1 << 5),       // expect value in following argument
    CaseInsensitive = (1 << 6),     // flag may be any case
    SpaceAppendable = (1 << 7),     // a flag that if specified multiple times
                                    // should have its value appended to the
                                    // old value with spaces
    CommaAppendable = (1 << 8),     // a flag that if specified multiple times
                                    // should have its value appended to the
                                    // old value with commas (e.g. C# /nowarn

    UserValueIgnored = UserValue | UserIgnored,
    UserValueRequired = UserValue | UserRequired
  };
};

#endif
