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

//----------------------------------------------------------------------------
static void reportError(cmGeneratorExpressionContext *context,
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

  virtual bool AcceptsSingleArbitraryContentParameter() const
    { return false; }

  virtual int NumExpectedParameters() const { return 1; }

  virtual std::string Evaluate(const std::vector<std::string> &parameters,
                               cmGeneratorExpressionContext *context,
                               const GeneratorExpressionContent *content
                              ) const = 0;
};

//----------------------------------------------------------------------------
static const struct ZeroNode : public cmGeneratorExpressionNode
{
  ZeroNode() {}

  virtual bool GeneratesContent() const { return false; }

  std::string Evaluate(const std::vector<std::string> &,
                       cmGeneratorExpressionContext *,
                       const GeneratorExpressionContent *) const
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
                       const GeneratorExpressionContent *) const
  {
    // Unreachable
    return std::string();
  }
} oneNode;

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
                       const GeneratorExpressionContent *content) const \
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
                       const GeneratorExpressionContent *content) const
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
static const struct ConfigurationNode : public cmGeneratorExpressionNode
{
  ConfigurationNode() {}
  virtual int NumExpectedParameters() const { return 0; }

  std::string Evaluate(const std::vector<std::string> &,
                       cmGeneratorExpressionContext *context,
                       const GeneratorExpressionContent *) const
  {
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
                       const GeneratorExpressionContent *content) const
  {
    if (!context->Config)
      {
      return std::string();
      }

    cmsys::RegularExpression configValidator;
    configValidator.compile("^[A-Za-z0-9_]*$");
    if (!configValidator.find(parameters.begin()->c_str()))
      {
      reportError(context, content->GetOriginalExpression(),
                  "Expression syntax not recognized.");
      return std::string();
      }
    return *parameters.begin() == context->Config ? "1" : "0";
  }
} configurationTestNode;

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
                       const GeneratorExpressionContent *content) const
  {
    // Lookup the referenced target.
    std::string name = *parameters.begin();

    cmsys::RegularExpression targetValidator;
    targetValidator.compile("^[A-Za-z0-9_]+$");
    if (!targetValidator.find(name.c_str()))
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
  if (identifier == "1")
    return &oneNode;
  if (identifier == "AND")
    return &andNode;
  if (identifier == "OR")
    return &orNode;
  if (identifier == "NOT")
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
                                  cmGeneratorExpressionContext *context) const
{
  std::string identifier;
  {
  std::vector<cmGeneratorExpressionEvaluator*>::const_iterator it
                                          = this->IdentifierChildren.begin();
  const std::vector<cmGeneratorExpressionEvaluator*>::const_iterator end
                                          = this->IdentifierChildren.end();
  for ( ; it != end; ++it)
    {
    identifier += (*it)->Evaluate(context);
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
        result += (*it)->Evaluate(context);
        if (context->HadError)
          {
          return std::string();
          }
        }
      }
    return result;
    }

  std::vector<std::string> parameters;
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
      parameter += (*it)->Evaluate(context);
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
    return std::string();
    }

  return node->Evaluate(parameters, context, this);
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
