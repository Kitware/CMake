/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

#include <cm/string_view>
#include <cm/type_traits>
#include <cmext/string_view>

#include "cmArgumentParser.h" // IWYU pragma: keep
#include "cmArgumentParserTypes.h"
#include "cmProjectInfoArguments.h"

/** \class cmPackageInfoArguments
 * \brief Convey information about a package.
 *
 * This class encapsulates several attributes of package metadata. It is used
 * both as a convenience container to convey several values in a single
 * container, and also provides utilities to obtain this metadata from commands
 * which produce packages (i.e. export and install).
 */
class cmPackageInfoArguments : public cmProjectInfoArguments
{
public:
  template <typename T,
            typename = cm::enable_if_t<
              std::is_base_of<cmPackageInfoArguments, T>::value>>
  static void Bind(cmArgumentParser<T>& parser)
  {
    cmPackageInfoArguments* const self = nullptr;
    cmPackageInfoArguments::Bind(parser, self);
  }

  void Bind(cmArgumentParser<void>& parser)
  {
    cmPackageInfoArguments::Bind(parser, this);
  }

  std::string GetNamespace() const;
  std::string GetPackageDirName() const;
  std::string GetPackageFileName() const;

  bool Check(cmExecutionStatus& status) const override;

  ArgumentParser::NonEmpty<std::string> Appendix;
  ArgumentParser::NonEmpty<std::string> DefaultLicense;
  ArgumentParser::NonEmpty<std::vector<std::string>> DefaultTargets;
  ArgumentParser::NonEmpty<std::vector<std::string>> DefaultConfigs;
  bool LowerCase = false;

protected:
  cm::string_view CommandName() const override;

  bool SetEffectiveProject(cmExecutionStatus& status) override;

  template <typename T>
  static void Bind(cmArgumentParser<T>& parser, cmPackageInfoArguments* self)
  {
    cmProjectInfoArguments* const base = self;

    Bind(base, parser, "PACKAGE_INFO"_s, &cmProjectInfoArguments::PackageName);
    Bind(self, parser, "LOWER_CASE_FILE"_s,
         &cmPackageInfoArguments::LowerCase);
    Bind(self, parser, "APPENDIX"_s, &cmPackageInfoArguments::Appendix);

    Bind(base, parser, "COMPAT_VERSION"_s,
         &cmProjectInfoArguments::VersionCompat);
    Bind(base, parser, "VERSION_SCHEMA"_s,
         &cmProjectInfoArguments::VersionSchema);
    Bind(self, parser, "DEFAULT_TARGETS"_s,
         &cmPackageInfoArguments::DefaultTargets);
    Bind(self, parser, "DEFAULT_CONFIGURATIONS"_s,
         &cmPackageInfoArguments::DefaultConfigs);
    Bind(self, parser, "DEFAULT_LICENSE"_s,
         &cmPackageInfoArguments::DefaultLicense);

    cmProjectInfoArguments::Bind(parser, self);
  }

  using cmProjectInfoArguments::Bind;

  static bool ArgWasSpecified(std::vector<std::string> const& value)
  {
    return !value.empty();
  }
  using cmProjectInfoArguments::ArgWasSpecified;
};

extern template void cmPackageInfoArguments::Bind<void>(
  cmArgumentParser<void>&, cmPackageInfoArguments*);
