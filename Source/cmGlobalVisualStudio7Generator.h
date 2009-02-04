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
#ifndef cmGlobalVisualStudio7Generator_h
#define cmGlobalVisualStudio7Generator_h

#include "cmGlobalVisualStudioGenerator.h"

class cmTarget;
struct cmVS7FlagTable;

/** \class cmGlobalVisualStudio7Generator
 * \brief Write a Unix makefiles.
 *
 * cmGlobalVisualStudio7Generator manages UNIX build process for a tree
 */
class cmGlobalVisualStudio7Generator : public cmGlobalVisualStudioGenerator
{
public:
  cmGlobalVisualStudio7Generator();
  static cmGlobalGenerator* New() { 
    return new cmGlobalVisualStudio7Generator; }
  
  ///! Get the name for the generator.
  virtual const char* GetName() const {
    return cmGlobalVisualStudio7Generator::GetActualName();}
  static const char* GetActualName() {return "Visual Studio 7";}

  ///! Create a local generator appropriate to this Global Generator
  virtual cmLocalGenerator *CreateLocalGenerator();

  /** Get the documentation entry for this generator.  */
  virtual void GetDocumentation(cmDocumentationEntry& entry) const;
  
  /**
   * Try to determine system infomation such as shared library
   * extension, pthreads, byte order etc.  
   */
  virtual void EnableLanguage(std::vector<std::string>const& languages, 
                              cmMakefile *, bool optional);

  /**
   * Try running cmake and building a file. This is used for dynalically
   * loaded commands, not as part of the usual build process.
   */
  virtual std::string GenerateBuildCommand(const char* makeProgram,
                                           const char *projectName, 
                                           const char* additionalOptions, 
                                           const char *targetName,
                                           const char* config,
                                           bool ignoreErrors,
                                           bool fast);

  /**
   * Generate the all required files for building this project/tree. This
   * basically creates a series of LocalGenerators for each directory and
   * requests that they Generate.  
   */
  virtual void Generate();

  /**
   * Generate the DSW workspace file.
   */
  virtual void OutputSLNFile();

  /**
   * Get the list of configurations
   */
  std::vector<std::string> *GetConfigurations();
  
  ///! Create a GUID or get an existing one.
  void CreateGUID(const char* name);
  std::string GetGUID(const char* name);

  ///! do configure step
  virtual void Configure();

  /** Append the subdirectory for the given configuration.  */
  virtual void AppendDirectoryForConfig(const char* prefix,
                                        const char* config,
                                        const char* suffix,
                                        std::string& dir);

  ///! What is the configurations directory variable called?
  virtual const char* GetCMakeCFGInitDirectory()  { return "$(OutDir)"; }

  struct TargetCompare
  {
    bool operator()(cmTarget const* l, cmTarget const* r);
  };

protected:
  static cmVS7FlagTable const* GetExtraFlagTableVS7();
  virtual void OutputSLNFile(cmLocalGenerator* root, 
                             std::vector<cmLocalGenerator*>& generators);
  virtual void WriteSLNFile(std::ostream& fout, cmLocalGenerator* root,
                            std::vector<cmLocalGenerator*>& generators);
  virtual void WriteProject(std::ostream& fout, 
                            const char* name, const char* path, cmTarget &t);
  virtual void WriteProjectDepends(std::ostream& fout, 
                           const char* name, const char* path, cmTarget &t);
  virtual void WriteProjectConfigurations(std::ostream& fout,
                                          const char* name,
                                          bool partOfDefaultBuild);
  virtual void WriteSLNFooter(std::ostream& fout);
  virtual void WriteSLNHeader(std::ostream& fout);
  virtual void AddPlatformDefinitions(cmMakefile* mf);

  class OrderedTargetDependSet: public std::multiset<cmTarget*, TargetCompare>
  {
  public:
    OrderedTargetDependSet(cmGlobalGenerator::TargetDependSet const&);
  };

  virtual void WriteTargetsToSolution(
    std::ostream& fout,
    cmLocalGenerator* root,
    OrderedTargetDependSet const& projectTargets,
    cmGlobalGenerator::TargetDependSet& originalTargets);
  virtual void WriteTargetDepends(
    std::ostream& fout,
    OrderedTargetDependSet const& projectTargets);
  virtual void WriteTargetConfigurations(
    std::ostream& fout,
    cmLocalGenerator* root,
    OrderedTargetDependSet const& projectTargets);
  
  void AddAllBuildDepends(cmLocalGenerator* root,
                          cmTarget* target,
                          cmGlobalGenerator::TargetDependSet& targets);
                                       
  void GenerateConfigurations(cmMakefile* mf);

  virtual void WriteExternalProject(std::ostream& fout, 
                                    const char* name, 
                                    const char* path,
                                    const std::vector<std::string>&
                                    dependencies);

  std::string ConvertToSolutionPath(const char* path);

  bool IsPartOfDefaultBuild(const char* project,
                            cmTarget* target);
  std::vector<std::string> Configurations;
  std::map<cmStdString, cmStdString> GUIDMap;

  // Set during OutputSLNFile with the name of the current project.
  // There is one SLN file per project.
  std::string CurrentProject;
};

#define CMAKE_CHECK_BUILD_SYSTEM_TARGET "ZERO_CHECK"

#endif
