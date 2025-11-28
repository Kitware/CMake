/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

#include <cm/string_view>
#include <cm/type_traits>
#include <cmext/string_view>

#include "cmArgumentParser.h" // IWYU pragma: keep
#include "cmArgumentParserTypes.h"

class cmExecutionStatus;

/** \class cmProjectInfoArguments
 * \brief Convey information about a project.
 *
 * This class encapsulates several attributes of project metadata;
 * specifically, those which can be inherited from the project. It is used as
 * the base class for classes specific to SBOM and PACKAGE_INFO exports.
 */
class cmProjectInfoArguments
{
public:
  cmProjectInfoArguments();
  cmProjectInfoArguments(cmProjectInfoArguments const&) = default;
  cmProjectInfoArguments& operator=(cmProjectInfoArguments const&) = default;
  cmProjectInfoArguments(cmProjectInfoArguments&&) = default;
  cmProjectInfoArguments& operator=(cmProjectInfoArguments&&) = default;

  virtual ~cmProjectInfoArguments() = default;

  /// Ensure that no conflicting options were specified.
  virtual bool Check(cmExecutionStatus& status) const;

  /// Set metadata (not already specified) from either the specified project,
  /// or from the project which matches the package name.
  bool SetMetadataFromProject(cmExecutionStatus& status);

  ArgumentParser::NonEmpty<std::string> PackageName;
  ArgumentParser::NonEmpty<std::string> Version;
  ArgumentParser::NonEmpty<std::string> VersionCompat;
  ArgumentParser::NonEmpty<std::string> VersionSchema;
  ArgumentParser::NonEmpty<std::string> License;
  ArgumentParser::NonEmpty<std::string> Description;
  ArgumentParser::NonEmpty<std::string> Website;

  ArgumentParser::NonEmpty<std::string> ProjectName;
  bool NoProjectDefaults = false;

protected:
  virtual cm::string_view CommandName() const = 0;

  virtual bool SetEffectiveProject(cmExecutionStatus& status);

  template <typename T>
  static void Bind(cmArgumentParser<T>& parser, cmProjectInfoArguments* self)
  {
    Bind(self, parser, "VERSION"_s, &cmProjectInfoArguments::Version);
    Bind(self, parser, "LICENSE"_s, &cmProjectInfoArguments::License);
    Bind(self, parser, "DESCRIPTION"_s, &cmProjectInfoArguments::Description);
    Bind(self, parser, "HOMEPAGE_URL"_s, &cmProjectInfoArguments::Website);

    Bind(self, parser, "PROJECT"_s, &cmProjectInfoArguments::ProjectName);
    Bind(self, parser, "NO_PROJECT_METADATA"_s,
         &cmProjectInfoArguments::NoProjectDefaults);
  }

  template <
    typename Self, typename T, typename U,
    typename =
      cm::enable_if_t<std::is_base_of<cmProjectInfoArguments, Self>::value>,
    typename =
      cm::enable_if_t<std::is_base_of<cmProjectInfoArguments, T>::value>>
  static void Bind(Self*, cmArgumentParser<T>& parser,
                   cm::static_string_view name, U Self::*member)
  {
    parser.Bind(name, member);
  }

  template <
    typename Self, typename T, typename U,
    typename =
      cm::enable_if_t<std::is_base_of<cmProjectInfoArguments, Self>::value>,
    typename =
      cm::enable_if_t<std::is_base_of<cmProjectInfoArguments, T>::value>>
  static void Bind(Self* self, cmArgumentParser<void>& parser,
                   cm::static_string_view name, U T::*member)
  {
    parser.Bind(name, (self)->*member);
  }

  static bool ArgWasSpecified(std::string const& value)
  {
    return !value.empty();
  }
};

extern template void cmProjectInfoArguments::Bind<void>(
  cmArgumentParser<void>&, cmProjectInfoArguments*);
