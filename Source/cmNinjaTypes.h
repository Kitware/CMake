/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmNinjaTypes_h
#define cmNinjaTypes_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

enum cmNinjaTargetDepends
{
  DependOnTargetArtifact,
  DependOnTargetOrdering
};

typedef std::vector<std::string> cmNinjaDeps;
typedef std::set<std::string> cmNinjaOuts;
typedef std::map<std::string, std::string> cmNinjaVars;

class cmNinjaRule
{
public:
  cmNinjaRule(std::string name)
    : Name(std::move(name))
  {
  }

  std::string Name;
  std::string Command;
  std::string Description;
  std::string Comment;
  std::string DepFile;
  std::string DepType;
  std::string RspFile;
  std::string RspContent;
  std::string Restat;
  bool Generator = false;
};

#endif // ! cmNinjaTypes_h
