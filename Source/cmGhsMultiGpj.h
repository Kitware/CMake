/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmGhsMultiGpj_h
#define cmGhsMultiGpj_h

#include "cmConfigure.h" // IWYU pragma: keep
#include <iosfwd>

class cmGeneratedFileStream;

class GhsMultiGpj
{
public:
  enum Types
  {
    INTERGRITY_APPLICATION,
    LIBRARY,
    PROJECT,
    PROGRAM,
    REFERENCE,
    SUBPROJECT
  };

  static void WriteGpjTag(Types const gpjType, std::ostream& fout);

  static const char* GetGpjTag(Types const gpjType);
};

#endif // ! cmGhsMultiGpjType_h
