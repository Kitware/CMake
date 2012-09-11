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

struct cmGeneratorExpressionEvaluator;

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
  /** Construct with an evaluation context and configuration.  */
  cmGeneratorExpression(cmMakefile* mf, const char* config,
                        cmListFileBacktrace const& backtrace,
                        bool quiet = false);

  ~cmGeneratorExpression();

  /** Evaluate generator expressions in a string.  */
  const char* Process(std::string const& input);
  const char* Process(const char* input);

  void Parse(const char* input);
  const char* Evaluate(cmMakefile* mf, const char* config,
                        bool quiet = false);

  /** Get set of targets found during evaluations.  */
  std::set<cmTarget*> const& GetTargets() const
    { return this->Targets; }
private:
  std::vector<cmGeneratorExpressionEvaluator*> Evaluators;
  cmMakefile* Makefile;
  const char* Config;
  cmListFileBacktrace const& Backtrace;
  bool Quiet;

  std::set<cmTarget*> Targets;
  const char* Input;
  bool NeedsParsing;

  std::string Output;
};
