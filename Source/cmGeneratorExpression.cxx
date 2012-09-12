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

#include <cmsys/String.h>

#include "cmGeneratorExpressionEvaluator.h"
#include "cmGeneratorExpressionLexer.h"
#include "cmGeneratorExpressionParser.h"

//----------------------------------------------------------------------------
cmGeneratorExpression::cmGeneratorExpression(
  cmListFileBacktrace const& backtrace):
  Backtrace(backtrace)
{
}

//----------------------------------------------------------------------------
const cmCompiledGeneratorExpression
cmGeneratorExpression::Parse(std::string const& input)
{
  return this->Parse(input.c_str());
}

//----------------------------------------------------------------------------
const cmCompiledGeneratorExpression
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

  return cmCompiledGeneratorExpression(this->Backtrace, evaluators, input,
                                       needsParsing);
}

//----------------------------------------------------------------------------
const char *cmCompiledGeneratorExpression::Evaluate(
  cmMakefile* mf, const char* config, bool quiet) const
{
  if (!this->NeedsParsing)
    {
    return this->Input;
    }

  this->Output.clear();

  std::vector<cmGeneratorExpressionEvaluator*>::const_iterator it
                                                  = this->Evaluators.begin();
  const std::vector<cmGeneratorExpressionEvaluator*>::const_iterator end
                                                  = this->Evaluators.end();

  cmGeneratorExpressionContext context;
  context.Makefile = mf;
  context.Config = config;
  context.Quiet = quiet;
  context.HadError = false;
  context.Backtrace = this->Backtrace;

  for ( ; it != end; ++it)
    {
    this->Output += (*it)->Evaluate(&context);
    if (context.HadError)
      {
      this->Output.clear();
      break;
      }
    }

  this->Targets = context.Targets;
  // TODO: Return a std::string from here instead?
  return this->Output.c_str();
}

cmCompiledGeneratorExpression::cmCompiledGeneratorExpression(
                      cmListFileBacktrace const& backtrace,
                      std::vector<cmGeneratorExpressionEvaluator*> evaluators,
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
