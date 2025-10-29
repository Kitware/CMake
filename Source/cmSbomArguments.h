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
#include "cmProjectInfoArguments.h"

class cmSbomArguments : public cmProjectInfoArguments
{
public:
  enum class SbomFormat
  {
    SPDX_3_0_JSON,
    NONE,
  };

  template <
    typename T,
    typename = cm::enable_if_t<std::is_base_of<cmSbomArguments, T>::value>>
  static void Bind(cmArgumentParser<T>& parser)
  {
    cmSbomArguments* const self = nullptr;
    cmSbomArguments::Bind(parser, self);
  }
  void Bind(cmArgumentParser<void>& parser) { Bind(parser, this); }

  bool Check(cmExecutionStatus& status) const override;
  std::string GetNamespace() const;
  std::string GetPackageDirName() const;
  std::string GetPackageFileName() const;
  SbomFormat GetFormat() const;

  ArgumentParser::NonEmpty<std::string> Format;

protected:
  cm::string_view CommandName() const override;

private:
  using cmProjectInfoArguments::Bind;

  template <typename T>
  static void Bind(cmArgumentParser<T>& parser, cmSbomArguments* self)
  {
    cmProjectInfoArguments* const base = self;
    Bind(base, parser, "SBOM"_s, &cmProjectInfoArguments::PackageName);
    Bind(self, parser, "FORMAT"_s, &cmSbomArguments::Format);
    cmProjectInfoArguments::Bind(parser, self);
  }
};

extern template void cmSbomArguments::Bind<void>(cmArgumentParser<void>&,
                                                 cmSbomArguments*);
