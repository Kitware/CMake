/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmGeneratorExpression_h
#define cmGeneratorExpression_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "cmListFileCache.h"

class cmCompiledGeneratorExpression;
class cmGeneratorTarget;
class cmLocalGenerator;
struct cmGeneratorExpressionContext;
struct cmGeneratorExpressionDAGChecker;
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
  /** Construct. */
  cmGeneratorExpression(cmListFileBacktrace backtrace = cmListFileBacktrace());
  ~cmGeneratorExpression();

  cmGeneratorExpression(cmGeneratorExpression const&) = delete;
  cmGeneratorExpression& operator=(cmGeneratorExpression const&) = delete;

  std::unique_ptr<cmCompiledGeneratorExpression> Parse(
    std::string input) const;
  std::unique_ptr<cmCompiledGeneratorExpression> Parse(
    const char* input) const;

  static std::string Evaluate(
    std::string input, cmLocalGenerator* lg, const std::string& config,
    cmGeneratorTarget const* headTarget = nullptr,
    cmGeneratorExpressionDAGChecker* dagChecker = nullptr,
    cmGeneratorTarget const* currentTarget = nullptr,
    std::string const& language = std::string());
  static std::string Evaluate(
    const char* input, cmLocalGenerator* lg, const std::string& config,
    cmGeneratorTarget const* headTarget = nullptr,
    cmGeneratorExpressionDAGChecker* dagChecker = nullptr,
    cmGeneratorTarget const* currentTarget = nullptr,
    std::string const& language = std::string());

  enum PreprocessContext
  {
    StripAllGeneratorExpressions,
    BuildInterface,
    InstallInterface
  };

  static std::string Preprocess(const std::string& input,
                                PreprocessContext context,
                                bool resolveRelative = false);

  static void Split(const std::string& input,
                    std::vector<std::string>& output);

  static std::string::size_type Find(const std::string& input);

  static bool IsValidTargetName(const std::string& input);

  static std::string StripEmptyListElements(const std::string& input);

  static inline bool StartsWithGeneratorExpression(const std::string& input)
  {
    return input.length() >= 2 && input[0] == '$' && input[1] == '<';
  }
  static inline bool StartsWithGeneratorExpression(const char* input)
  {
    return input != nullptr && input[0] == '$' && input[1] == '<';
  }

  static void ReplaceInstallPrefix(std::string& input,
                                   const std::string& replacement);

private:
  cmListFileBacktrace Backtrace;
};

class cmCompiledGeneratorExpression
{
public:
  ~cmCompiledGeneratorExpression();

  cmCompiledGeneratorExpression(cmCompiledGeneratorExpression const&) = delete;
  cmCompiledGeneratorExpression& operator=(
    cmCompiledGeneratorExpression const&) = delete;

  const std::string& Evaluate(
    cmLocalGenerator* lg, const std::string& config,
    cmGeneratorTarget const* headTarget = nullptr,
    cmGeneratorExpressionDAGChecker* dagChecker = nullptr,
    cmGeneratorTarget const* currentTarget = nullptr,
    std::string const& language = std::string()) const;

  /** Get set of targets found during evaluations.  */
  std::set<cmGeneratorTarget*> const& GetTargets() const
  {
    return this->DependTargets;
  }

  std::set<std::string> const& GetSeenTargetProperties() const
  {
    return this->SeenTargetProperties;
  }

  std::set<cmGeneratorTarget const*> const& GetAllTargetsSeen() const
  {
    return this->AllTargetsSeen;
  }

  std::string const& GetInput() const { return this->Input; }

  cmListFileBacktrace GetBacktrace() const { return this->Backtrace; }
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

  void SetQuiet(bool quiet) { this->Quiet = quiet; }

  void GetMaxLanguageStandard(cmGeneratorTarget const* tgt,
                              std::map<std::string, std::string>& mapping);

private:
  const std::string& EvaluateWithContext(
    cmGeneratorExpressionContext& context,
    cmGeneratorExpressionDAGChecker* dagChecker) const;

  cmCompiledGeneratorExpression(cmListFileBacktrace backtrace,
                                std::string input);

  friend class cmGeneratorExpression;

  cmListFileBacktrace Backtrace;
  std::vector<std::unique_ptr<cmGeneratorExpressionEvaluator>> Evaluators;
  const std::string Input;
  bool NeedsEvaluation;
  bool EvaluateForBuildsystem;
  bool Quiet;

  mutable std::set<cmGeneratorTarget*> DependTargets;
  mutable std::set<cmGeneratorTarget const*> AllTargetsSeen;
  mutable std::set<std::string> SeenTargetProperties;
  mutable std::map<cmGeneratorTarget const*,
                   std::map<std::string, std::string>>
    MaxLanguageStandard;
  mutable std::string Output;
  mutable bool HadContextSensitiveCondition;
  mutable bool HadHeadSensitiveCondition;
  mutable std::set<cmGeneratorTarget const*> SourceSensitiveTargets;
};

class cmGeneratorExpressionInterpreter
{
public:
  cmGeneratorExpressionInterpreter(cmLocalGenerator* localGenerator,
                                   std::string config,
                                   cmGeneratorTarget const* headTarget,
                                   std::string language = std::string())
    : LocalGenerator(localGenerator)
    , Config(std::move(config))
    , HeadTarget(headTarget)
    , Language(std::move(language))
  {
  }

  cmGeneratorExpressionInterpreter(cmGeneratorExpressionInterpreter const&) =
    delete;
  cmGeneratorExpressionInterpreter& operator=(
    cmGeneratorExpressionInterpreter const&) = delete;

  const std::string& Evaluate(std::string expression,
                              const std::string& property);
  const std::string& Evaluate(const char* expression,
                              const std::string& property);

protected:
  cmGeneratorExpression GeneratorExpression;
  std::unique_ptr<cmCompiledGeneratorExpression> CompiledGeneratorExpression;
  cmLocalGenerator* LocalGenerator = nullptr;
  std::string Config;
  cmGeneratorTarget const* HeadTarget = nullptr;
  std::string Language;
};

#endif
