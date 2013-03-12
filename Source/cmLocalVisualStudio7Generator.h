/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmLocalVisualStudio7Generator_h
#define cmLocalVisualStudio7Generator_h

#include "cmLocalVisualStudioGenerator.h"
#include "cmVisualStudioGeneratorOptions.h"

class cmTarget;
class cmSourceFile;
class cmCustomCommand;
class cmSourceGroup;


class cmLocalVisualStudio7GeneratorOptions;
class cmLocalVisualStudio7GeneratorFCInfo;
class cmLocalVisualStudio7GeneratorInternals;

/** \class cmLocalVisualStudio7Generator
 * \brief Write Visual Studio .NET project files.
 *
 * cmLocalVisualStudio7Generator produces a Visual Studio .NET project
 * file for each target in its directory.
 */
class cmLocalVisualStudio7Generator : public cmLocalVisualStudioGenerator
{
public:
  ///! Set cache only and recurse to false by default.
  cmLocalVisualStudio7Generator(VSVersion v);

  virtual ~cmLocalVisualStudio7Generator();

  virtual void AddHelperCommands();

  /**
   * Generate the makefile for this directory.
   */
  virtual void Generate();

  enum BuildType {STATIC_LIBRARY, DLL, EXECUTABLE, WIN32_EXECUTABLE, UTILITY};

  /**
   * Specify the type of the build: static, dll, or executable.
   */
  void SetBuildType(BuildType,const char *name);

  void SetPlatformName(const char* n) { this->PlatformName = n;}

  void SetExtraFlagTable(cmVS7FlagTable const* table)
    { this->ExtraFlagTable = table; }
  virtual std::string GetTargetDirectory(cmTarget const&) const;
  cmSourceFile* CreateVCProjBuildRule();
  void WriteStampFiles();
  virtual std::string ComputeLongestObjectDirectory(cmTarget&) const;

  virtual void ReadAndStoreExternalGUID(const char* name,
                                        const char* path);
  virtual void AddCMakeListsRules();
protected:
  void CreateSingleVCProj(const char *lname, cmTarget &tgt);
private:
  typedef cmVisualStudioGeneratorOptions Options;
  typedef cmLocalVisualStudio7GeneratorFCInfo FCInfo;
  std::string GetBuildTypeLinkerFlags(std::string rootLinkerFlags,
                                      const char* configName);
  void FixGlobalTargets();
  void WriteProjectFiles();
  void WriteVCProjHeader(std::ostream& fout, const char *libName,
                         cmTarget &tgt, std::vector<cmSourceGroup> &sgs);
  void WriteVCProjFooter(std::ostream& fout, cmTarget &target);
  void WriteVCProjFile(std::ostream& fout, const char *libName,
                       cmTarget &tgt);
  void WriteConfigurations(std::ostream& fout,
                           const char *libName, cmTarget &tgt);
  void WriteConfiguration(std::ostream& fout,
                          const char* configName,
                          const char* libName, cmTarget &tgt);
  std::string EscapeForXML(const char* s);
  std::string ConvertToXMLOutputPath(const char* path);
  std::string ConvertToXMLOutputPathSingle(const char* path);
  void OutputTargetRules(std::ostream& fout, const char* configName,
                         cmTarget &target, const char *libName);
  void OutputBuildTool(std::ostream& fout, const char* configName,
                       cmTarget& t, bool debug);
  void OutputLibraryDirectories(std::ostream& fout,
                                std::vector<std::string> const& dirs);
  void WriteProjectSCC(std::ostream& fout, cmTarget& target);
  void WriteProjectStart(std::ostream& fout, const char *libName,
                         cmTarget &tgt, std::vector<cmSourceGroup> &sgs);
  void WriteProjectStartFortran(std::ostream& fout, const char *libName,
                                cmTarget &tgt);
  void WriteVCProjBeginGroup(std::ostream& fout,
                          const char* group,
                          const char* filter);
  void WriteVCProjEndGroup(std::ostream& fout);

  void WriteCustomRule(std::ostream& fout,
                       const char* source,
                       const cmCustomCommand& command,
                       FCInfo& fcinfo);
  void WriteTargetVersionAttribute(std::ostream& fout, cmTarget& target);

  bool WriteGroup(const cmSourceGroup *sg,
                  cmTarget& target, std::ostream &fout,
                  const char *libName, std::vector<std::string> *configs);

  friend class cmLocalVisualStudio7GeneratorFCInfo;
  friend class cmLocalVisualStudio7GeneratorInternals;

  class EventWriter;
  friend class EventWriter;

  cmVS7FlagTable const* ExtraFlagTable;
  std::string ModuleDefinitionFile;
  bool FortranProject;
  bool WindowsCEProject;
  std::string PlatformName; // Win32 or x64
  cmLocalVisualStudio7GeneratorInternals* Internal;
};


#endif

