/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmGeneratorExpression_h
#define cmGeneratorExpression_h

#include "cmStandardIncludes.h"
#include "cmListFileCache.h"

#include <stack>

#include <cmsys/RegularExpression.hxx>
#include <cmsys/auto_ptr.hxx>

class cmTarget;
class cmMakefile;
class cmListFileBacktrace;

struct cmGeneratorExpressionEvaluator;
struct cmGeneratorExpressionDAGChecker;

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
  ~cmGeneratorExpression();

  cmsys::auto_ptr<cmCompiledGeneratorExpression> Parse(
                                                std::string const& input);
  cmsys::auto_ptr<cmCompiledGeneratorExpression> Parse(const char* input);

  enum PreprocessContext {
    StripAllGeneratorExpressions,
    BuildInterface,
    InstallInterface
  };

  static std::string Preprocess(const std::string &input,
                                PreprocessContext context);

  static void Split(const std::string &input,
                    std::vector<std::string> &output);

private:
  cmGeneratorExpression(const cmGeneratorExpression &);
  void operator=(const cmGeneratorExpression &);

  cmListFileBacktrace const& Backtrace;
};

class cmCompiledGeneratorExpression
{
public:
  const char* Evaluate(cmMakefile* mf, const char* config,
                       bool quiet = false,
                       cmTarget *headTarget = 0,
                       cmTarget *currentTarget = 0,
                       cmGeneratorExpressionDAGChecker *dagChecker = 0) const;
  const char* Evaluate(cmMakefile* mf, const char* config,
                       bool quiet,
                       cmTarget *headTarget,
                       cmGeneratorExpressionDAGChecker *dagChecker) const;

  /** Get set of targets found during evaluations.  */
  std::set<cmTarget*> const& GetTargets() const
    { return this->Targets; }

  std::map<cmStdString, cmStdString> const& GetSeenTargetProperties() const
    { return this->SeenTargetProperties; }

  ~cmCompiledGeneratorExpression();

  std::string GetInput() const
  {
    return this->Input;
  }

  cmListFileBacktrace GetBacktrace() const
  {
    return this->Backtrace;
  }
  bool GetHadContextSensitiveCondition() const
  {
    return this->HadContextSensitiveCondition;
  }

private:
  cmCompiledGeneratorExpression(cmListFileBacktrace const& backtrace,
              const char *input);

  friend class cmGeneratorExpression;

  cmCompiledGeneratorExpression(const cmCompiledGeneratorExpression &);
  void operator=(const cmCompiledGeneratorExpression &);

  cmListFileBacktrace Backtrace;
  std::vector<cmGeneratorExpressionEvaluator*> Evaluators;
  const std::string Input;
  bool NeedsParsing;

  mutable std::set<cmTarget*> Targets;
  mutable std::map<cmStdString, cmStdString> SeenTargetProperties;
  mutable std::string Output;
  mutable bool HadContextSensitiveCondition;
};

#endif
