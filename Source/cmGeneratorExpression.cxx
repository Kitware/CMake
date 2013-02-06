/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmGeneratorExpression.h"

#include "cmMakefile.h"
#include "cmTarget.h"
#include "assert.h"

#include <cmsys/String.h>

#include "cmGeneratorExpressionEvaluator.h"
#include "cmGeneratorExpressionLexer.h"
#include "cmGeneratorExpressionParser.h"
#include "cmGeneratorExpressionDAGChecker.h"

//----------------------------------------------------------------------------
cmGeneratorExpression::cmGeneratorExpression(
  cmListFileBacktrace const& backtrace):
  Backtrace(backtrace)
{
}

//----------------------------------------------------------------------------
cmsys::auto_ptr<cmCompiledGeneratorExpression>
cmGeneratorExpression::Parse(std::string const& input)
{
  return this->Parse(input.c_str());
}

//----------------------------------------------------------------------------
cmsys::auto_ptr<cmCompiledGeneratorExpression>
cmGeneratorExpression::Parse(const char* input)
{
  return cmsys::auto_ptr<cmCompiledGeneratorExpression>(
                                      new cmCompiledGeneratorExpression(
                                        this->Backtrace,
                                        input));
}

cmGeneratorExpression::~cmGeneratorExpression()
{
}

//----------------------------------------------------------------------------
const char *cmCompiledGeneratorExpression::Evaluate(
  cmMakefile* mf, const char* config, bool quiet,
  cmTarget *headTarget,
  cmGeneratorExpressionDAGChecker *dagChecker) const
{
  return this->Evaluate(mf,
                        config,
                        quiet,
                        headTarget,
                        headTarget,
                        dagChecker);
}

//----------------------------------------------------------------------------
const char *cmCompiledGeneratorExpression::Evaluate(
  cmMakefile* mf, const char* config, bool quiet,
  cmTarget *headTarget,
  cmTarget *currentTarget,
  cmGeneratorExpressionDAGChecker *dagChecker) const
{
  if (!this->NeedsParsing)
    {
    return this->Input.c_str();
    }

  this->Output = "";

  std::vector<cmGeneratorExpressionEvaluator*>::const_iterator it
                                                  = this->Evaluators.begin();
  const std::vector<cmGeneratorExpressionEvaluator*>::const_iterator end
                                                  = this->Evaluators.end();

  cmGeneratorExpressionContext context;
  context.Makefile = mf;
  context.Config = config;
  context.Quiet = quiet;
  context.HadError = false;
  context.HadContextSensitiveCondition = false;
  context.HeadTarget = headTarget;
  context.CurrentTarget = currentTarget ? currentTarget : headTarget;
  context.Backtrace = this->Backtrace;

  for ( ; it != end; ++it)
    {
    const std::string result = (*it)->Evaluate(&context, dagChecker);
    this->Output += result;

    for(std::set<cmStdString>::const_iterator
          p = context.SeenTargetProperties.begin();
          p != context.SeenTargetProperties.end(); ++p)
      {
      this->SeenTargetProperties[*p] += result + ";";
      }
    if (context.HadError)
      {
      this->Output = "";
      break;
      }
    }
  if (!context.HadError)
    {
    this->HadContextSensitiveCondition = context.HadContextSensitiveCondition;
    }

  this->Targets = context.Targets;
  // TODO: Return a std::string from here instead?
  return this->Output.c_str();
}

cmCompiledGeneratorExpression::cmCompiledGeneratorExpression(
              cmListFileBacktrace const& backtrace,
              const char *input)
  : Backtrace(backtrace), Input(input ? input : ""),
    HadContextSensitiveCondition(false)
{
  cmGeneratorExpressionLexer l;
  std::vector<cmGeneratorExpressionToken> tokens =
                                              l.Tokenize(this->Input.c_str());
  this->NeedsParsing = l.GetSawGeneratorExpression();

  if (this->NeedsParsing)
    {
    cmGeneratorExpressionParser p(tokens);
    p.Parse(this->Evaluators);
    }
}


//----------------------------------------------------------------------------
cmCompiledGeneratorExpression::~cmCompiledGeneratorExpression()
{
  std::vector<cmGeneratorExpressionEvaluator*>::const_iterator it
                                                  = this->Evaluators.begin();
  const std::vector<cmGeneratorExpressionEvaluator*>::const_iterator end
                                                  = this->Evaluators.end();

  for ( ; it != end; ++it)
    {
    delete *it;
    }
}

//----------------------------------------------------------------------------
static std::string stripEmptyListElements(const std::string &input)
{
  std::string result;

  const char *c = input.c_str();
  bool skipSemiColons = true;
  for ( ; *c; ++c)
    {
    if(c[0] == ';')
      {
      if(skipSemiColons)
        {
        continue;
        }
      skipSemiColons = true;
      }
    else
      {
      skipSemiColons = false;
      }
    result += *c;
    }

  if (!result.empty() && *(result.end() - 1) == ';')
    {
    result.resize(result.size() - 1);
    }

  return result;
}

//----------------------------------------------------------------------------
static std::string stripAllGeneratorExpressions(const std::string &input)
{
  std::string result;
  std::string::size_type pos = 0;
  std::string::size_type lastPos = pos;
  while((pos = input.find("$<", lastPos)) != input.npos)
    {
    result += input.substr(lastPos, pos - lastPos);
    pos += 2;
    int nestingLevel = 1;
    const char *c = input.c_str() + pos;
    const char * const cStart = c;
    for ( ; *c; ++c)
      {
      if(c[0] == '$' && c[1] == '<')
        {
        ++nestingLevel;
        ++c;
        continue;
        }
      if(c[0] == '>')
        {
        --nestingLevel;
        if (nestingLevel == 0)
          {
          break;
          }
        }
      }
    const std::string::size_type traversed = (c - cStart) + 1;
    if (!*c)
      {
      result += "$<" + input.substr(pos, traversed);
      }
    pos += traversed;
    lastPos = pos;
    }
  result += input.substr(lastPos);
  return stripEmptyListElements(result);
}

//----------------------------------------------------------------------------
static std::string stripExportInterface(const std::string &input,
                          cmGeneratorExpression::PreprocessContext context)
{
  std::string result;

  std::string::size_type pos = 0;
  std::string::size_type lastPos = pos;
  while((pos = input.find("$<BUILD_INTERFACE:", lastPos)) != input.npos
    || (pos = input.find("$<INSTALL_INTERFACE:", lastPos)) != input.npos)
    {
    result += input.substr(lastPos, pos - lastPos);
    const bool gotInstallInterface = input[pos + 2] == 'I';
    pos += gotInstallInterface ? sizeof("$<INSTALL_INTERFACE:") - 1
                               : sizeof("$<BUILD_INTERFACE:") - 1;
    int nestingLevel = 1;
    const char *c = input.c_str() + pos;
    const char * const cStart = c;
    for ( ; *c; ++c)
      {
      if(c[0] == '$' && c[1] == '<')
        {
        ++nestingLevel;
        ++c;
        continue;
        }
      if(c[0] == '>')
        {
        --nestingLevel;
        if (nestingLevel != 0)
          {
          continue;
          }
        if(context == cmGeneratorExpression::BuildInterface
            && !gotInstallInterface)
          {
          result += input.substr(pos, c - cStart);
          }
        else if(context == cmGeneratorExpression::InstallInterface
            && gotInstallInterface)
          {
          result += input.substr(pos, c - cStart);
          }
        break;
        }
      }
    const std::string::size_type traversed = (c - cStart) + 1;
    if (!*c)
      {
      result += std::string(gotInstallInterface ? "$<INSTALL_INTERFACE:"
                                                : "$<BUILD_INTERFACE:")
             + input.substr(pos, traversed);
      }
    pos += traversed;
    lastPos = pos;
    }
  result += input.substr(lastPos);

  return stripEmptyListElements(result);
}

//----------------------------------------------------------------------------
void cmGeneratorExpression::Split(const std::string &input,
                                  std::vector<std::string> &output)
{
  std::string::size_type pos = 0;
  std::string::size_type lastPos = pos;
  while((pos = input.find("$<", lastPos)) != input.npos)
    {
    std::string part = input.substr(lastPos, pos - lastPos);
    std::string preGenex;
    if (!part.empty())
      {
      std::string::size_type startPos = input.rfind(";", pos);
      if (startPos != pos - 1 && startPos >= lastPos)
        {
        part = input.substr(lastPos, startPos - lastPos);
        preGenex = input.substr(startPos + 1, pos - startPos - 1);
        }
      cmSystemTools::ExpandListArgument(part.c_str(), output);
      }
    pos += 2;
    int nestingLevel = 1;
    const char *c = input.c_str() + pos;
    const char * const cStart = c;
    for ( ; *c; ++c)
      {
      if(c[0] == '$' && c[1] == '<')
        {
        ++nestingLevel;
        ++c;
        continue;
        }
      if(c[0] == '>')
        {
        --nestingLevel;
        if (nestingLevel == 0)
          {
          break;
          }
        }
      }
    for ( ; *c; ++c)
      {
      // Capture the part after the genex and before the next ';'
      if(c[0] == ';')
        {
        --c;
        break;
        }
      }
    const std::string::size_type traversed = (c - cStart) + 1;
    output.push_back(preGenex + "$<" + input.substr(pos, traversed));
    pos += traversed;
    lastPos = pos;
    }
  if (lastPos < input.size())
    {
    cmSystemTools::ExpandListArgument(input.substr(lastPos), output);
    }
}

//----------------------------------------------------------------------------
std::string cmGeneratorExpression::Preprocess(const std::string &input,
                                              PreprocessContext context)
{
  if (context == StripAllGeneratorExpressions)
    {
    return stripAllGeneratorExpressions(input);
    }
  else if (context == BuildInterface || context == InstallInterface)
    {
    return stripExportInterface(input, context);
    }

  assert(!"cmGeneratorExpression::Preprocess called with invalid args");
  return std::string();
}

//----------------------------------------------------------------------------
std::string::size_type cmGeneratorExpression::Find(const std::string &input)
{
  const std::string::size_type openpos = input.find("$<");
  if (openpos != std::string::npos
        && input.find(">", openpos) != std::string::npos)
      {
      return openpos;
      }
    }
  return std::string::npos;
}
