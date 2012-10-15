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
  Backtrace(backtrace), CompiledExpression(0)
{
}

//----------------------------------------------------------------------------
const cmCompiledGeneratorExpression &
cmGeneratorExpression::Parse(std::string const& input)
{
  return this->Parse(input.c_str());
}

//----------------------------------------------------------------------------
const cmCompiledGeneratorExpression &
cmGeneratorExpression::Parse(const char* input)
{
  cmGeneratorExpressionLexer l;
  std::vector<cmGeneratorExpressionToken> tokens = l.Tokenize(input);
  bool needsParsing = l.GetSawGeneratorExpression();
  std::vector<cmGeneratorExpressionEvaluator*> evaluators;

  if (needsParsing)
    {
    cmGeneratorExpressionParser p(tokens);
    p.Parse(evaluators);
    }

  delete this->CompiledExpression;
  this->CompiledExpression = new cmCompiledGeneratorExpression(
                                      this->Backtrace,
                                      evaluators,
                                      input,
                                      needsParsing);
  return *this->CompiledExpression;
}

cmGeneratorExpression::~cmGeneratorExpression()
{
  delete this->CompiledExpression;
}

//----------------------------------------------------------------------------
const char *cmCompiledGeneratorExpression::Evaluate(
  cmMakefile* mf, const char* config, bool quiet,
  cmGeneratorTarget *target,
  cmGeneratorExpressionDAGChecker *dagChecker) const
{
  if (!this->NeedsParsing)
    {
    return this->Input;
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
  context.Target = target;
  context.Backtrace = this->Backtrace;

  for ( ; it != end; ++it)
    {
    this->Output += (*it)->Evaluate(&context, dagChecker);
    if (context.HadError)
      {
      this->Output = "";
      break;
      }
    }

  this->Targets = context.Targets;
  // TODO: Return a std::string from here instead?
  return this->Output.c_str();
}

cmCompiledGeneratorExpression::cmCompiledGeneratorExpression(
              cmListFileBacktrace const& backtrace,
              const std::vector<cmGeneratorExpressionEvaluator*> &evaluators,
              const char *input, bool needsParsing)
  : Backtrace(backtrace), Evaluators(evaluators), Input(input),
    NeedsParsing(needsParsing)
{

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

std::string cmGeneratorExpression::Preprocess(const std::string &input,
                                              PreprocessContext context)
{
  if (context != StripAllGeneratorExpressions)
  {
    assert(!"cmGeneratorExpression::Preprocess called with invalid args");
    return std::string();
  }

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
  return result;
}
