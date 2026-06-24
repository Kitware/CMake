/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmAddCustomRuleCommand.h"

#include <algorithm>
#include <utility>

#include <cm/memory>
#include <cm/optional>
#include <cm/string_view>
#include <cmext/algorithm>
#include <cmext/string_view>

#include "cmsys/RegularExpression.hxx"

#include "cmArgumentParser.h"
#include "cmArgumentParserTypes.h"
#include "cmExecutionStatus.h"
#include "cmGeneratorExpression.h"
#include "cmMakefile.h"
#include "cmRange.h"
#include "cmRule.h"
#include "cmState.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

namespace {
bool IsReservedName(std::string const& name)
{
  static cmsys::RegularExpression reservedNameValidator("^[A-Z_.:+-]+$");

  return reservedNameValidator.find(name);
}

template <typename Result>
class FromRuleArgumentParser : public cmArgumentParser<Result>
{
public:
  FromRuleArgumentParser()
    : cmArgumentParser<Result>()
  {
    this->Bind("CHAIN"_s, &Result::Chain)
      .Bind("OVERRIDE"_s, &Result::Override)
      .BindParsedKeywords(&Result::ParsedKeywords);
  }
};
}

bool cmAddCustomRuleCommand(std::vector<std::string> const& args,
                            cmExecutionStatus& status)
{
  if (args.size() < 2) {
    status.SetError("called with incorrect number of arguments");
    return false;
  }

  // keywords
  static cm::static_string_view const COMMAND{ "COMMAND"_s };
  static cm::static_string_view const OUTPUT{ "OUTPUT"_s };
  static cm::static_string_view const DEPENDS{ "DEPENDS"_s };
  static cm::static_string_view const DEPFILE{ "DEPFILE"_s };
  static cm::static_string_view const BYPRODUCTS{ "BYPRODUCTS"_s };
  static cm::static_string_view const GLOBAL{ "GLOBAL"_s };
  static cm::static_string_view const CONFIGURATOR{ "CONFIGURATOR"_s };
  static cm::static_string_view const FOR_FILE_SET{ "FOR_FILE_SET"_s };
  static cm::static_string_view const FOR_SOURCE{ "FOR_SOURCE"_s };
  static cm::static_string_view const FROM_RULE{ "FROM_RULE"_s };

  static cm::string_view const Keywords[]{ COMMAND,      OUTPUT,     DEPENDS,
                                           DEPFILE,      BYPRODUCTS, GLOBAL,
                                           CONFIGURATOR, FROM_RULE };
  cmMakefile& mf = status.GetMakefile();
  std::string const& ruleName = args[0];

  // Check the rule name.
  if (cm::contains(Keywords, ruleName)) {
    status.SetError("rule name is missing.");
    return false;
  }
  // check name validity
  if (IsReservedName(ruleName)) {
    status.SetError("names in all uppercase are reserved for CMake.");
    return false;
  }
  if (!cmGeneratorExpression::IsValidTargetName(ruleName)) {
    status.SetError(cmStrCat("invalid name for RULE: ", ruleName, '.'));
    return false;
  }

  // Make sure the rule does not already exist.
  if (mf.FindRuleToUse(ruleName)) {
    status.SetError(
      cmStrCat("cannot create RULE \"", ruleName,
               "\" because another RULE with the same name already exists."));
    return false;
  }

  struct BaseArguments : public ArgumentParser::ParseResult
  {
    cm::optional<ArgumentParser::NonEmpty<std::vector<std::string>>>
      Configurators;
    bool Global = false;
    std::vector<cm::string_view> ParsedKeywords;

    cm::RuleScope GetScope()
    {
      return this->Global ? cm::RuleScope::Global : cm::RuleScope::Local;
    }
  };

  if (cm::contains(args, FROM_RULE)) {
    struct Arguments : public BaseArguments
    {
      std::string FromRule;
    };

    std::vector<std::string> unexpectedArgs;
    auto parser = cmArgumentParser<Arguments>{}
                    .Bind(FROM_RULE, &Arguments::FromRule)
                    .Bind(CONFIGURATOR, &Arguments::Configurators)
                    .Bind(GLOBAL, &Arguments::Global)
                    .BindParsedKeywords(&Arguments::ParsedKeywords);
    auto parsedArgs =
      parser.Parse(cmMakeRange(args).advance(1), &unexpectedArgs);

    // do various checks for arguments consistency
    if (!parsedArgs.Check("", &unexpectedArgs, status)) {
      cmSystemTools::SetFatalErrorOccurred();
      return false;
    }

    if ((std::count(parsedArgs.ParsedKeywords.cbegin(),
                    parsedArgs.ParsedKeywords.cend(), FROM_RULE) > 1) ||
        (std::count(parsedArgs.ParsedKeywords.cbegin(),
                    parsedArgs.ParsedKeywords.cend(), CONFIGURATOR) > 1)) {
      status.SetError(
        "only one occurrence of \"FROM_RULE\" or \"CONFIGURATOR\" "
        "options is allowed.");
      return false;
    }

    // Configurator syntax: <configurator> (CHAIN|OVERRIDE)
    struct ConfiguratorArguments : public ArgumentParser::ParseResult
    {
      std::string Configurator;
      bool Chain = false;
      bool Override = false;
      std::vector<cm::string_view> ParsedKeywords;
    };

    // configurators syntax: FOR_FILE_SET <configurator> <options>
    //                       FOR_SOURCE <configurator> <options>
    struct ConfiguratorsArguments : public ArgumentParser::ParseResult
    {
      cm::optional<ConfiguratorArguments> ForFileSet;
      cm::optional<ConfiguratorArguments> ForSource;
    } parsedConfigurators;

    if (parsedArgs.Configurators) {
      auto fileSetConfiguratorParser =
        FromRuleArgumentParser<ConfiguratorArguments>{}.Bind(
          FOR_FILE_SET, &ConfiguratorArguments::Configurator);

      auto sourceConfiguratorParser =
        FromRuleArgumentParser<ConfiguratorArguments>{}.Bind(
          FOR_SOURCE, &ConfiguratorArguments::Configurator);

      auto configuratorsParser =
        cmArgumentParser<ConfiguratorsArguments>{}
          .BindSubParser(FOR_FILE_SET, fileSetConfiguratorParser,
                         &ConfiguratorsArguments::ForFileSet)
          .BindSubParser(FOR_SOURCE, sourceConfiguratorParser,
                         &ConfiguratorsArguments::ForSource);

      unexpectedArgs.clear();
      configuratorsParser.Parse(parsedConfigurators, *parsedArgs.Configurators,
                                &unexpectedArgs);

      // do various checks for arguments consistency
      if (!parsedConfigurators.Check("", &unexpectedArgs, status)) {
        cmSystemTools::SetFatalErrorOccurred();
        return false;
      }

      if (!parsedConfigurators.ForFileSet && !parsedConfigurators.ForSource) {
        status.SetError(
          cmStrCat("cannot create RULE \"", ruleName,
                   "\" because the options \"FOR_FILE_SET\" or \"FOR_SOURCE\" "
                   "are expected for the \"CONFIGURATOR\" option."));
        cmSystemTools::SetFatalErrorOccurred();
        return false;
      }

      if ((parsedConfigurators.ForFileSet &&
           std::count(parsedConfigurators.ForFileSet->ParsedKeywords.cbegin(),
                      parsedConfigurators.ForFileSet->ParsedKeywords.cend(),
                      FOR_FILE_SET) > 1) ||
          (parsedConfigurators.ForSource &&
           std::count(parsedConfigurators.ForSource->ParsedKeywords.cbegin(),
                      parsedConfigurators.ForSource->ParsedKeywords.cend(),
                      FOR_SOURCE) > 1)) {
        status.SetError(
          "only one occurrence of \"FOR_FILE_SET\" or \"FOR_SOURCE\" "
          "sub-options of \"CONFIGURATOR\" option is allowed.");
        return false;
      }

      auto checkConfigurator =
        [&status, &mf,
         &ruleName](cm::optional<ConfiguratorArguments>& configurator,
                    cm::string_view type) -> bool {
        if (!configurator) {
          return true;
        }
        ConfiguratorArguments& ca = configurator.value();

        cm::optional<cmStateEnums::CommandType> commandType =
          mf.GetState()->GetCommandType(ca.Configurator);
        if (!commandType) {
          status.SetError(cmStrCat("command specified for \"", type,
                                   "\" does not exist: ", ca.Configurator,
                                   '.'));
          cmSystemTools::SetFatalErrorOccurred();
          return false;
        }
        if (*commandType != cmStateEnums::CommandType::Function) {
          status.SetError(cmStrCat("command specified for \"", type,
                                   "\" is not a function: ", ca.Configurator,
                                   '.'));
          cmSystemTools::SetFatalErrorOccurred();
          return false;
        }
        if (ca.Chain && ca.Override) {
          status.SetError(cmStrCat("cannot create RULE \"", ruleName,
                                   "\" because the \"CHAIN\" and \"OVERRIDE\" "
                                   "options of CONFIGURATOR \"",
                                   ca.Configurator,
                                   "\" are mutually exclusive."));
          cmSystemTools::SetFatalErrorOccurred();
          return false;
        }

        if (!ca.Chain && !ca.Override) {
          ca.Override = true;
        }

        return true;
      };

      if (!checkConfigurator(parsedConfigurators.ForFileSet, FOR_FILE_SET) ||
          !checkConfigurator(parsedConfigurators.ForSource, FOR_SOURCE)) {
        return false;
      }
    }

    cmRule const* rule = mf.FindRuleToUse(parsedArgs.FromRule);
    if (!rule) {
      status.SetError(cmStrCat("cannot create RULE \"", ruleName,
                               "\" because the RULE \"", parsedArgs.FromRule,
                               "\" does not exist or is not accessible."));
      cmSystemTools::SetFatalErrorOccurred();
      return false;
    }

    auto newRule = cm::make_unique<cmSpecializedRule>(mf, ruleName, *rule,
                                                      parsedArgs.GetScope());

    if (parsedArgs.Configurators) {
      if (parsedConfigurators.ForFileSet) {
        newRule->SetConfigurator(
          cmRule::ConfiguratorType::FileSet,
          std::move(parsedConfigurators.ForFileSet->Configurator),
          parsedConfigurators.ForFileSet->Chain
            ? cmRule::ChainConfigurators::Yes
            : cmRule::ChainConfigurators::No);
      }
      if (parsedConfigurators.ForSource) {
        newRule->SetConfigurator(
          cmRule::ConfiguratorType::Source,
          std::move(parsedConfigurators.ForSource->Configurator),
          parsedConfigurators.ForSource->Chain
            ? cmRule::ChainConfigurators::Yes
            : cmRule::ChainConfigurators::No);
      }
    }

    mf.AddRule(std::move(newRule));

    return true;
  }

  struct Arguments : public BaseArguments
  {
    ArgumentParser::NonEmpty<std::vector<std::string>> Output;
    cm::optional<ArgumentParser::MaybeEmpty<std::vector<std::string>>>
      Byproducts;
    ArgumentParser::NonEmpty<std::vector<std::vector<std::string>>> Commands;
    cm::optional<ArgumentParser::MaybeEmpty<std::vector<std::string>>> Depends;
    cm::optional<std::string> Depfile;
  };

  std::vector<std::string> unexpectedArgs;
  auto parser = cmArgumentParser<Arguments>{}
                  .Bind(OUTPUT, &Arguments::Output)
                  .Bind(BYPRODUCTS, &Arguments::Byproducts)
                  .Bind(COMMAND, &Arguments::Commands)
                  .Bind(DEPENDS, &Arguments::Depends)
                  .Bind(DEPFILE, &Arguments::Depfile)
                  .Bind(CONFIGURATOR, &Arguments::Configurators)
                  .Bind(GLOBAL, &Arguments::Global)
                  .BindParsedKeywords(&Arguments::ParsedKeywords);

  auto parsedArgs =
    parser.Parse(cmMakeRange(args).advance(1), &unexpectedArgs);

  // do various checks for arguments consistency
  if (!parsedArgs.Check("", &unexpectedArgs, status)) {
    cmSystemTools::SetFatalErrorOccurred();
    return false;
  }

  if (parsedArgs.Commands.empty() || parsedArgs.Output.empty()) {
    status.SetError(cmStrCat(
      "cannot create RULE \"", ruleName,
      "\" because the mandatory options \"COMMAND\" or \"OUTPUT\" are "
      "missing."));
    cmSystemTools::SetFatalErrorOccurred();
    return false;
  }

  if ((std::count(parsedArgs.ParsedKeywords.cbegin(),
                  parsedArgs.ParsedKeywords.cend(), DEPFILE) > 1) ||
      (std::count(parsedArgs.ParsedKeywords.cbegin(),
                  parsedArgs.ParsedKeywords.cend(), CONFIGURATOR) > 1)) {
    status.SetError("only one occurrence of \"DEPFILE\" or \"CONFIGURATOR\" "
                    "options is allowed.");
    return false;
  }

  struct ConfiguratorsArguments : public ArgumentParser::ParseResult
  {
    cm::optional<std::string> ForFileSet;
    cm::optional<std::string> ForSource;
    std::vector<cm::string_view> ParsedKeywords;
  } parsedConfigurators;

  if (parsedArgs.Configurators) {
    // parse the arguments of CONFIGURATOR option
    // CONFIGURATOR syntax: FOR_FILE_SET <configurator>
    //                      FOR_SOURCE <configurator>
    auto configuratorsParser =
      cmArgumentParser<ConfiguratorsArguments>{}
        .Bind(FOR_FILE_SET, &ConfiguratorsArguments::ForFileSet)
        .Bind(FOR_SOURCE, &ConfiguratorsArguments::ForSource)
        .BindParsedKeywords(&ConfiguratorsArguments::ParsedKeywords);

    unexpectedArgs.clear();
    configuratorsParser.Parse(parsedConfigurators, *parsedArgs.Configurators,
                              &unexpectedArgs);

    // do various checks for arguments consistency
    if (!parsedConfigurators.Check("", &unexpectedArgs, status)) {
      cmSystemTools::SetFatalErrorOccurred();
      return false;
    }

    if ((std::count(parsedConfigurators.ParsedKeywords.cbegin(),
                    parsedConfigurators.ParsedKeywords.cend(),
                    FOR_FILE_SET) > 1) ||
        (std::count(parsedConfigurators.ParsedKeywords.cbegin(),
                    parsedConfigurators.ParsedKeywords.cend(),
                    FOR_SOURCE) > 1)) {
      status.SetError(
        "only one occurrence of \"FOR_FILE_SET\" or \"FOR_SOURCE\" "
        "sub-options of \"CONFIGURATOR\" option is allowed.");
      return false;
    }

    if (!parsedConfigurators.ForFileSet && !parsedConfigurators.ForSource) {
      status.SetError(cmStrCat(
        "cannot create RULE \"", ruleName,
        "\" because the options \"FOR_FILE_SET\" or \"FOR_SOURCE\" are "
        "expected for the \"CONFIGURATOR\" option."));
      cmSystemTools::SetFatalErrorOccurred();
      return false;
    }

    auto checkConfigurator =
      [&status, &mf](cm::optional<std::string> const& configurator,
                     cm::string_view type) -> bool {
      if (!configurator) {
        return true;
      }

      auto commandType = mf.GetState()->GetCommandType(*configurator);
      if (!commandType) {
        status.SetError(cmStrCat("command specified for \"", type,
                                 "\" does not exist: ", *configurator, '.'));
        cmSystemTools::SetFatalErrorOccurred();
        return false;
      }
      if (*commandType != cmStateEnums::CommandType::Function) {
        status.SetError(cmStrCat("command specified for \"", type,
                                 "\" is not a function: ", *configurator,
                                 '.'));
        cmSystemTools::SetFatalErrorOccurred();
        return false;
      }
      return true;
    };

    if (!checkConfigurator(parsedConfigurators.ForFileSet, FOR_FILE_SET) ||
        !checkConfigurator(parsedConfigurators.ForSource, FOR_SOURCE)) {
      return false;
    }
  }

  auto rule =
    cm::make_unique<cmCustomRule>(mf, ruleName, parsedArgs.Commands,
                                  parsedArgs.Output, parsedArgs.GetScope());
  if (parsedArgs.Byproducts) {
    rule->SetByproducts(std::move(*parsedArgs.Byproducts));
  }
  if (parsedArgs.Depends) {
    rule->SetDepends(std::move(*parsedArgs.Depends));
  }
  if (parsedArgs.Depfile) {
    rule->SetDepfile(std::move(*parsedArgs.Depfile));
  }
  if (parsedArgs.Configurators) {
    if (parsedConfigurators.ForFileSet) {
      rule->SetConfigurator(cmRule::ConfiguratorType::FileSet,
                            std::move(*parsedConfigurators.ForFileSet));
    }
    if (parsedConfigurators.ForSource) {
      rule->SetConfigurator(cmRule::ConfiguratorType::Source,
                            std::move(*parsedConfigurators.ForSource));
    }
  }

  mf.AddRule(std::move(rule));

  return true;
}
