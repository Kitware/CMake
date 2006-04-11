/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmLocalGenerator_h
#define cmLocalGenerator_h

#include "cmStandardIncludes.h"

class cmMakefile;
class cmGlobalGenerator;
class cmTarget;
class cmTargetManifest;
class cmSourceFile;
class cmCustomCommand;

/** \class cmLocalGenerator
 * \brief Create required build files for a directory.
 *
 * Subclasses of this abstract class generate makefiles, DSP, etc for various
 * platforms. This class should never be constructued directly. A
 * GlobalGenerator will create it and invoke the appropriate commands on it.
 */
class cmLocalGenerator
{
public:
  cmLocalGenerator();
  virtual ~cmLocalGenerator();
  
  /**
   * Generate the makefile for this directory. 
   */
  virtual void Generate() {};

  /**
   * Process the CMakeLists files for this directory to fill in the
   * Makefile ivar 
   */
  virtual void Configure();

  /**
   * Perform any final calculations prior to generation
   */
  virtual void ConfigureFinalPass();

  /**
   * Generate the install rules files in this directory.
   */
  virtual void GenerateInstallRules();

  /**
   * Generate the test files for tests.
   */
  virtual void GenerateTestFiles();

  /**
   * Generate a manifest of target files that will be built.
   */
  virtual void GenerateTargetManifest(cmTargetManifest&);

  ///! Get the makefile for this generator
  cmMakefile *GetMakefile() {
    return this->Makefile; };
  
  ///! Get the GlobalGenerator this is associated with
  cmGlobalGenerator *GetGlobalGenerator() {
    return this->GlobalGenerator; };

  ///! Set the Global Generator, done on creation by the GlobalGenerator
  void SetGlobalGenerator(cmGlobalGenerator *gg);

  /** 
   * Convert something to something else. This is a centralized coversion
   * routine used by the generators to handle relative paths and the like.
   * The flags determine what is actually done. 
   *
   * relative: treat the argument as a directory and convert it to make it
   * relative or full or unchanged. If relative (HOME, START etc) then that
   * specifies what it should be relative to.
   *
   * output: make the result suitable for output to a...
   *
   * optional: should any relative path operation be controlled by the rel
   * path setting
   */
  enum RelativeRoot { NONE, FULL, HOME, START, HOME_OUTPUT, START_OUTPUT };
  enum OutputFormat { UNCHANGED, MAKEFILE, SHELL };
  std::string Convert(const char* source, 
                      RelativeRoot relative, 
                      OutputFormat output = UNCHANGED,
                      bool optional = false);
  
  ///! Call this prior to using Convert
  void SetupPathConversions();
  
  /**
   * Convert the given path to an output path that is optionally
   * relative based on the cache option CMAKE_USE_RELATIVE_PATHS.  The
   * remote path must use forward slashes and not already be escaped
   * or quoted.
   */
  std::string ConvertToOptionallyRelativeOutputPath(const char* remote);

  // flag to determine if this project should be included in a parent project
  bool GetExcludeAll()
    {
      return this->ExcludeFromAll;
    }
  void SetExcludeAll(bool b)
    {
      this->ExcludeFromAll = b;
    }
  
  ///! set/get the parent generator 
  cmLocalGenerator* GetParent(){return this->Parent;}
  void SetParent(cmLocalGenerator* g) { this->Parent = g; g->AddChild(this); }

  ///! set/get the children
  void AddChild(cmLocalGenerator* g) { this->Children.push_back(g); }
  std::vector<cmLocalGenerator*>& GetChildren() { return this->Children; };
    

  void AddLanguageFlags(std::string& flags, const char* lang,
                        const char* config);
  void AddSharedFlags(std::string& flags, const char* lang, bool shared);
  void AddConfigVariableFlags(std::string& flags, const char* var,
                              const char* config);
  void AppendFlags(std::string& flags, const char* newFlags);
  ///! Get the include flags for the current makefile and language
  const char* GetIncludeFlags(const char* lang); 

  /** Translate a dependency as given in CMake code to the name to
      appear in a generated build file.  If the given name is that of
      a CMake target it will be transformed to the real output
      location of that target for the given configuration.  If the
      given name is the full path to a file it will be returned.
      Otherwise the name is treated as a relative path with respect to
      the source directory of this generator.  This should only be
      used for dependencies of custom commands.  */
  std::string GetRealDependency(const char* name, const char* config);

  ///! for existing files convert to output path and short path if spaces
  std::string ConvertToOutputForExisting(const char* p);
  
  /** Called from command-line hook to check dependencies.  */
  virtual void CheckDependencies(cmMakefile* /* mf */, 
                                 bool /* verbose */,
                                 bool /* clear */) {};
  
  /** Called from command-line hook to scan dependencies.  */
  virtual bool ScanDependencies(std::vector<std::string> const& /* args */)
    {return true;};

  /** Compute the list of link libraries and directories for the given
      target and configuration.  */
  void ComputeLinkInformation(cmTarget& target, const char* config,
                              std::vector<cmStdString>& outLibs,
                              std::vector<cmStdString>& outDirs,
                              std::vector<cmStdString>* fullPathLibs=0);

  /** Get the include flags for the current makefile and language.  */
  void GetIncludeDirectories(std::vector<std::string>& dirs);

  // Create a struct to hold the varibles passed into
  // ExpandRuleVariables
  struct RuleVariables
  {
    RuleVariables()
      {
        this->Language= 0;
        this->Objects= 0;
        this->Target= 0;
        this->LinkLibraries= 0;
        this->Source= 0;
        this->Object= 0;
        this->Flags= 0;
        this->ObjectsQuoted= 0;
        this->TargetSOName= 0;
        this->TargetInstallNameDir = 0;
        this->LinkFlags= 0;
      }
    const char* Language;
    const char* Objects;
    const char* Target;
    const char* LinkLibraries;
    const char* Source;
    const char* Object;
    const char* Flags;
    const char* ObjectsQuoted;
    const char* TargetSOName;
    const char* TargetInstallNameDir;
    const char* LinkFlags;
    const char* LanguageCompileFlags;
  };

protected:
  /** Construct a script from the given list of command lines.  */
  std::string ConstructScript(const cmCustomCommandLines& commandLines,
                              const char* workingDirectory,
                              const char* newline = "\n");

  /** Construct a comment for a custom command.  */
  std::string ConstructComment(const cmCustomCommand& cc,
                               const char* default_comment = "");

  /** Fill out these strings for the given target.  Libraries to link,
   *  flags, and linkflags. */
  void GetTargetFlags(std::string& linkLibs, 
                      std::string& flags,
                      std::string& linkFlags,
                      cmTarget&target);
  
  ///! put all the libraries for a target on into the given stream
  virtual void OutputLinkLibraries(std::ostream&, cmTarget&, bool relink);

  // Expand rule variables in CMake of the type found in language rules
  void ExpandRuleVariables(std::string& string,
                           const RuleVariables& replaceValues);
  // Expand rule variables in a single string
  std::string ExpandRuleVariable(std::string const& variable,
                                 const RuleVariables& replaceValues);
  
  /** Convert a target to a utility target for unsupported 
   *  languages of a generator */
  void AddBuildTargetRule(const char* llang, cmTarget& target);
  ///! add a custom command to build a .o file that is part of a target 
  void AddCustomCommandToCreateObject(const char* ofname, 
                                      const char* lang, 
                                      cmSourceFile& source,
                                      cmTarget& target);
  // Create Custom Targets and commands for unsupported languages
  // The set passed in should contain the languages supported by the
  // generator directly.  Any targets containing files that are not
  // of the types listed will be compiled as custom commands and added
  // to a custom target.
  void CreateCustomTargetsAndCommands(std::set<cmStdString> const&);

  // Handle old-style install rules stored in the targets.
  void GenerateTargetInstallRules(
    std::ostream& os, const char* config,
    std::vector<std::string> const& configurationTypes);

  cmMakefile *Makefile;
  cmGlobalGenerator *GlobalGenerator;
  // members used for relative path function ConvertToMakefilePath
  std::string RelativePathToSourceDir;
  std::string RelativePathToBinaryDir;
  std::vector<std::string> HomeDirectoryComponents;
  std::vector<std::string> StartDirectoryComponents;
  std::vector<std::string> HomeOutputDirectoryComponents;
  std::vector<std::string> StartOutputDirectoryComponents;
  bool ExcludeFromAll;
  cmLocalGenerator* Parent;
  std::vector<cmLocalGenerator*> Children;
  std::map<cmStdString, cmStdString> LanguageToIncludeFlags;
  bool WindowsShell;
  bool ForceUnixPath;
  bool UseRelativePaths;
  bool IgnoreLibPrefix;
  bool Configured;
  bool EmitUniversalBinaryFlags;
  // Hack for ExpandRuleVariable until object-oriented version is
  // committed.
  std::string TargetImplib;
};

#endif
