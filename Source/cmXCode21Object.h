/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

#include "cmXCodeObject.h"

class cmXCode21Object : public cmXCodeObject
{
public:
  cmXCode21Object(PBXType ptype, Type type, std::string id);
  void PrintComment(std::ostream&) override;
  static void PrintList(std::vector<std::unique_ptr<cmXCodeObject>> const&,
                        std::ostream& out, PBXType t);
  static void PrintList(std::vector<std::unique_ptr<cmXCodeObject>> const&,
                        std::ostream& out);
};
