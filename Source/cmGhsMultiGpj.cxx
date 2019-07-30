/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGhsMultiGpj.h"

#include <ostream>

static const char* GHS_TAG[] = { "[INTEGRITY Application]",
                                 "[Library]",
                                 "[Project]",
                                 "[Program]",
                                 "[Reference]",
                                 "[Subproject]",
                                 "[Custom Target]" };

const char* GhsMultiGpj::GetGpjTag(Types gpjType)
{
  char const* tag;
  switch (gpjType) {
    case INTERGRITY_APPLICATION:
    case LIBRARY:
    case PROJECT:
    case PROGRAM:
    case REFERENCE:
    case SUBPROJECT:
    case CUSTOM_TARGET:
      tag = GHS_TAG[gpjType];
      break;
    default:
      tag = "";
  }
  return tag;
}

void GhsMultiGpj::WriteGpjTag(Types gpjType, std::ostream& fout)
{
  char const* tag;
  tag = GhsMultiGpj::GetGpjTag(gpjType);
  fout << tag << std::endl;
}
