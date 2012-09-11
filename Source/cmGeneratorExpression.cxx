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
  cmMakefile* mf, const char* config,
  cmListFileBacktrace const& backtrace, bool quiet):
  Makefile(mf), Config(config), Backtrace(backtrace), Quiet(quiet),
  NeedsParsing(true)
{
}

//----------------------------------------------------------------------------
const char* cmGeneratorExpression::Process(std::string const& input)
{
  return this->Process(input.c_str());
}

//----------------------------------------------------------------------------
const char* cmGeneratorExpression::Process(const char* input)
{
  this->Parse(input);
  return this->Evaluate(this->Makefile, this->Config, this->Quiet);
}

//----------------------------------------------------------------------------
void cmGeneratorExpression::Parse(const char* input)
{
  this->Evaluators.clear();

  this->Input = input;
  cmGeneratorExpressionLexer l;
  std::vector<cmGeneratorExpressionToken> tokens = l.Tokenize(this->Input);
  this->NeedsParsing = l.GetSawGeneratorExpression();

  if (!this->NeedsParsing)
    {
    return;
    }

  cmGeneratorExpressionParser p(tokens);
  p.Parse(this->Evaluators);
}

//----------------------------------------------------------------------------
const char *cmGeneratorExpression::Evaluate(
  cmMakefile* mf, const char* config, bool quiet)
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
  context.Backtrace = this->Backtrace;

  for ( ; it != end; ++it)
    {
    this->Output += (*it)->Evaluate(&context);
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

//----------------------------------------------------------------------------
cmGeneratorExpression::~cmGeneratorExpression()
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
