/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <set>
#include <utility>
#include <vector>

#include <cm/optional>
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
extern cm::string_view const SOURCES;
extern cm::string_view const CXX_MODULES;

enum class FileSetLookup
{
  // Search for file sets attached to the target
  Target,
  // Search also file sets inherited from link libraries
  Dependencies
};

// Define the various modes regarding graph dependency for
// the generated files (Ninja specific)
// items must be kept in this order: "Lower" modes are "stronger" in that they
// have more restrictions (and therefore allow for more build graph
// optimization).
// std::set rely on it.
enum class DependencyMode
{
  IndependentFiles, // files in the file set are independent from each other
  Includables,      // files can be used by another source during compilation
};
using DependencySet = std::set<DependencyMode>;

struct FileSetDescriptor
{
  FileSetDescriptor(cm::string_view type, FileSetLookup lookup,
                    DependencySet dependencies,
                    DependencyMode defaultDependency)
    : Type(type)
    , Lookup(lookup)
    , SupportedDependencies(std::move(dependencies))
    , DefaultDependency(defaultDependency)
  {
  }

  FileSetDescriptor(FileSetLookup lookup)
    : Type()
    , Lookup(lookup)
    , SupportedDependencies({ DependencyMode::Includables })
    , DefaultDependency(DependencyMode::Includables)
  {
  }

  cm::string_view const Type;
  FileSetLookup const Lookup;
  DependencySet const SupportedDependencies;
  DependencyMode const DefaultDependency;
};

cm::optional<FileSetDescriptor> GetFileSetDescriptor(cm::string_view type);
DependencyMode GetDependencyMode(cm::string_view type);
DependencyMode GetDependencyMode(cm::string_view type,
                                 DependencyMode requestedMode);

std::vector<cm::string_view> const& GetKnownTypes();
bool IsKnownType(cm::string_view type);

// check validity of a user's file set name
bool IsValidName(cm::string_view type);
}
}
