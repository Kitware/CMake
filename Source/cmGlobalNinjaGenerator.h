/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2011 Peter Collingbourne <peter@pcc.me.uk>
  Copyright 2011 Nicolas Despres <nicolas.despres@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmGlobalNinjaGenerator_h
#  define cmGlobalNinjaGenerator_h

#  include "cmGlobalGenerator.h"
#  include "cmNinjaTypes.h"

//#define NINJA_GEN_VERBOSE_FILES

class cmLocalGenerator;
class cmGeneratedFileStream;
class cmGeneratorTarget;

/**
 * \class cmGlobalNinjaGenerator
 * \brief Write a build.ninja file.
 *
 * The main differences between this generator and the UnixMakefile
 * generator family are:
 * - We don't care about VERBOSE variable or RULE_MESSAGES property since
 *   it is handle by Ninja's -v option.
 * - We don't care about computing any progress status since Ninja manages
 *   it itself.
 * - We don't care about generating a clean target since Ninja already have
 *   a clean tool.
 * - We generate one build.ninja and one rules.ninja per project.
 * - We try to minimize the number of generated rules: one per target and
 *   language.
 * - We use Ninja special variable $in and $out to produce nice output.
 * - We extensively use Ninja variable overloading system to minimize the
 *   number of generated rules.
 */
class cmGlobalNinjaGenerator : public cmGlobalGenerator
{
public:
  /// The default name of Ninja's build file. Typically: build.ninja.
  static const char* NINJA_BUILD_FILE;

  /// The default name of Ninja's rules file. Typically: rules.ninja.
  /// It is included in the main build.ninja file.
  static const char* NINJA_RULES_FILE;

  /// The indentation string used when generating Ninja's build file.
  static const char* INDENT;

  /// Write @a count times INDENT level to output stream @a os.
  static void Indent(std::ostream& os, int count);

  /// Write a divider in the given output stream @a os.
  static void WriteDivider(std::ostream& os);

  static std::string EncodeIdent(const std::string &ident, std::ostream &vars);
  static std::string EncodeLiteral(const std::string &lit);
  static std::string EncodePath(const std::string &path);

  /**
   * Write the given @a comment to the output stream @a os. It
   * handles new line character properly.
   */
  static void WriteComment(std::ostream& os, const std::string& comment);

  /**
   * Write a build statement to @a os with the @a comment using
   * the @a rule the list of @a outputs files and inputs.
   * It also writes the variables bound to this build statement.
   * @warning no escaping of any kind is done here.
   */
  static void WriteBuild(std::ostream& os,
                         const std::string& comment,
                         const std::string& rule,
                         const cmNinjaDeps& outputs,
                         const cmNinjaDeps& explicitDeps,
                         const cmNinjaDeps& implicitDeps,
                         const cmNinjaDeps& orderOnlyDeps,
                         const cmNinjaVars& variables,
                         int cmdLineLimit = -1);

  /**
   * Helper to write a build statement with the special 'phony' rule.
   */
  static void WritePhonyBuild(std::ostream& os,
                              const std::string& comment,
                              const cmNinjaDeps& outputs,
                              const cmNinjaDeps& explicitDeps,
                              const cmNinjaDeps& implicitDeps = cmNinjaDeps(),
                              const cmNinjaDeps& orderOnlyDeps = cmNinjaDeps(),
                              const cmNinjaVars& variables = cmNinjaVars());

  void WriteCustomCommandBuild(const std::string& command,
                               const std::string& description,
                               const std::string& comment,
                               const cmNinjaDeps& outputs,
                               const cmNinjaDeps& deps = cmNinjaDeps(),
                             const cmNinjaDeps& orderOnlyDeps = cmNinjaDeps());
  void WriteMacOSXContentBuild(const std::string& input,
                               const std::string& output);

  /**
   * Write a rule statement named @a name to @a os with the @a comment,
   * the mandatory @a command, the @a depfile and the @a description.
   * It also writes the variables bound to this rule statement.
   * @warning no escaping of any kind is done here.
   */
  static void WriteRule(std::ostream& os,
                        const std::string& name,
                        const std::string& command,
                        const std::string& description,
                        const std::string& comment,
                        const std::string& depfile,
                        const std::string& rspfile,
                        const std::string& rspcontent,
                        bool restat,
                        bool generator);

  /**
   * Write a variable named @a name to @a os with value @a value and an
   * optional @a comment. An @a indent level can be specified.
   * @warning no escaping of any kind is done here.
   */
  static void WriteVariable(std::ostream& os,
                            const std::string& name,
                            const std::string& value,
                            const std::string& comment = "",
                            int indent = 0);

  /**
   * Write an include statement including @a filename with an optional
   * @a comment to the @a os stream.
   */
  static void WriteInclude(std::ostream& os,
                           const std::string& filename,
                           const std::string& comment = "");

  /**
   * Write a default target statement specifying @a targets as
   * the default targets.
   */
  static void WriteDefault(std::ostream& os,
                           const cmNinjaDeps& targets,
                           const std::string& comment = "");


  static bool IsMinGW() { return UsingMinGW; }


public:
  /// Default constructor.
  cmGlobalNinjaGenerator();

  /// Convenience method for creating an instance of this class.
  static cmGlobalGenerator* New() {
    return new cmGlobalNinjaGenerator; }

  /// Destructor.
  virtual ~cmGlobalNinjaGenerator() { }

  /// Overloaded methods. @see cmGlobalGenerator::CreateLocalGenerator()
  virtual cmLocalGenerator* CreateLocalGenerator();

  /// Overloaded methods. @see cmGlobalGenerator::GetName().
  virtual const char* GetName() const {
    return cmGlobalNinjaGenerator::GetActualName(); }

  /// @return the name of this generator.
  static const char* GetActualName() { return "Ninja"; }

  /// Overloaded methods. @see cmGlobalGenerator::GetDocumentation()
  virtual void GetDocumentation(cmDocumentationEntry& entry) const;

  /// Overloaded methods. @see cmGlobalGenerator::Generate()
  virtual void Generate();

  /// Overloaded methods. @see cmGlobalGenerator::EnableLanguage()
  virtual void EnableLanguage(std::vector<std::string>const& languages,
                              cmMakefile* mf,
                              bool optional);

  /// Overloaded methods. @see cmGlobalGenerator::GenerateBuildCommand()
  virtual std::string GenerateBuildCommand(const char* makeProgram,
                                           const char* projectName,
                                           const char* additionalOptions,
                                           const char* targetName,
                                           const char* config,
                                           bool ignoreErrors,
                                           bool fast);

  // Setup target names
  virtual const char* GetAllTargetName()           const { return "all"; }
  virtual const char* GetInstallTargetName()       const { return "install"; }
  virtual const char* GetInstallLocalTargetName()  const {
    return "install/local";
  }
  virtual const char* GetInstallStripTargetName()  const {
    return "install/strip";
  }
  virtual const char* GetTestTargetName()          const { return "test"; }
  virtual const char* GetPackageTargetName()       const { return "package"; }
  virtual const char* GetPackageSourceTargetName() const {
    return "package_source";
  }
  virtual const char* GetEditCacheTargetName()     const {
    return "edit_cache";
  }
  virtual const char* GetRebuildCacheTargetName()  const {
    return "rebuild_cache";
  }
  virtual const char* GetCleanTargetName()         const { return "clean"; }


  cmGeneratedFileStream* GetBuildFileStream() const {
    return this->BuildFileStream; }

  cmGeneratedFileStream* GetRulesFileStream() const {
    return this->RulesFileStream; }

  void AddCXXCompileCommand(const std::string &commandLine,
                            const std::string &sourceFile);

  /**
   * Add a rule to the generated build system.
   * Call WriteRule() behind the scene but perform some check before like:
   * - Do not add twice the same rule.
   */
  void AddRule(const std::string& name,
               const std::string& command,
               const std::string& description,
               const std::string& comment,
               const std::string& depfile = "",
               const std::string& rspfile = "",
               const std::string& rspcontent = "",
               bool restat = false,
               bool generator = false);

  bool HasRule(const std::string& name);

  void AddCustomCommandRule();
  void AddMacOSXContentRule();

  bool HasCustomCommandOutput(const std::string &output) {
    return this->CustomCommandOutputs.find(output) !=
           this->CustomCommandOutputs.end();
  }

  /// Called when we have seen the given custom command.  Returns true
  /// if we has seen it before.
  bool SeenCustomCommand(cmCustomCommand const *cc) {
    return !this->CustomCommands.insert(cc).second;
  }

  /// Called when we have seen the given custom command output.
  void SeenCustomCommandOutput(const std::string &output) {
    this->CustomCommandOutputs.insert(output);
    // We don't need the assumed dependencies anymore, because we have
    // an output.
    this->AssumedSourceDependencies.erase(output);
  }

  void AddAssumedSourceDependencies(const std::string &source,
                                    const cmNinjaDeps &deps) {
    std::set<std::string> &ASD = this->AssumedSourceDependencies[source];
    // Because we may see the same source file multiple times (same source
    // specified in multiple targets), compute the union of any assumed
    // dependencies.
    ASD.insert(deps.begin(), deps.end());
  }

  void AppendTargetOutputs(cmTarget* target, cmNinjaDeps& outputs);
  void AppendTargetDepends(cmTarget* target, cmNinjaDeps& outputs);
  void AddDependencyToAll(cmTarget* target);
  void AddDependencyToAll(const std::string& input);

  const std::vector<cmLocalGenerator*>& GetLocalGenerators() const {
    return LocalGenerators; }

  bool IsExcluded(cmLocalGenerator* root, cmTarget& target) {
    return cmGlobalGenerator::IsExcluded(root, target); }

  int GetRuleCmdLength(const std::string& name) {
    return RuleCmdLength[name]; }

  void AddTargetAlias(const std::string& alias, cmTarget* target);


protected:

  /// Overloaded methods.
  /// @see cmGlobalGenerator::CheckALLOW_DUPLICATE_CUSTOM_TARGETS()
  virtual bool CheckALLOW_DUPLICATE_CUSTOM_TARGETS() { return true; }


private:

  /// @see cmGlobalGenerator::ComputeTargetObjects
  virtual void ComputeTargetObjects(cmGeneratorTarget* gt) const;

  void OpenBuildFileStream();
  void CloseBuildFileStream();

  void CloseCompileCommandsStream();

  void OpenRulesFileStream();
  void CloseRulesFileStream();

  /// Write the common disclaimer text at the top of each build file.
  void WriteDisclaimer(std::ostream& os);

  void WriteAssumedSourceDependencies();

  void WriteTargetAliases(std::ostream& os);

  void WriteBuiltinTargets(std::ostream& os);
  void WriteTargetAll(std::ostream& os);
  void WriteTargetRebuildManifest(std::ostream& os);
  void WriteTargetClean(std::ostream& os);
  void WriteTargetHelp(std::ostream& os);

  std::string ninjaCmd() const;


  /// The file containing the build statement. (the relation ship of the
  /// compilation DAG).
  cmGeneratedFileStream* BuildFileStream;
  /// The file containing the rule statements. (The action attached to each
  /// edge of the compilation DAG).
  cmGeneratedFileStream* RulesFileStream;
  cmGeneratedFileStream* CompileCommandsStream;

  /// The type used to store the set of rules added to the generated build
  /// system.
  typedef std::set<std::string> RulesSetType;

  /// The set of rules added to the generated build system.
  RulesSetType Rules;

  /// Length of rule command, used by rsp file evaluation
  std::map<std::string, int> RuleCmdLength;

  /// The set of dependencies to add to the "all" target.
  cmNinjaDeps AllDependencies;

  /// The set of custom commands we have seen.
  std::set<cmCustomCommand const*> CustomCommands;

  /// The set of custom command outputs we have seen.
  std::set<std::string> CustomCommandOutputs;

  /// The mapping from source file to assumed dependencies.
  std::map<std::string, std::set<std::string> > AssumedSourceDependencies;

  typedef std::map<std::string, cmTarget*> TargetAliasMap;
  TargetAliasMap TargetAliases;

  static cmLocalGenerator* LocalGenerator;

  static bool UsingMinGW;

};

#endif // ! cmGlobalNinjaGenerator_h
