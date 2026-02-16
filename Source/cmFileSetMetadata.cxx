/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmFileSetMetadata.h"

#include <string>

#include <cmext/algorithm>
#include <cmext/string_view>

#include "cmsys/RegularExpression.hxx"

#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

namespace cm {
namespace FileSetMetadata {
cm::string_view VisibilityToName(Visibility vis)
{
  switch (vis) {
    case Visibility::Interface:
      return "INTERFACE"_s;
    case Visibility::Public:
      return "PUBLIC"_s;
    case Visibility::Private:
      return "PRIVATE"_s;
  }
  return ""_s;
}

Visibility VisibilityFromName(cm::string_view name, cmMakefile* mf)
{
  if (name == "INTERFACE"_s) {
    return Visibility::Interface;
  }
  if (name == "PUBLIC"_s) {
    return Visibility::Public;
  }
  if (name == "PRIVATE"_s) {
    return Visibility::Private;
  }
  auto msg = cmStrCat("File set visibility \"", name, "\" is not valid.");
  if (mf) {
    mf->IssueMessage(MessageType::FATAL_ERROR, msg);
  } else {
    cmSystemTools::Error(msg);
  }
  return Visibility::Private;
}

bool VisibilityIsForSelf(Visibility vis)
{
  switch (vis) {
    case Visibility::Interface:
      return false;
    case Visibility::Public:
    case Visibility::Private:
      return true;
  }
  return false;
}

bool VisibilityIsForInterface(Visibility vis)
{
  switch (vis) {
    case Visibility::Interface:
    case Visibility::Public:
      return true;
    case Visibility::Private:
      return false;
  }
  return false;
}

cm::string_view const HEADERS = "HEADERS"_s;
cm::string_view const CXX_MODULES = "CXX_MODULES"_s;

namespace {
std::vector<cm::string_view> KnownTypes{ HEADERS, CXX_MODULES };

cmsys::RegularExpression const ValidNameRegex("^[a-z0-9][a-zA-Z0-9_]*$");
}

std::vector<cm::string_view> const& GetKnownTypes()
{
  return KnownTypes;
}

bool IsKnownType(cm::string_view type)
{
  return cm::contains(GetKnownTypes(), type);
}

bool IsValidName(cm::string_view name)
{
  cmsys::RegularExpressionMatch match;
  return ValidNameRegex.find(name.data(), match);
}

}
}
