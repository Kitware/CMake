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

#include <cmsys/RegularExpression.hxx>
#include <cmsys/auto_ptr.hxx>

class cmGeneratorTarget;
class cmLocalGenerator;
class cmListFileBacktrace;

struct cmGeneratorExpressionEvaluator;
struct cmGeneratorExpressionContext;
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
  cmGeneratorExpression(
      cmListFileBacktrace const& backtrace = cmListFileBacktrace());
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
                                PreprocessContext context,
                                bool resolveRelative = false);

  static void Split(const std::string &input,
                    std::vector<std::string> &output);

  static std::string::size_type Find(const std::string &input);

  static bool IsValidTargetName(const std::string &input);

  static std::string StripEmptyListElements(const std::string &input);
private:
  cmGeneratorExpression(const cmGeneratorExpression &);
  void operator=(const cmGeneratorExpression &);

  cmListFileBacktrace Backtrace;
};

class cmCompiledGeneratorExpression
{
public:
  const char* Evaluate(cmLocalGenerator* lg, const std::string& config,
                       bool quiet = false,
                       cmGeneratorTarget const* headTarget = 0,
                       cmGeneratorTarget const* currentTarget = 0,
                       cmGeneratorExpressionDAGChecker *dagChecker = 0,
                       std::string const& language = std::string()) const;
  const char* Evaluate(cmLocalGenerator* lg, const std::string& config,
                       bool quiet,
                       cmGeneratorTarget const* headTarget,
                       cmGeneratorExpressionDAGChecker *dagChecker,
                       std::string const& language = std::string()) const;

  /** Get set of targets found during evaluations.  */
  std::set<cmGeneratorTarget*> const& GetTargets() const
    { return this->DependTargets; }

  std::set<std::string> const& GetSeenTargetProperties() const
    { return this->SeenTargetProperties; }

  std::set<cmGeneratorTarget const*> const& GetAllTargetsSeen() const
    { return this->AllTargetsSeen; }

  ~cmCompiledGeneratorExpression();

  std::string const& GetInput() const
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
  bool GetHadHeadSensitiveCondition() const
  {
    return this->HadHeadSensitiveCondition;
  }
  std::set<cmGeneratorTarget const*> GetSourceSensitiveTargets() const
  {
    return this->SourceSensitiveTargets;
  }

  void SetEvaluateForBuildsystem(bool eval)
  {
    this->EvaluateForBuildsystem = eval;
  }

  void GetMaxLanguageStandard(cmGeneratorTarget const* tgt,
                    std::map<std::string, std::string>& mapping);

private:
  const char* EvaluateWithContext(cmGeneratorExpressionContext& context,
                           cmGeneratorExpressionDAGChecker *dagChecker) const;

  cmCompiledGeneratorExpression(cmListFileBacktrace const& backtrace,
              const std::string& input);

  friend class cmGeneratorExpression;

  cmCompiledGeneratorExpression(const cmCompiledGeneratorExpression &);
  void operator=(const cmCompiledGeneratorExpression &);

  cmListFileBacktrace Backtrace;
  std::vector<cmGeneratorExpressionEvaluator*> Evaluators;
  const std::string Input;
  bool NeedsEvaluation;

  mutable std::set<cmGeneratorTarget*> DependTargets;
  mutable std::set<cmGeneratorTarget const*> AllTargetsSeen;
  mutable std::set<std::string> SeenTargetProperties;
  mutable std::map<cmGeneratorTarget const*,
                   std::map<std::string, std::string> > MaxLanguageStandard;
  mutable std::string Output;
  mutable bool HadContextSensitiveCondition;
  mutable bool HadHeadSensitiveCondition;
  mutable std::set<cmGeneratorTarget const*>  SourceSensitiveTargets;
  bool EvaluateForBuildsystem;
};

#endif
