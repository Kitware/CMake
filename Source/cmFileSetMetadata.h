/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <cstdint>
#include <set>
#include <utility>
#include <vector>

#include <cm/optional>
#include <cm/string_view>
#include <cmext/enum_set>

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
  Includables       // files can be used by another source during compilation
};
using DependencySet = std::set<DependencyMode>;

enum class FileSetAttributes : std::uint16_t
{
  FrameworkCompatible,    // Can be part of an Apple framework
  FilesInMultipleFileSets // Files of this file set type can be part of other
                          // file sets
};
using AttributeSet = cm::enum_set<FileSetAttributes, 2>;

struct FileSetDescriptor
{
  FileSetDescriptor(cm::string_view type, FileSetLookup lookup,
                    DependencySet dependencies,
                    DependencyMode defaultDependency, AttributeSet attributes)
    : Type(type)
    , Lookup(lookup)
    , SupportedDependencies(std::move(dependencies))
    , DefaultDependency(defaultDependency)
    , Attributes(attributes)
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
  AttributeSet const Attributes;
};

cm::optional<FileSetDescriptor> GetFileSetDescriptor(cm::string_view type);
DependencyMode GetDependencyMode(cm::string_view type);
DependencyMode GetDependencyMode(cm::string_view type,
                                 DependencyMode requestedMode);

AttributeSet GetAttributes(cm::string_view type);
bool IsFrameworkSupported(cm::string_view type);

std::vector<cm::string_view> const& GetKnownTypes();
bool IsKnownType(cm::string_view type);

// check validity of a user's file set name
bool IsValidName(cm::string_view type);
}
}

CM_ENUM_SET_TRAITS(cm::FileSetMetadata::AttributeSet)
