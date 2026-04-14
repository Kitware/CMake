/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmFileSetMetadata.h"

#include <map>
#include <string>
#include <utility>

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
cm::string_view const SOURCES = "SOURCES"_s;
cm::string_view const CXX_MODULES = "CXX_MODULES"_s;

namespace {
std::map<cm::string_view, FileSetDescriptor> const FileSetDescriptors{
  { cm::FileSetMetadata::HEADERS,
    { cm::FileSetMetadata::HEADERS,
      cm::FileSetMetadata::FileSetLookup::Target,
      { DependencyMode ::Includables },
      DependencyMode ::Includables,
      FrameworkCompatible::No } },
  { cm::FileSetMetadata::SOURCES,
    { cm::FileSetMetadata::SOURCES,
      cm::FileSetMetadata::FileSetLookup::Dependencies,
      { DependencyMode ::IndependentFiles, DependencyMode ::Includables },
      DependencyMode ::Includables,
      FrameworkCompatible::Yes } },
  { cm::FileSetMetadata::CXX_MODULES,
    { cm::FileSetMetadata::CXX_MODULES,
      cm::FileSetMetadata::FileSetLookup::Target,
      { DependencyMode ::IndependentFiles },
      DependencyMode ::IndependentFiles,
      FrameworkCompatible::No } },
};

std::vector<cm::string_view> KnownTypes{ HEADERS, SOURCES, CXX_MODULES };

cmsys::RegularExpression const ValidNameRegex("^[a-z0-9][a-zA-Z0-9_]*$");
}

cm::optional<FileSetDescriptor> GetFileSetDescriptor(cm::string_view type)
{
  auto it = FileSetDescriptors.find(type);
  if (it != FileSetDescriptors.end()) {
    return it->second;
  }
  return cm::nullopt;
}

DependencyMode GetDependencyMode(cm::string_view type)
{
  auto descriptor = GetFileSetDescriptor(type);
  if (descriptor) {
    return descriptor->DefaultDependency;
  }
  return DependencyMode::Includables;
}
DependencyMode GetDependencyMode(cm::string_view type,
                                 DependencyMode requestedMode)
{
  auto descriptor = GetFileSetDescriptor(type);
  if (descriptor) {
    // Select the requested mode or the next-weakest mode that is supported by
    // the file set type
    auto mode = descriptor->SupportedDependencies.lower_bound(requestedMode);
    return mode == descriptor->SupportedDependencies.end()
      ? descriptor->DefaultDependency
      : *mode;
  }
  return DependencyMode::Includables;
}

bool IsFrameworkSupported(cm::string_view type)
{
  auto descriptor = GetFileSetDescriptor(type);
  if (descriptor) {
    return descriptor->FrameworkSupported == FrameworkCompatible::Yes;
  }
  return false;
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
