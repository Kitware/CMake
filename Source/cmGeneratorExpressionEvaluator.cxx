/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2012 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmMakefile.h"

#include "cmGeneratorExpressionEvaluator.h"
#include "cmGeneratorExpressionParser.h"
#include "cmGeneratorExpressionDAGChecker.h"
#include "cmGeneratorExpression.h"

#include <cmsys/String.h>

#include <assert.h>

//----------------------------------------------------------------------------
#if !defined(__SUNPRO_CC) || __SUNPRO_CC > 0x510
static
#endif
void reportError(cmGeneratorExpressionContext *context,
                        const std::string &expr, const std::string &result)
{
  context->HadError = true;
  if (context->Quiet)
    {
    return;
    }

  cmOStringStream e;
  e << "Error evaluating generator expression:\n"
    << "  " << expr << "\n"
    << result;
  context->Makefile->GetCMakeInstance()
    ->IssueMessage(cmake::FATAL_ERROR, e.str().c_str(),
                    context->Backtrace);
}

//----------------------------------------------------------------------------
struct cmGeneratorExpressionNode
{
  enum {
    DynamicParameters = 0,
    OneOrMoreParameters = -1,
    ZeroOrMoreParameters = -2
  };
  virtual ~cmGeneratorExpressionNode() {}

  virtual bool GeneratesContent() const { return true; }

  virtual bool RequiresLiteralInput() const { return false; }

  virtual bool AcceptsArbitraryContentParameter() const
    { return false; }

  virtual int NumExpectedParameters() const { return 1; }

  virtual std::string Evaluate(const std::vector<std::string> &parameters,
                               cmGeneratorExpressionContext *context,
                               const GeneratorExpressionContent *content,
                               cmGeneratorExpressionDAGChecker *dagChecker
                              ) const = 0;
};

//----------------------------------------------------------------------------
static const struct ZeroNode : public cmGeneratorExpressionNode
{
  ZeroNode() {}

  virtual bool GeneratesContent() const { return false; }

  virtual bool AcceptsArbitraryContentParameter() const { return true; }

  std::string Evaluate(const std::vector<std::string> &,
                       cmGeneratorExpressionContext *,
                       const GeneratorExpressionContent *,
                       cmGeneratorExpressionDAGChecker *) const
  {
    // Unreachable
    return std::string();
  }
} zeroNode;

//----------------------------------------------------------------------------
static const struct OneNode : public cmGeneratorExpressionNode
{
  OneNode() {}

  virtual bool AcceptsArbitraryContentParameter() const { return true; }

  std::string Evaluate(const std::vector<std::string> &,
                       cmGeneratorExpressionContext *,
                       const GeneratorExpressionContent *,
                       cmGeneratorExpressionDAGChecker *) const
  {
    // Unreachable
    return std::string();
  }
} oneNode;

//----------------------------------------------------------------------------
static const struct OneNode buildInterfaceNode;

//----------------------------------------------------------------------------
static const struct ZeroNode installInterfaceNode;

//----------------------------------------------------------------------------
#define BOOLEAN_OP_NODE(OPNAME, OP, SUCCESS_VALUE, FAILURE_VALUE) \
static const struct OP ## Node : public cmGeneratorExpressionNode \
{ \
  OP ## Node () {} \
  virtual int NumExpectedParameters() const { return OneOrMoreParameters; } \
 \
  std::string Evaluate(const std::vector<std::string> &parameters, \
                       cmGeneratorExpressionContext *context, \
                       const GeneratorExpressionContent *content, \
                       cmGeneratorExpressionDAGChecker *) const \
  { \
    std::vector<std::string>::const_iterator it = parameters.begin(); \
    const std::vector<std::string>::const_iterator end = parameters.end(); \
    for ( ; it != end; ++it) \
      { \
      if (*it == #FAILURE_VALUE) \
        { \
        return #FAILURE_VALUE; \
        } \
      else if (*it != #SUCCESS_VALUE) \
        { \
        reportError(context, content->GetOriginalExpression(), \
        "Parameters to $<" #OP "> must resolve to either '0' or '1'."); \
        return std::string(); \
        } \
      } \
    return #SUCCESS_VALUE; \
  } \
} OPNAME;

BOOLEAN_OP_NODE(andNode, AND, 1, 0)
BOOLEAN_OP_NODE(orNode, OR, 0, 1)

#undef BOOLEAN_OP_NODE

//----------------------------------------------------------------------------
static const struct NotNode : public cmGeneratorExpressionNode
{
  NotNode() {}

  std::string Evaluate(const std::vector<std::string> &parameters,
                       cmGeneratorExpressionContext *context,
                       const GeneratorExpressionContent *content,
                       cmGeneratorExpressionDAGChecker *) const
  {
    if (*parameters.begin() != "0" && *parameters.begin() != "1")
      {
      reportError(context, content->GetOriginalExpression(),
            "$<NOT> parameter must resolve to exactly one '0' or '1' value.");
      return std::string();
      }
    return *parameters.begin() == "0" ? "1" : "0";
  }
} notNode;

//----------------------------------------------------------------------------
static const struct BoolNode : public cmGeneratorExpressionNode
{
  BoolNode() {}

  virtual int NumExpectedParameters() const { return 1; }

  std::string Evaluate(const std::vector<std::string> &parameters,
                       cmGeneratorExpressionContext *,
                       const GeneratorExpressionContent *,
                       cmGeneratorExpressionDAGChecker *) const
  {
    return !cmSystemTools::IsOff(parameters.begin()->c_str()) ? "1" : "0";
  }
} boolNode;

//----------------------------------------------------------------------------
static const struct StrEqualNode : public cmGeneratorExpressionNode
{
  StrEqualNode() {}

  virtual int NumExpectedParameters() const { return 2; }

  std::string Evaluate(const std::vector<std::string> &parameters,
                       cmGeneratorExpressionContext *,
                       const GeneratorExpressionContent *,
                       cmGeneratorExpressionDAGChecker *) const
  {
    return *parameters.begin() == parameters[1] ? "1" : "0";
  }
} strEqualNode;

//----------------------------------------------------------------------------
static const struct Angle_RNode : public cmGeneratorExpressionNode
{
  Angle_RNode() {}

  virtual int NumExpectedParameters() const { return 0; }

  std::string Evaluate(const std::vector<std::string> &,
                       cmGeneratorExpressionContext *,
                       const GeneratorExpressionContent *,
                       cmGeneratorExpressionDAGChecker *) const
  {
    return ">";
  }
} angle_rNode;

//----------------------------------------------------------------------------
static const struct CommaNode : public cmGeneratorExpressionNode
{
  CommaNode() {}

  virtual int NumExpectedParameters() const { return 0; }

  std::string Evaluate(const std::vector<std::string> &,
                       cmGeneratorExpressionContext *,
                       const GeneratorExpressionContent *,
                       cmGeneratorExpressionDAGChecker *) const
  {
    return ",";
  }
} commaNode;

//----------------------------------------------------------------------------
static const struct SemicolonNode : public cmGeneratorExpressionNode
{
  SemicolonNode() {}

  virtual int NumExpectedParameters() const { return 0; }

  std::string Evaluate(const std::vector<std::string> &,
                       cmGeneratorExpressionContext *,
                       const GeneratorExpressionContent *,
                       cmGeneratorExpressionDAGChecker *) const
  {
    return ";";
  }
} semicolonNode;

//----------------------------------------------------------------------------
struct CompilerIdNode : public cmGeneratorExpressionNode
{
  CompilerIdNode() {}

  virtual int NumExpectedParameters() const { return ZeroOrMoreParameters; }

  std::string EvaluateWithLanguage(const std::vector<std::string> &parameters,
                       cmGeneratorExpressionContext *context,
                       const GeneratorExpressionContent *content,
                       cmGeneratorExpressionDAGChecker *,
                       const std::string &lang) const
  {
    const char *compilerId = context->Makefile ?
                              context->Makefile->GetSafeDefinition((
                              "CMAKE_" + lang + "_COMPILER_ID").c_str()) : "";
    if (parameters.size() == 0)
      {
      return compilerId ? compilerId : "";
      }
    cmsys::RegularExpression compilerIdValidator;
    compilerIdValidator.compile("^[A-Za-z0-9_]*$");
    if (!compilerIdValidator.find(parameters.begin()->c_str()))
      {
      reportError(context, content->GetOriginalExpression(),
                  "Expression syntax not recognized.");
      return std::string();
      }
    if (!compilerId)
      {
      return parameters.front().empty() ? "1" : "0";
      }

    if (cmsysString_strcasecmp(parameters.begin()->c_str(), compilerId) == 0)
      {
      return "1";
      }
    return "0";
  }
};

//----------------------------------------------------------------------------
static const struct CCompilerIdNode : public CompilerIdNode
{
  CCompilerIdNode() {}

  std::string Evaluate(const std::vector<std::string> &parameters,
                       cmGeneratorExpressionContext *context,
                       const GeneratorExpressionContent *content,
                       cmGeneratorExpressionDAGChecker *dagChecker) const
  {
    if (parameters.size() != 0 && parameters.size() != 1)
      {
      reportError(context, content->GetOriginalExpression(),
          "$<C_COMPILER_ID> expression requires one or two parameters");
      return std::string();
      }
    if (!context->HeadTarget)
      {
      reportError(context, content->GetOriginalExpression(),
          "$<C_COMPILER_ID> may only be used with targets.  It may not "
          "be used with add_custom_command.");
      }
    return this->EvaluateWithLanguage(parameters, context, content,
                                      dagChecker, "C");
  }
} cCompilerIdNode;

//----------------------------------------------------------------------------
static const struct CXXCompilerIdNode : public CompilerIdNode
{
  CXXCompilerIdNode() {}

  std::string Evaluate(const std::vector<std::string> &parameters,
                       cmGeneratorExpressionContext *context,
                       const GeneratorExpressionContent *content,
                       cmGeneratorExpressionDAGChecker *dagChecker) const
  {
    if (parameters.size() != 0 && parameters.size() != 1)
      {
      reportError(context, content->GetOriginalExpression(),
          "$<CXX_COMPILER_ID> expression requires one or two parameters");
      return std::string();
      }
    if (!context->HeadTarget)
      {
      reportError(context, content->GetOriginalExpression(),
          "$<CXX_COMPILER_ID> may only be used with targets.  It may not "
          "be used with add_custom_command.");
      }
    return this->EvaluateWithLanguage(parameters, context, content,
                                      dagChecker, "CXX");
  }
} cxxCompilerIdNode;

//----------------------------------------------------------------------------
struct CompilerVersionNode : public cmGeneratorExpressionNode
{
  CompilerVersionNode() {}

  virtual int NumExpectedParameters() const { return ZeroOrMoreParameters; }

  std::string EvaluateWithLanguage(const std::vector<std::string> &parameters,
                       cmGeneratorExpressionContext *context,
                       const GeneratorExpressionContent *content,
                       cmGeneratorExpressionDAGChecker *,
                       const std::string &lang) const
  {
    const char *compilerVersion = context->Makefile ?
                              context->Makefile->GetSafeDefinition((
                        "CMAKE_" + lang + "_COMPILER_VERSION").c_str()) : "";
    if (parameters.size() == 0)
      {
      return compilerVersion ? compilerVersion : "";
      }

    cmsys::RegularExpression compilerIdValidator;
    compilerIdValidator.compile("^[0-9\\.]*$");
    if (!compilerIdValidator.find(parameters.begin()->c_str()))
      {
      reportError(context, content->GetOriginalExpression(),
                  "Expression syntax not recognized.");
      return std::string();
      }
    if (!compilerVersion)
      {
      return parameters.front().empty() ? "1" : "0";
      }

    return cmSystemTools::VersionCompare(cmSystemTools::OP_EQUAL,
                                      parameters.begin()->c_str(),
                                      compilerVersion) ? "1" : "0";
  }
};

//----------------------------------------------------------------------------
static const struct CCompilerVersionNode : public CompilerVersionNode
{
  CCompilerVersionNode() {}

  std::string Evaluate(const std::vector<std::string> &parameters,
                       cmGeneratorExpressionContext *context,
                       const GeneratorExpressionContent *content,
                       cmGeneratorExpressionDAGChecker *dagChecker) const
  {
    if (parameters.size() != 0 && parameters.size() != 1)
      {
      reportError(context, content->GetOriginalExpression(),
          "$<C_COMPILER_VERSION> expression requires one or two parameters");
      return std::string();
      }
    if (!context->HeadTarget)
      {
      reportError(context, content->GetOriginalExpression(),
          "$<C_COMPILER_VERSION> may only be used with targets.  It may not "
          "be used with add_custom_command.");
      }
    return this->EvaluateWithLanguage(parameters, context, content,
                                      dagChecker, "C");
  }
} cCompilerVersionNode;

//----------------------------------------------------------------------------
static const struct CxxCompilerVersionNode : public CompilerVersionNode
{
  CxxCompilerVersionNode() {}

  std::string Evaluate(const std::vector<std::string> &parameters,
                       cmGeneratorExpressionContext *context,
                       const GeneratorExpressionContent *content,
                       cmGeneratorExpressionDAGChecker *dagChecker) const
  {
    if (parameters.size() != 0 && parameters.size() != 1)
      {
      reportError(context, content->GetOriginalExpression(),
          "$<CXX_COMPILER_VERSION> expression requires one or two "
          "parameters");
      return std::string();
      }
    if (!context->HeadTarget)
      {
      reportError(context, content->GetOriginalExpression(),
          "$<CXX_COMPILER_VERSION> may only be used with targets.  It may "
          "not be used with add_custom_command.");
      }
    return this->EvaluateWithLanguage(parameters, context, content,
                                      dagChecker, "CXX");
  }
} cxxCompilerVersionNode;


//----------------------------------------------------------------------------
static const struct VersionGreaterNode : public cmGeneratorExpressionNode
{
  VersionGreaterNode() {}

  virtual int NumExpectedParameters() const { return 2; }

  std::string Evaluate(const std::vector<std::string> &parameters,
                       cmGeneratorExpressionContext *,
                       const GeneratorExpressionContent *,
                       cmGeneratorExpressionDAGChecker *) const
  {
    return cmSystemTools::VersionCompare(cmSystemTools::OP_GREATER,
                                         parameters.front().c_str(),
                                         parameters[1].c_str()) ? "1" : "0";
  }
} versionGreaterNode;

//----------------------------------------------------------------------------
static const struct VersionLessNode : public cmGeneratorExpressionNode
{
  VersionLessNode() {}

  virtual int NumExpectedParameters() const { return 2; }

  std::string Evaluate(const std::vector<std::string> &parameters,
                       cmGeneratorExpressionContext *,
                       const GeneratorExpressionContent *,
                       cmGeneratorExpressionDAGChecker *) const
  {
    return cmSystemTools::VersionCompare(cmSystemTools::OP_LESS,
                                         parameters.front().c_str(),
                                         parameters[1].c_str()) ? "1" : "0";
  }
} versionLessNode;

//----------------------------------------------------------------------------
static const struct VersionEqualNode : public cmGeneratorExpressionNode
{
  VersionEqualNode() {}

  virtual int NumExpectedParameters() const { return 2; }

  std::string Evaluate(const std::vector<std::string> &parameters,
                       cmGeneratorExpressionContext *,
                       const GeneratorExpressionContent *,
                       cmGeneratorExpressionDAGChecker *) const
  {
    return cmSystemTools::VersionCompare(cmSystemTools::OP_EQUAL,
                                         parameters.front().c_str(),
                                         parameters[1].c_str()) ? "1" : "0";
  }
} versionEqualNode;

//----------------------------------------------------------------------------
static const struct LinkOnlyNode : public cmGeneratorExpressionNode
{
  LinkOnlyNode() {}

  std::string Evaluate(const std::vector<std::string> &parameters,
                       cmGeneratorExpressionContext *,
                       const GeneratorExpressionContent *,
                       cmGeneratorExpressionDAGChecker *dagChecker) const
  {
    if(!dagChecker->GetTransitivePropertiesOnly())
      {
      return parameters.front();
      }
    return "";
  }
} linkOnlyNode;

//----------------------------------------------------------------------------
static const struct ConfigurationNode : public cmGeneratorExpressionNode
{
  ConfigurationNode() {}

  virtual int NumExpectedParameters() const { return 0; }

  std::string Evaluate(const std::vector<std::string> &,
                       cmGeneratorExpressionContext *context,
                       const GeneratorExpressionContent *,
                       cmGeneratorExpressionDAGChecker *) const
  {
    context->HadContextSensitiveCondition = true;
    return context->Config ? context->Config : "";
  }
} configurationNode;

//----------------------------------------------------------------------------
static const struct ConfigurationTestNode : public cmGeneratorExpressionNode
{
  ConfigurationTestNode() {}

  virtual int NumExpectedParameters() const { return 1; }

  std::string Evaluate(const std::vector<std::string> &parameters,
                       cmGeneratorExpressionContext *context,
                       const GeneratorExpressionContent *content,
                       cmGeneratorExpressionDAGChecker *) const
  {
    cmsys::RegularExpression configValidator;
    configValidator.compile("^[A-Za-z0-9_]*$");
    if (!configValidator.find(parameters.begin()->c_str()))
      {
      reportError(context, content->GetOriginalExpression(),
                  "Expression syntax not recognized.");
      return std::string();
      }
    context->HadContextSensitiveCondition = true;
    if (!context->Config)
      {
      return parameters.front().empty() ? "1" : "0";
      }

    if (cmsysString_strcasecmp(parameters.begin()->c_str(),
                                  context->Config) == 0)
      {
      return "1";
      }

    if (context->CurrentTarget
        && context->CurrentTarget->IsImported())
      {
      const char* loc = 0;
      const char* imp = 0;
      std::string suffix;
      if (context->CurrentTarget->GetMappedConfig(context->Config,
                                                  &loc,
                                                  &imp,
                                                  suffix))
        {
        // This imported target has an appropriate location
        // for this (possibly mapped) config.
        // Check if there is a proper config mapping for the tested config.
        std::vector<std::string> mappedConfigs;
        std::string mapProp = "MAP_IMPORTED_CONFIG_";
        mapProp += cmSystemTools::UpperCase(context->Config);
        if(const char* mapValue =
                        context->CurrentTarget->GetProperty(mapProp.c_str()))
          {
          cmSystemTools::ExpandListArgument(cmSystemTools::UpperCase(mapValue),
                                            mappedConfigs);
          return std::find(mappedConfigs.begin(), mappedConfigs.end(),
                           cmSystemTools::UpperCase(parameters.front()))
              != mappedConfigs.end() ? "1" : "0";
          }
        }
      }
    return "0";
  }
} configurationTestNode;

static const struct JoinNode : public cmGeneratorExpressionNode
{
  JoinNode() {}

  virtual int NumExpectedParameters() const { return 2; }

  virtual bool AcceptsArbitraryContentParameter() const { return true; }

  std::string Evaluate(const std::vector<std::string> &parameters,
                       cmGeneratorExpressionContext *,
                       const GeneratorExpressionContent *,
                       cmGeneratorExpressionDAGChecker *) const
  {
    std::string result;

    std::vector<std::string> list;
    cmSystemTools::ExpandListArgument(parameters.front(), list);
    std::string sep;
    for(std::vector<std::string>::const_iterator li = list.begin();
      li != list.end(); ++li)
      {
      result += sep + *li;
      sep = parameters[1];
      }
    return result;
  }
} joinNode;

#define TRANSITIVE_PROPERTY_NAME(PROPERTY) \
  , #PROPERTY

//----------------------------------------------------------------------------
static const char* targetPropertyTransitiveWhitelist[] = {
  0
  CM_FOR_EACH_TRANSITIVE_PROPERTY_NAME(TRANSITIVE_PROPERTY_NAME)
};

std::string getLinkedTargetsContent(const std::vector<std::string> &libraries,
                                  cmTarget *target,
                                  cmTarget *headTarget,
                                  cmGeneratorExpressionContext *context,
                                  cmGeneratorExpressionDAGChecker *dagChecker,
                                  const std::string &interfacePropertyName)
{
  cmGeneratorExpression ge(context->Backtrace);

  std::string sep;
  std::string depString;
  for (std::vector<std::string>::const_iterator
      it = libraries.begin();
      it != libraries.end(); ++it)
    {
    if (*it == target->GetName())
      {
      // Broken code can have a target in its own link interface.
      // Don't follow such link interface entries so as not to create a
      // self-referencing loop.
      continue;
      }
    if (context->Makefile->FindTargetToUse(it->c_str()))
      {
      depString +=
        sep + "$<TARGET_PROPERTY:" + *it + "," + interfacePropertyName + ">";
      sep = ";";
      }
    }
  cmsys::auto_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(depString);
  std::string linkedTargetsContent = cge->Evaluate(context->Makefile,
                      context->Config,
                      context->Quiet,
                      headTarget,
                      target,
                      dagChecker);
  if (cge->GetHadContextSensitiveCondition())
    {
    context->HadContextSensitiveCondition = true;
    }
  return linkedTargetsContent;
}

//----------------------------------------------------------------------------
struct TransitiveWhitelistCompare
{
  explicit TransitiveWhitelistCompare(const std::string &needle)
    : Needle(needle) {}
  bool operator() (const char *item)
  { return strcmp(item, this->Needle.c_str()) == 0; }
private:
  std::string Needle;
};

//----------------------------------------------------------------------------
static const struct TargetPropertyNode : public cmGeneratorExpressionNode
{
  TargetPropertyNode() {}

  // This node handles errors on parameter count itself.
  virtual int NumExpectedParameters() const { return OneOrMoreParameters; }

  std::string Evaluate(const std::vector<std::string> &parameters,
                       cmGeneratorExpressionContext *context,
                       const GeneratorExpressionContent *content,
                       cmGeneratorExpressionDAGChecker *dagCheckerParent
                      ) const
  {
    if (parameters.size() != 1 && parameters.size() != 2)
      {
      reportError(context, content->GetOriginalExpression(),
          "$<TARGET_PROPERTY:...> expression requires one or two parameters");
      return std::string();
      }
    cmsys::RegularExpression propertyNameValidator;
    propertyNameValidator.compile("^[A-Za-z0-9_]+$");

    cmTarget* target = context->HeadTarget;
    std::string propertyName = *parameters.begin();

    if (!target && parameters.size() == 1)
      {
      reportError(context, content->GetOriginalExpression(),
          "$<TARGET_PROPERTY:prop> may only be used with targets.  It may not "
          "be used with add_custom_command.  Specify the target to read a "
          "property from using the $<TARGET_PROPERTY:tgt,prop> signature "
          "instead.");
      return std::string();
      }

    if (parameters.size() == 2)
      {
      if (parameters.begin()->empty() && parameters[1].empty())
        {
        reportError(context, content->GetOriginalExpression(),
            "$<TARGET_PROPERTY:tgt,prop> expression requires a non-empty "
            "target name and property name.");
        return std::string();
        }
      if (parameters.begin()->empty())
        {
        reportError(context, content->GetOriginalExpression(),
            "$<TARGET_PROPERTY:tgt,prop> expression requires a non-empty "
            "target name.");
        return std::string();
        }

      std::string targetName = parameters.front();
      propertyName = parameters[1];
      if (!cmGeneratorExpression::IsValidTargetName(targetName))
        {
        if (!propertyNameValidator.find(propertyName.c_str()))
          {
          ::reportError(context, content->GetOriginalExpression(),
                        "Target name and property name not supported.");
          return std::string();
          }
        ::reportError(context, content->GetOriginalExpression(),
                      "Target name not supported.");
        return std::string();
        }
      if(propertyName == "ALIASED_TARGET")
        {
        if(context->Makefile->IsAlias(targetName.c_str()))
          {
          if(cmTarget* tgt =
                      context->Makefile->FindTargetToUse(targetName.c_str()))
            {
            return tgt->GetName();
            }
          }
        return "";
        }
      target = context->Makefile->FindTargetToUse(
                                                targetName.c_str());

      if (!target)
        {
        cmOStringStream e;
        e << "Target \""
          << targetName
          << "\" not found.";
        reportError(context, content->GetOriginalExpression(), e.str());
        return std::string();
        }
        context->AllTargets.insert(target);
      }

    if (target == context->HeadTarget)
      {
      // Keep track of the properties seen while processing.
      // The evaluation of the LINK_LIBRARIES generator expressions
      // will check this to ensure that properties have one consistent
      // value for all evaluations.
      context->SeenTargetProperties.insert(propertyName);
      }

    if (propertyName.empty())
      {
      reportError(context, content->GetOriginalExpression(),
          "$<TARGET_PROPERTY:...> expression requires a non-empty property "
          "name.");
      return std::string();
      }

    if (!propertyNameValidator.find(propertyName.c_str()))
      {
      ::reportError(context, content->GetOriginalExpression(),
                    "Property name not supported.");
      return std::string();
      }

    assert(target);

    if (propertyName == "LINKER_LANGUAGE")
      {
      if (target->LinkLanguagePropagatesToDependents() &&
          dagCheckerParent && dagCheckerParent->EvaluatingLinkLibraries())
        {
        reportError(context, content->GetOriginalExpression(),
            "LINKER_LANGUAGE target property can not be used while evaluating "
            "link libraries for a static library");
        return std::string();
        }
      const char *lang = target->GetLinkerLanguage(context->Config);
      return lang ? lang : "";
      }

    cmGeneratorExpressionDAGChecker dagChecker(context->Backtrace,
                                               target->GetName(),
                                               propertyName,
                                               content,
                                               dagCheckerParent);

    switch (dagChecker.check())
      {
    case cmGeneratorExpressionDAGChecker::SELF_REFERENCE:
      dagChecker.reportError(context, content->GetOriginalExpression());
      return std::string();
    case cmGeneratorExpressionDAGChecker::CYCLIC_REFERENCE:
      // No error. We just skip cyclic references.
      return std::string();
    case cmGeneratorExpressionDAGChecker::ALREADY_SEEN:
      for (size_t i = 1;
          i < (sizeof(targetPropertyTransitiveWhitelist) /
                sizeof(*targetPropertyTransitiveWhitelist));
          ++i)
        {
        if (targetPropertyTransitiveWhitelist[i] == propertyName)
          {
          // No error. We're not going to find anything new here.
          return std::string();
          }
        }
    case cmGeneratorExpressionDAGChecker::DAG:
      break;
      }

    const char *prop = target->GetProperty(propertyName.c_str());

    if (dagCheckerParent)
      {
      if (dagCheckerParent->EvaluatingLinkLibraries())
        {
        if(!prop)
          {
          return std::string();
          }
        }
      else
        {
#define ASSERT_TRANSITIVE_PROPERTY_METHOD(METHOD) \
  dagCheckerParent->METHOD () ||

        assert(
          CM_FOR_EACH_TRANSITIVE_PROPERTY_METHOD(
                                            ASSERT_TRANSITIVE_PROPERTY_METHOD)
          false);
        }
      }

    std::string linkedTargetsContent;

    std::string interfacePropertyName;

    if (propertyName == "INTERFACE_INCLUDE_DIRECTORIES"
        || propertyName == "INCLUDE_DIRECTORIES")
      {
      interfacePropertyName = "INTERFACE_INCLUDE_DIRECTORIES";
      }
    else if (propertyName == "INTERFACE_SYSTEM_INCLUDE_DIRECTORIES")
      {
      interfacePropertyName = "INTERFACE_SYSTEM_INCLUDE_DIRECTORIES";
      }
    else if (propertyName == "INTERFACE_COMPILE_DEFINITIONS"
        || propertyName == "COMPILE_DEFINITIONS"
        || strncmp(propertyName.c_str(), "COMPILE_DEFINITIONS_", 20) == 0)
      {
      interfacePropertyName = "INTERFACE_COMPILE_DEFINITIONS";
      }
    else if (propertyName == "INTERFACE_COMPILE_OPTIONS"
        || propertyName == "COMPILE_OPTIONS")
      {
      interfacePropertyName = "INTERFACE_COMPILE_OPTIONS";
      }

    cmTarget *headTarget = context->HeadTarget ? context->HeadTarget : target;

    const char **transBegin = targetPropertyTransitiveWhitelist + 1;
    const char **transEnd = targetPropertyTransitiveWhitelist
              + (sizeof(targetPropertyTransitiveWhitelist) /
              sizeof(*targetPropertyTransitiveWhitelist));
    if (std::find_if(transBegin, transEnd,
              TransitiveWhitelistCompare(propertyName)) != transEnd)
      {

      std::vector<std::string> libs;
      target->GetTransitivePropertyLinkLibraries(context->Config,
                                                 headTarget, libs);
      if (!libs.empty())
        {
        linkedTargetsContent =
                  getLinkedTargetsContent(libs, target,
                                          headTarget,
                                          context, &dagChecker,
                                          interfacePropertyName);
        }
      }
    else if (std::find_if(transBegin, transEnd,
              TransitiveWhitelistCompare(interfacePropertyName)) != transEnd)
      {
      const cmTarget::LinkImplementation *impl = target->GetLinkImplementation(
                                                    context->Config,
                                                    headTarget);
      if(impl)
        {
        linkedTargetsContent =
                  getLinkedTargetsContent(impl->Libraries, target,
                                          headTarget,
                                          context, &dagChecker,
                                          interfacePropertyName);
        }
      }

    linkedTargetsContent =
          cmGeneratorExpression::StripEmptyListElements(linkedTargetsContent);

    if (!prop)
      {
      if (target->IsImported())
        {
        return linkedTargetsContent;
        }
      if (target->IsLinkInterfaceDependentBoolProperty(propertyName,
                                                       context->Config))
        {
        context->HadContextSensitiveCondition = true;
        return target->GetLinkInterfaceDependentBoolProperty(
                                                propertyName,
                                                context->Config) ? "1" : "0";
        }
      if (target->IsLinkInterfaceDependentStringProperty(propertyName,
                                                         context->Config))
        {
        context->HadContextSensitiveCondition = true;
        const char *propContent =
                              target->GetLinkInterfaceDependentStringProperty(
                                                propertyName,
                                                context->Config);
        return propContent ? propContent : "";
        }

      return linkedTargetsContent;
      }

    for (size_t i = 1;
         i < (sizeof(targetPropertyTransitiveWhitelist) /
              sizeof(*targetPropertyTransitiveWhitelist));
         ++i)
      {
      if (targetPropertyTransitiveWhitelist[i] == interfacePropertyName)
        {
        cmGeneratorExpression ge(context->Backtrace);
        cmsys::auto_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(prop);
        std::string result = cge->Evaluate(context->Makefile,
                            context->Config,
                            context->Quiet,
                            headTarget,
                            target,
                            &dagChecker);

        if (cge->GetHadContextSensitiveCondition())
          {
          context->HadContextSensitiveCondition = true;
          }
        if (!linkedTargetsContent.empty())
          {
          result += (result.empty() ? "" : ";") + linkedTargetsContent;
          }
        return result;
        }
      }
    return prop;
  }
} targetPropertyNode;

//----------------------------------------------------------------------------
static const struct TargetNameNode : public cmGeneratorExpressionNode
{
  TargetNameNode() {}

  virtual bool GeneratesContent() const { return true; }

  virtual bool AcceptsArbitraryContentParameter() const { return true; }
  virtual bool RequiresLiteralInput() const { return true; }

  std::string Evaluate(const std::vector<std::string> &parameters,
                       cmGeneratorExpressionContext *,
                       const GeneratorExpressionContent *,
                       cmGeneratorExpressionDAGChecker *) const
  {
    return parameters.front();
  }

  virtual int NumExpectedParameters() const { return 1; }

} targetNameNode;

//----------------------------------------------------------------------------
static const char* targetPolicyWhitelist[] = {
  0
#define TARGET_POLICY_STRING(POLICY) \
  , #POLICY

  CM_FOR_EACH_TARGET_POLICY(TARGET_POLICY_STRING)

#undef TARGET_POLICY_STRING
};

cmPolicies::PolicyStatus statusForTarget(cmTarget *tgt, const char *policy)
{
#define RETURN_POLICY(POLICY) \
  if (strcmp(policy, #POLICY) == 0) \
  { \
    return tgt->GetPolicyStatus ## POLICY (); \
  } \

  CM_FOR_EACH_TARGET_POLICY(RETURN_POLICY)

#undef RETURN_POLICY

  assert("!Unreachable code. Not a valid policy");
  return cmPolicies::WARN;
}

cmPolicies::PolicyID policyForString(const char *policy_id)
{
#define RETURN_POLICY_ID(POLICY_ID) \
  if (strcmp(policy_id, #POLICY_ID) == 0) \
  { \
    return cmPolicies:: POLICY_ID; \
  } \

  CM_FOR_EACH_TARGET_POLICY(RETURN_POLICY_ID)

#undef RETURN_POLICY_ID

  assert("!Unreachable code. Not a valid policy");
  return cmPolicies::CMP0002;
}

//----------------------------------------------------------------------------
static const struct TargetPolicyNode : public cmGeneratorExpressionNode
{
  TargetPolicyNode() {}

  virtual int NumExpectedParameters() const { return 1; }

  std::string Evaluate(const std::vector<std::string> &parameters,
                       cmGeneratorExpressionContext *context ,
                       const GeneratorExpressionContent *content,
                       cmGeneratorExpressionDAGChecker *) const
  {
    if (!context->HeadTarget)
      {
      reportError(context, content->GetOriginalExpression(),
          "$<TARGET_POLICY:prop> may only be used with targets.  It may not "
          "be used with add_custom_command.");
      return std::string();
      }

    context->HadContextSensitiveCondition = true;

    for (size_t i = 1;
         i < (sizeof(targetPolicyWhitelist) /
              sizeof(*targetPolicyWhitelist));
         ++i)
      {
      const char *policy = targetPolicyWhitelist[i];
      if (parameters.front() == policy)
        {
        cmMakefile *mf = context->HeadTarget->GetMakefile();
        switch(statusForTarget(context->HeadTarget, policy))
          {
          case cmPolicies::WARN:
            mf->IssueMessage(cmake::AUTHOR_WARNING,
                             mf->GetPolicies()->
                             GetPolicyWarning(policyForString(policy)));
          case cmPolicies::REQUIRED_IF_USED:
          case cmPolicies::REQUIRED_ALWAYS:
          case cmPolicies::OLD:
            return "0";
          case cmPolicies::NEW:
            return "1";
          }
        }
      }
    reportError(context, content->GetOriginalExpression(),
      "$<TARGET_POLICY:prop> may only be used with a limited number of "
      "policies.  Currently it may be used with the following policies:\n"

#define STRINGIFY_HELPER(X) #X
#define STRINGIFY(X) STRINGIFY_HELPER(X)

#define TARGET_POLICY_LIST_ITEM(POLICY) \
      " * " STRINGIFY(POLICY) "\n"

      CM_FOR_EACH_TARGET_POLICY(TARGET_POLICY_LIST_ITEM)

#undef TARGET_POLICY_LIST_ITEM
      );
    return std::string();
  }

} targetPolicyNode;

//----------------------------------------------------------------------------
static const struct InstallPrefixNode : public cmGeneratorExpressionNode
{
  InstallPrefixNode() {}

  virtual bool GeneratesContent() const { return true; }
  virtual int NumExpectedParameters() const { return 0; }

  std::string Evaluate(const std::vector<std::string> &,
                       cmGeneratorExpressionContext *context,
                       const GeneratorExpressionContent *content,
                       cmGeneratorExpressionDAGChecker *) const
  {
    reportError(context, content->GetOriginalExpression(),
                "INSTALL_PREFIX is a marker for install(EXPORT) only.  It "
                "should never be evaluated.");
    return std::string();
  }

} installPrefixNode;

//----------------------------------------------------------------------------
template<bool linker, bool soname>
struct TargetFilesystemArtifactResultCreator
{
  static std::string Create(cmTarget* target,
                            cmGeneratorExpressionContext *context,
                            const GeneratorExpressionContent *content);
};

//----------------------------------------------------------------------------
template<>
struct TargetFilesystemArtifactResultCreator<false, true>
{
  static std::string Create(cmTarget* target,
                            cmGeneratorExpressionContext *context,
                            const GeneratorExpressionContent *content)
  {
    // The target soname file (.so.1).
    if(target->IsDLLPlatform())
      {
      ::reportError(context, content->GetOriginalExpression(),
                    "TARGET_SONAME_FILE is not allowed "
                    "for DLL target platforms.");
      return std::string();
      }
    if(target->GetType() != cmTarget::SHARED_LIBRARY)
      {
      ::reportError(context, content->GetOriginalExpression(),
                    "TARGET_SONAME_FILE is allowed only for "
                    "SHARED libraries.");
      return std::string();
      }
    std::string result = target->GetDirectory(context->Config);
    result += "/";
    result += target->GetSOName(context->Config);
    return result;
  }
};

//----------------------------------------------------------------------------
template<>
struct TargetFilesystemArtifactResultCreator<true, false>
{
  static std::string Create(cmTarget* target,
                            cmGeneratorExpressionContext *context,
                            const GeneratorExpressionContent *content)
  {
    // The file used to link to the target (.so, .lib, .a).
    if(!target->IsLinkable())
      {
      ::reportError(context, content->GetOriginalExpression(),
                    "TARGET_LINKER_FILE is allowed only for libraries and "
                    "executables with ENABLE_EXPORTS.");
      return std::string();
      }
    return target->GetFullPath(context->Config,
                               target->HasImportLibrary());
  }
};

//----------------------------------------------------------------------------
template<>
struct TargetFilesystemArtifactResultCreator<false, false>
{
  static std::string Create(cmTarget* target,
                            cmGeneratorExpressionContext *context,
                            const GeneratorExpressionContent *)
  {
    return target->GetFullPath(context->Config, false, true);
  }
};


//----------------------------------------------------------------------------
template<bool dirQual, bool nameQual>
struct TargetFilesystemArtifactResultGetter
{
  static std::string Get(const std::string &result);
};

//----------------------------------------------------------------------------
template<>
struct TargetFilesystemArtifactResultGetter<false, true>
{
  static std::string Get(const std::string &result)
  { return cmSystemTools::GetFilenameName(result); }
};

//----------------------------------------------------------------------------
template<>
struct TargetFilesystemArtifactResultGetter<true, false>
{
  static std::string Get(const std::string &result)
  { return cmSystemTools::GetFilenamePath(result); }
};

//----------------------------------------------------------------------------
template<>
struct TargetFilesystemArtifactResultGetter<false, false>
{
  static std::string Get(const std::string &result)
  { return result; }
};

//----------------------------------------------------------------------------
template<bool linker, bool soname, bool dirQual, bool nameQual>
struct TargetFilesystemArtifact : public cmGeneratorExpressionNode
{
  TargetFilesystemArtifact() {}

  virtual int NumExpectedParameters() const { return 1; }

  std::string Evaluate(const std::vector<std::string> &parameters,
                       cmGeneratorExpressionContext *context,
                       const GeneratorExpressionContent *content,
                       cmGeneratorExpressionDAGChecker *dagChecker) const
  {
    // Lookup the referenced target.
    std::string name = *parameters.begin();

    if (!cmGeneratorExpression::IsValidTargetName(name))
      {
      ::reportError(context, content->GetOriginalExpression(),
                    "Expression syntax not recognized.");
      return std::string();
      }
    cmTarget* target = context->Makefile->FindTargetToUse(name.c_str());
    if(!target)
      {
      ::reportError(context, content->GetOriginalExpression(),
                    "No target \"" + name + "\"");
      return std::string();
      }
    if(target->GetType() >= cmTarget::OBJECT_LIBRARY &&
      target->GetType() != cmTarget::UNKNOWN_LIBRARY)
      {
      ::reportError(context, content->GetOriginalExpression(),
                  "Target \"" + name + "\" is not an executable or library.");
      return std::string();
      }
    if (dagChecker && dagChecker->EvaluatingLinkLibraries(name.c_str()))
      {
      ::reportError(context, content->GetOriginalExpression(),
                    "Expressions which require the linker language may not "
                    "be used while evaluating link libraries");
      return std::string();
      }
    context->DependTargets.insert(target);
    context->AllTargets.insert(target);

    std::string result =
                TargetFilesystemArtifactResultCreator<linker, soname>::Create(
                          target,
                          context,
                          content);
    if (context->HadError)
      {
      return std::string();
      }
    return
        TargetFilesystemArtifactResultGetter<dirQual, nameQual>::Get(result);
  }
};

//----------------------------------------------------------------------------
static const
TargetFilesystemArtifact<false, false, false, false> targetFileNode;
static const
TargetFilesystemArtifact<true, false, false, false> targetLinkerFileNode;
static const
TargetFilesystemArtifact<false, true, false, false> targetSoNameFileNode;
static const
TargetFilesystemArtifact<false, false, false, true> targetFileNameNode;
static const
TargetFilesystemArtifact<true, false, false, true> targetLinkerFileNameNode;
static const
TargetFilesystemArtifact<false, true, false, true> targetSoNameFileNameNode;
static const
TargetFilesystemArtifact<false, false, true, false> targetFileDirNode;
static const
TargetFilesystemArtifact<true, false, true, false> targetLinkerFileDirNode;
static const
TargetFilesystemArtifact<false, true, true, false> targetSoNameFileDirNode;

//----------------------------------------------------------------------------
static const
cmGeneratorExpressionNode* GetNode(const std::string &identifier)
{
  if (identifier == "0")
    return &zeroNode;
  else if (identifier == "1")
    return &oneNode;
  else if (identifier == "AND")
    return &andNode;
  else if (identifier == "OR")
    return &orNode;
  else if (identifier == "NOT")
    return &notNode;
  else if (identifier == "C_COMPILER_ID")
    return &cCompilerIdNode;
  else if (identifier == "CXX_COMPILER_ID")
    return &cxxCompilerIdNode;
  else if (identifier == "VERSION_GREATER")
    return &versionGreaterNode;
  else if (identifier == "VERSION_LESS")
    return &versionLessNode;
  else if (identifier == "VERSION_EQUAL")
    return &versionEqualNode;
  else if (identifier == "C_COMPILER_VERSION")
    return &cCompilerVersionNode;
  else if (identifier == "CXX_COMPILER_VERSION")
    return &cxxCompilerVersionNode;
  else if (identifier == "CONFIGURATION")
    return &configurationNode;
  else if (identifier == "CONFIG")
    return &configurationTestNode;
  else if (identifier == "TARGET_FILE")
    return &targetFileNode;
  else if (identifier == "TARGET_LINKER_FILE")
    return &targetLinkerFileNode;
  else if (identifier == "TARGET_SONAME_FILE")
    return &targetSoNameFileNode;
  else if (identifier == "TARGET_FILE_NAME")
    return &targetFileNameNode;
  else if (identifier == "TARGET_LINKER_FILE_NAME")
    return &targetLinkerFileNameNode;
  else if (identifier == "TARGET_SONAME_FILE_NAME")
    return &targetSoNameFileNameNode;
  else if (identifier == "TARGET_FILE_DIR")
    return &targetFileDirNode;
  else if (identifier == "TARGET_LINKER_FILE_DIR")
    return &targetLinkerFileDirNode;
  else if (identifier == "TARGET_SONAME_FILE_DIR")
    return &targetSoNameFileDirNode;
  else if (identifier == "STREQUAL")
    return &strEqualNode;
  else if (identifier == "BOOL")
    return &boolNode;
  else if (identifier == "ANGLE-R")
    return &angle_rNode;
  else if (identifier == "COMMA")
    return &commaNode;
  else if (identifier == "SEMICOLON")
    return &semicolonNode;
  else if (identifier == "TARGET_PROPERTY")
    return &targetPropertyNode;
  else if (identifier == "TARGET_NAME")
    return &targetNameNode;
  else if (identifier == "TARGET_POLICY")
    return &targetPolicyNode;
  else if (identifier == "BUILD_INTERFACE")
    return &buildInterfaceNode;
  else if (identifier == "INSTALL_INTERFACE")
    return &installInterfaceNode;
  else if (identifier == "INSTALL_PREFIX")
    return &installPrefixNode;
  else if (identifier == "JOIN")
    return &joinNode;
  else if (identifier == "LINK_ONLY")
    return &linkOnlyNode;
  return 0;

}

//----------------------------------------------------------------------------
GeneratorExpressionContent::GeneratorExpressionContent(
                                                    const char *startContent,
                                                    unsigned int length)
  : StartContent(startContent), ContentLength(length)
{

}

//----------------------------------------------------------------------------
std::string GeneratorExpressionContent::GetOriginalExpression() const
{
  return std::string(this->StartContent, this->ContentLength);
}

//----------------------------------------------------------------------------
std::string GeneratorExpressionContent::ProcessArbitraryContent(
    const cmGeneratorExpressionNode *node,
    const std::string &identifier,
    cmGeneratorExpressionContext *context,
    cmGeneratorExpressionDAGChecker *dagChecker,
    std::vector<std::vector<cmGeneratorExpressionEvaluator*> >::const_iterator
    pit) const
{
  std::string result;

  const
  std::vector<std::vector<cmGeneratorExpressionEvaluator*> >::const_iterator
                                      pend = this->ParamChildren.end();
  for ( ; pit != pend; ++pit)
    {
    std::vector<cmGeneratorExpressionEvaluator*>::const_iterator it
                                                            = pit->begin();
    const std::vector<cmGeneratorExpressionEvaluator*>::const_iterator end
                                                              = pit->end();
    for ( ; it != end; ++it)
      {
      if (node->RequiresLiteralInput())
        {
        if ((*it)->GetType() != cmGeneratorExpressionEvaluator::Text)
          {
          reportError(context, this->GetOriginalExpression(),
                "$<" + identifier + "> expression requires literal input.");
          return std::string();
          }
        }
      result += (*it)->Evaluate(context, dagChecker);
      if (context->HadError)
        {
        return std::string();
        }
      }
      if ((pit + 1) != pend)
        {
        result += ",";
        }
    }
  if (node->RequiresLiteralInput())
    {
    std::vector<std::string> parameters;
    parameters.push_back(result);
    return node->Evaluate(parameters, context, this, dagChecker);
    }
  return result;
}

//----------------------------------------------------------------------------
std::string GeneratorExpressionContent::Evaluate(
                            cmGeneratorExpressionContext *context,
                            cmGeneratorExpressionDAGChecker *dagChecker) const
{
  std::string identifier;
  {
  std::vector<cmGeneratorExpressionEvaluator*>::const_iterator it
                                          = this->IdentifierChildren.begin();
  const std::vector<cmGeneratorExpressionEvaluator*>::const_iterator end
                                          = this->IdentifierChildren.end();
  for ( ; it != end; ++it)
    {
    identifier += (*it)->Evaluate(context, dagChecker);
    if (context->HadError)
      {
      return std::string();
      }
    }
  }

  const cmGeneratorExpressionNode *node = GetNode(identifier);

  if (!node)
    {
    reportError(context, this->GetOriginalExpression(),
              "Expression did not evaluate to a known generator expression");
    return std::string();
    }

  if (!node->GeneratesContent())
    {
    if (node->NumExpectedParameters() == 1
        && node->AcceptsArbitraryContentParameter())
      {
      if (this->ParamChildren.empty())
        {
        reportError(context, this->GetOriginalExpression(),
                  "$<" + identifier + "> expression requires a parameter.");
        }
      }
    else
      {
      std::vector<std::string> parameters;
      this->EvaluateParameters(node, identifier, context, dagChecker,
                               parameters);
      }
    return std::string();
    }

  if (node->NumExpectedParameters() == 1
        && node->AcceptsArbitraryContentParameter())
    {
    return this->ProcessArbitraryContent(node, identifier, context,
                                         dagChecker,
                                         this->ParamChildren.begin());
    }

  std::vector<std::string> parameters;
  this->EvaluateParameters(node, identifier, context, dagChecker, parameters);
  if (context->HadError)
    {
    return std::string();
    }

  return node->Evaluate(parameters, context, this, dagChecker);
}

//----------------------------------------------------------------------------
std::string GeneratorExpressionContent::EvaluateParameters(
                                const cmGeneratorExpressionNode *node,
                                const std::string &identifier,
                                cmGeneratorExpressionContext *context,
                                cmGeneratorExpressionDAGChecker *dagChecker,
                                std::vector<std::string> &parameters) const
{
  const int numExpected = node->NumExpectedParameters();
  {
  std::vector<std::vector<cmGeneratorExpressionEvaluator*> >::const_iterator
                                        pit = this->ParamChildren.begin();
  const
  std::vector<std::vector<cmGeneratorExpressionEvaluator*> >::const_iterator
                                        pend = this->ParamChildren.end();
  const bool acceptsArbitraryContent
                                  = node->AcceptsArbitraryContentParameter();
  for ( ; pit != pend; ++pit)
    {
    std::string parameter;
    std::vector<cmGeneratorExpressionEvaluator*>::const_iterator it =
                                                              pit->begin();
    const std::vector<cmGeneratorExpressionEvaluator*>::const_iterator end =
                                                              pit->end();
    for ( ; it != end; ++it)
      {
      parameter += (*it)->Evaluate(context, dagChecker);
      if (context->HadError)
        {
        return std::string();
        }
      }
    parameters.push_back(parameter);
    if (acceptsArbitraryContent
        && parameters.size() == (unsigned int)numExpected - 1)
      {
      assert(pit != pend);
      std::string lastParam = this->ProcessArbitraryContent(node, identifier,
                                                            context,
                                                            dagChecker,
                                                            pit + 1);
      parameters.push_back(lastParam);
      return std::string();
      }
    }
  }

  if ((numExpected > cmGeneratorExpressionNode::DynamicParameters
      && (unsigned int)numExpected != parameters.size()))
    {
    if (numExpected == 0)
      {
      reportError(context, this->GetOriginalExpression(),
                  "$<" + identifier + "> expression requires no parameters.");
      }
    else if (numExpected == 1)
      {
      reportError(context, this->GetOriginalExpression(),
                  "$<" + identifier + "> expression requires "
                  "exactly one parameter.");
      }
    else
      {
      cmOStringStream e;
      e << "$<" + identifier + "> expression requires "
        << numExpected
        << " comma separated parameters, but got "
        << parameters.size() << " instead.";
      reportError(context, this->GetOriginalExpression(), e.str());
      }
    return std::string();
    }

  if (numExpected == cmGeneratorExpressionNode::OneOrMoreParameters
      && parameters.empty())
    {
    reportError(context, this->GetOriginalExpression(), "$<" + identifier
                      + "> expression requires at least one parameter.");
    }
  return std::string();
}

//----------------------------------------------------------------------------
static void deleteAll(const std::vector<cmGeneratorExpressionEvaluator*> &c)
{
  std::vector<cmGeneratorExpressionEvaluator*>::const_iterator it
                                                  = c.begin();
  const std::vector<cmGeneratorExpressionEvaluator*>::const_iterator end
                                                  = c.end();
  for ( ; it != end; ++it)
    {
    delete *it;
    }
}

//----------------------------------------------------------------------------
GeneratorExpressionContent::~GeneratorExpressionContent()
{
  deleteAll(this->IdentifierChildren);

  typedef std::vector<cmGeneratorExpressionEvaluator*> EvaluatorVector;
  std::vector<EvaluatorVector>::const_iterator pit =
                                                  this->ParamChildren.begin();
  const std::vector<EvaluatorVector>::const_iterator pend =
                                                  this->ParamChildren.end();
  for ( ; pit != pend; ++pit)
    {
    deleteAll(*pit);
    }
}
