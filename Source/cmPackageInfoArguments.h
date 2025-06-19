/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include <cm/type_traits>
#include <cmext/string_view>

#include "cmArgumentParser.h" // IWYU pragma: keep
#include "cmArgumentParserTypes.h"

class cmExecutionStatus;

/** \class cmPackageInfoArguments
 * \brief Convey information about a package.
 *
 * This class encapsulates several attributes of package metadata. It is used
 * both as a convenience container to convey several values in a single
 * container, and also provides utilities to obtain this metadata from commands
 * which produce packages (i.e. export and install).
 */
class cmPackageInfoArguments
{
public:
  template <typename T,
            typename = cm::enable_if_t<
              std::is_base_of<cmPackageInfoArguments, T>::value>>
  static void Bind(cmArgumentParser<T>& parser)
  {
    cmPackageInfoArguments::Bind(parser, nullptr);
  }

  void Bind(cmArgumentParser<void>& parser)
  {
    cmPackageInfoArguments::Bind(parser, this);
  }

  std::string GetNamespace() const;
  std::string GetPackageDirName() const;
  std::string GetPackageFileName() const;

  /// Ensure that no conflicting options were specified.  If \p enable is
  /// \c false, forbid specifying any options whatsoever.
  bool Check(cmExecutionStatus& status, bool enable = true) const;

  /// Set metadata (not already specified) from either the specified project,
  /// or from the project which matches the package name.
  bool SetMetadataFromProject(cmExecutionStatus& status);

  ArgumentParser::NonEmpty<std::string> PackageName;
  ArgumentParser::NonEmpty<std::string> Appendix;
  ArgumentParser::NonEmpty<std::string> Version;
  ArgumentParser::NonEmpty<std::string> VersionCompat;
  ArgumentParser::NonEmpty<std::string> VersionSchema;
  ArgumentParser::NonEmpty<std::string> License;
  ArgumentParser::NonEmpty<std::string> DefaultLicense;
  ArgumentParser::NonEmpty<std::string> Description;
  ArgumentParser::NonEmpty<std::string> Website;
  ArgumentParser::NonEmpty<std::vector<std::string>> DefaultTargets;
  ArgumentParser::NonEmpty<std::vector<std::string>> DefaultConfigs;
  bool LowerCase = false;

  ArgumentParser::NonEmpty<std::string> ProjectName;
  bool NoProjectDefaults = false;

private:
  bool SetEffectiveProject(cmExecutionStatus& status);

  template <typename T>
  static void Bind(cmArgumentParser<T>& parser, cmPackageInfoArguments* self)
  {
    Bind(self, parser, "PACKAGE_INFO"_s, &cmPackageInfoArguments::PackageName);
    Bind(self, parser, "LOWER_CASE_FILE"_s,
         &cmPackageInfoArguments::LowerCase);
    Bind(self, parser, "APPENDIX"_s, &cmPackageInfoArguments::Appendix);
    Bind(self, parser, "VERSION"_s, &cmPackageInfoArguments::Version);
    Bind(self, parser, "COMPAT_VERSION"_s,
         &cmPackageInfoArguments::VersionCompat);
    Bind(self, parser, "VERSION_SCHEMA"_s,
         &cmPackageInfoArguments::VersionSchema);
    Bind(self, parser, "DEFAULT_TARGETS"_s,
         &cmPackageInfoArguments::DefaultTargets);
    Bind(self, parser, "DEFAULT_CONFIGURATIONS"_s,
         &cmPackageInfoArguments::DefaultConfigs);
    Bind(self, parser, "LICENSE"_s, &cmPackageInfoArguments::License);
    Bind(self, parser, "DEFAULT_LICENSE"_s,
         &cmPackageInfoArguments::DefaultLicense);
    Bind(self, parser, "DESCRIPTION"_s, &cmPackageInfoArguments::Description);
    Bind(self, parser, "HOMEPAGE_URL"_s, &cmPackageInfoArguments::Website);

    Bind(self, parser, "PROJECT"_s, &cmPackageInfoArguments::ProjectName);
    Bind(self, parser, "NO_PROJECT_METADATA"_s,
         &cmPackageInfoArguments::NoProjectDefaults);
  }

  template <typename T, typename U,
            typename = cm::enable_if_t<
              std::is_base_of<cmPackageInfoArguments, T>::value>>
  static void Bind(cmPackageInfoArguments*, cmArgumentParser<T>& parser,
                   cm::static_string_view name,
                   U cmPackageInfoArguments::*member)
  {
    parser.Bind(name, member);
  }

  template <typename U>
  static void Bind(cmPackageInfoArguments* self,
                   cmArgumentParser<void>& parser, cm::static_string_view name,
                   U cmPackageInfoArguments::*member)
  {
    parser.Bind(name, (self)->*member);
  }
};

extern template void cmPackageInfoArguments::Bind<void>(
  cmArgumentParser<void>&, cmPackageInfoArguments*);
