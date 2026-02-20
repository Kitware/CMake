/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <vector>

#include <cm/string_view>

class cmMakefile;

namespace cm {
namespace FileSetMetadata {
enum class Visibility
{
  Private,
  Public,
  Interface
};

cm::string_view VisibilityToName(Visibility vis);
Visibility VisibilityFromName(cm::string_view name, cmMakefile* mf);

bool VisibilityIsForSelf(Visibility vis);
bool VisibilityIsForInterface(Visibility vis);

// Pre-defined FileSet types
extern cm::string_view const HEADERS;
extern cm::string_view const CXX_MODULES;

std::vector<cm::string_view> const& GetKnownTypes();
bool IsKnownType(cm::string_view type);

// check validity of a user's file set name
bool IsValidName(cm::string_view type);
}
}
