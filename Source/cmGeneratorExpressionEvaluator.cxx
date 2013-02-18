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
  virtual ~cmGeneratorExpressionNode() {}

  virtual bool GeneratesContent() const { return true; }

  virtual bool RequiresLiteralInput() const { return false; }

  virtual bool AcceptsSingleArbitraryContentParameter() const
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

  virtual bool AcceptsSingleArbitraryContentParameter() const { return true; }

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

  virtual bool AcceptsSingleArbitraryContentParameter() const { return true; }

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
/* We let -1 carry the meaning 'at least one' */ \
  virtual int NumExpectedParameters() const { return -1; } \
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
      return context->CurrentTarget->GetMappedConfig(context->Config,
                                                  &loc,
                                                  &imp,
                                                  suffix) ? "1" : "0";
      }
    return "0";
  }
} configurationTestNode;


static const struct TargetDefinedNode : public cmGeneratorExpressionNode
{
  TargetDefinedNode() {}

  virtual int NumExpectedParameters() const { return 1; }

  std::string Evaluate(const std::vector<std::string> &parameters,
                       cmGeneratorExpressionContext *context,
                       const GeneratorExpressionContent *,
                       cmGeneratorExpressionDAGChecker *) const
  {
    return context->Makefile->FindTargetToUse(parameters.front().c_str())
      ? "1" : "0";
  }
} targetDefinedNode;

//----------------------------------------------------------------------------
static const char* targetPropertyTransitiveWhitelist[] = {
    "INTERFACE_INCLUDE_DIRECTORIES"
  , "INTERFACE_COMPILE_DEFINITIONS"
};

//----------------------------------------------------------------------------
static const struct TargetPropertyNode : public cmGeneratorExpressionNode
{
  TargetPropertyNode() {}

  // This node handles errors on parameter count itself.
  virtual int NumExpectedParameters() const { return -1; }

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

    cmGeneratorExpressionDAGChecker dagChecker(context->Backtrace,
                                               target->GetName(),
                                               propertyName,
                                               content,
                                               dagCheckerParent);

    switch (dagChecker.check())
      {
    case cmGeneratorExpressionDAGChecker::SELF_REFERENCE:
      // It would be better to consider it an error for the foo target
      // to have a INTERFACE_INCLUDE_DIRECTORIES which depends directly on its
      // own INTERFACE_INCLUDE_DIRECTORIES property, but as the error of a
      // target having itself in its own LINK_INTERFACE_LIBRARIES is 'allowed'
      // and tested, and as the interface includes and defines are now based
      // on the link interface, it breaks the CMakeOnly.LinkInterfaceLoop test.
//       dagChecker.reportError(context, content->GetOriginalExpression());
      return std::string();
    case cmGeneratorExpressionDAGChecker::CYCLIC_REFERENCE:
      // No error. We just skip cyclic references.
      return std::string();
    case cmGeneratorExpressionDAGChecker::ALREADY_SEEN:
      for (size_t i = 0;
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

    std::string linkedTargetsContent;

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
        assert(dagCheckerParent->EvaluatingIncludeDirectories()
            || dagCheckerParent->EvaluatingCompileDefinitions());

        if (propertyName == "INTERFACE_INCLUDE_DIRECTORIES"
            || propertyName == "INTERFACE_COMPILE_DEFINITIONS")
          {
          const cmTarget::LinkInterface *iface = target->GetLinkInterface(
                                                        context->Config,
                                                        context->HeadTarget);
          if(iface)
            {
            cmGeneratorExpression ge(context->Backtrace);

            std::string sep;
            std::string depString;
            for (std::vector<std::string>::const_iterator
                it = iface->Libraries.begin();
                it != iface->Libraries.end(); ++it)
              {
              if (context->Makefile->FindTargetToUse(it->c_str()))
                {
                depString +=
                  sep + "$<TARGET_PROPERTY:" + *it + "," + propertyName + ">";
                sep = ";";
                }
              }
            cmsys::auto_ptr<cmCompiledGeneratorExpression> cge =
                                                          ge.Parse(depString);
            linkedTargetsContent = cge->Evaluate(context->Makefile,
                                context->Config,
                                context->Quiet,
                                context->HeadTarget,
                                target,
                                &dagChecker);
            if (cge->GetHadContextSensitiveCondition())
              {
              context->HadContextSensitiveCondition = true;
              }
            }
          }
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

    for (size_t i = 0;
         i < (sizeof(targetPropertyTransitiveWhitelist) /
              sizeof(*targetPropertyTransitiveWhitelist));
         ++i)
      {
      if (targetPropertyTransitiveWhitelist[i] == propertyName)
        {
        cmGeneratorExpression ge(context->Backtrace);
        cmsys::auto_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(prop);
        std::string result = cge->Evaluate(context->Makefile,
                            context->Config,
                            context->Quiet,
                            context->HeadTarget,
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

  virtual bool AcceptsSingleArbitraryContentParameter() const { return true; }
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
    "CMP0003"
  , "CMP0004"
  , "CMP0008"
  , "CMP0020"
};

cmPolicies::PolicyStatus statusForTarget(cmTarget *tgt, const char *policy)
{
#define RETURN_POLICY(POLICY) \
  if (strcmp(policy, #POLICY) == 0) \
  { \
    return tgt->GetPolicyStatus ## POLICY (); \
  } \

  RETURN_POLICY(CMP0003)
  RETURN_POLICY(CMP0004)
  RETURN_POLICY(CMP0008)
  RETURN_POLICY(CMP0020)

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

  RETURN_POLICY_ID(CMP0003)
  RETURN_POLICY_ID(CMP0004)
  RETURN_POLICY_ID(CMP0008)
  RETURN_POLICY_ID(CMP0020)

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

    for (size_t i = 0;
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
      "policies.  Currently it may be used with policies CMP0003, CMP0004, "
      "CMP0008 and CMP0020."
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
                       cmGeneratorExpressionDAGChecker *) const
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
    if(target->GetType() >= cmTarget::UTILITY &&
      target->GetType() != cmTarget::UNKNOWN_LIBRARY)
      {
      ::reportError(context, content->GetOriginalExpression(),
                  "Target \"" + name + "\" is not an executable or library.");
      return std::string();
      }
    context->Targets.insert(target);

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
  else if (identifier == "TARGET_DEFINED")
    return &targetDefinedNode;
  else if (identifier == "INSTALL_PREFIX")
    return &installPrefixNode;
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
    if (node->AcceptsSingleArbitraryContentParameter())
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

  if (node->AcceptsSingleArbitraryContentParameter())
    {
    std::string result;
    std::vector<std::vector<cmGeneratorExpressionEvaluator*> >::const_iterator
                                        pit = this->ParamChildren.begin();
    const
    std::vector<std::vector<cmGeneratorExpressionEvaluator*> >::const_iterator
                                        pend = this->ParamChildren.end();
    for ( ; pit != pend; ++pit)
      {
      if (!result.empty())
        {
        result += ",";
        }

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
      }
    if (node->RequiresLiteralInput())
      {
      std::vector<std::string> parameters;
      parameters.push_back(result);
      return node->Evaluate(parameters, context, this, dagChecker);
      }
    return result;
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
  {
  std::vector<std::vector<cmGeneratorExpressionEvaluator*> >::const_iterator
                                        pit = this->ParamChildren.begin();
  const
  std::vector<std::vector<cmGeneratorExpressionEvaluator*> >::const_iterator
                                        pend = this->ParamChildren.end();
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
    }
  }

  int numExpected = node->NumExpectedParameters();
  if ((numExpected != -1 && (unsigned int)numExpected != parameters.size()))
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

  if (numExpected == -1 && parameters.empty())
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
  typedef std::vector<cmGeneratorExpressionToken> TokenVector;
  std::vector<EvaluatorVector>::const_iterator pit =
                                                  this->ParamChildren.begin();
  const std::vector<EvaluatorVector>::const_iterator pend =
                                                  this->ParamChildren.end();
  for ( ; pit != pend; ++pit)
    {
    deleteAll(*pit);
    }
}
