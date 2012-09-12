/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmStandardIncludes.h"

#include <stack>

#include <cmsys/RegularExpression.hxx>

class cmTarget;
class cmMakefile;
class cmListFileBacktrace;

class cmGeneratorExpressionEvaluator;

class cmCompiledGeneratorExpression;

/** \class cmGeneratorExpression
 * \brief Evaluate generate-time query expression syntax.
 *
 * cmGeneratorExpression instances are used by build system generator
 * implementations to evaluate the $<> generator expression syntax.
 * Generator expressions are evaluated just before the generate step
 * writes strings into the build system.  They have knowledge of the
 * build configuration which is not available at configure time.
 */
class cmGeneratorExpression
{
public:
  /** Construct. */
  cmGeneratorExpression(cmListFileBacktrace const& backtrace);

  const cmCompiledGeneratorExpression Parse(std::string const& input);
  const cmCompiledGeneratorExpression Parse(const char* input);

private:
  cmListFileBacktrace const& Backtrace;
};

class cmCompiledGeneratorExpression
{
public:
  const char* Evaluate(cmMakefile* mf, const char* config,
                        bool quiet = false) const;

  /** Get set of targets found during evaluations.  */
  std::set<cmTarget*> const& GetTargets() const
    { return this->Targets; }

  ~cmCompiledGeneratorExpression();

private:
  cmCompiledGeneratorExpression(cmListFileBacktrace const& backtrace,
                      std::vector<cmGeneratorExpressionEvaluator*> evaluators,
                      const char *input, bool needsParsing);

  friend class cmGeneratorExpression;

private:
  const std::vector<cmGeneratorExpressionEvaluator*> Evaluators;
  cmListFileBacktrace const& Backtrace;

  mutable std::set<cmTarget*> Targets;
  const char* const Input;
  const bool NeedsParsing;

  mutable std::string Output;
};
