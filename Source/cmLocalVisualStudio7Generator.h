/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmLocalVisualStudio7Generator_h
#define cmLocalVisualStudio7Generator_h

#include "cmLocalGenerator.h"

class cmMakeDepend;
class cmTarget;
class cmSourceFile;

// please remove me.... Yuck
#include "cmSourceGroup.h"

/** \class cmLocalVisualStudio7Generator
 * \brief Write a LocalUnix makefiles.
 *
 * cmLocalVisualStudio7Generator produces a LocalUnix makefile from its
 * member m_Makefile.
 */
class cmLocalVisualStudio7Generator : public cmLocalGenerator
{
public:
  ///! Set cache only and recurse to false by default.
  cmLocalVisualStudio7Generator();

  virtual ~cmLocalVisualStudio7Generator();
  
  /**
   * Generate the makefile for this directory. fromTheTop indicates if this
   * is being invoked as part of a global Generate or specific to this
   * directory. The difference is that when done from the Top we might skip
   * some steps to save time, such as dependency generation for the
   * makefiles. This is done by a direct invocation from make. 
   */
  virtual void Generate(bool fromTheTop);

  enum BuildType {STATIC_LIBRARY, DLL, EXECUTABLE, WIN32_EXECUTABLE, UTILITY};

  /**
   * Specify the type of the build: static, dll, or executable.
   */
  void SetBuildType(BuildType,const char *name);

  /**
   * Return array of created DSP names in a STL vector.
   * Each executable must have its own dsp.
   */
  std::vector<std::string> GetCreatedProjectNames() 
    {
    return m_CreatedProjectNames;
    }

private:
  void OutputVCProjFile();
  void WriteVCProjHeader(std::ostream& fout, const char *libName,
                         const cmTarget &tgt, std::vector<cmSourceGroup> &sgs);
  void WriteVCProjFooter(std::ostream& fout);
  void CreateSingleVCProj(const char *lname, cmTarget &tgt);
  void WriteVCProjFile(std::ostream& fout, const char *libName, 
                       cmTarget &tgt);
  void AddVCProjBuildRule(cmSourceGroup&);
  void WriteConfigurations(std::ostream& fout,
                           const char *libName,
                           const cmTarget &tgt);
  void WriteConfiguration(std::ostream& fout,
                          const char* configName,
                          const char* libName,
                          const cmTarget &tgt); 
  std::string ConvertToXMLOutputPath(const char* path);
  std::string ConvertToXMLOutputPathSingle(const char* path);
  void OutputDefineFlags(std::ostream& fout);
  void OutputTargetRules(std::ostream& fout,
                         const cmTarget &target, 
                         const char *libName);
  void OutputBuildTool(std::ostream& fout, const char* configName,
                       const char* libname, const cmTarget& t);
  void OutputLibraries(std::ostream& fout,
                       const char* configName,
                       const char* libName,
                       const cmTarget &target);
  void OutputLibraryDirectories(std::ostream& fout,
                                const char* configName,
                                const char* libName,
                                const cmTarget &target);
  void OutputModuleDefinitionFile(std::ostream& fout,
                                  const cmTarget &target);
  void WriteProjectStart(std::ostream& fout, const char *libName,
                         const cmTarget &tgt, std::vector<cmSourceGroup> &sgs);
  void WriteVCProjBeginGroup(std::ostream& fout, 
			  const char* group,
			  const char* filter);
  void WriteVCProjEndGroup(std::ostream& fout);
  std::string CombineCommands(const cmSourceGroup::Commands &commands,
                              cmSourceGroup::CommandFiles &totalCommand,
                              const char *source);
  void WriteCustomRule(std::ostream& fout,
                       const char* source,
                       const char* command,
                       const std::set<std::string>& depends,
                       const std::set<std::string>& outputs,
                       const char* extraFlags);

  std::vector<std::string> m_CreatedProjectNames;
  std::string m_LibraryOutputPath;
  std::string m_ExecutableOutputPath;
  std::string m_ModuleDefinitionFile;

  /*
  std::string m_DSPHeaderTemplate;
  std::string m_DSPFooterTemplate;
  void WriteDSPBeginGroup(std::ostream& fout, 
			  const char* group,
			  const char* filter);
  void WriteDSPEndGroup(std::ostream& fout);


  void WriteCustomRule(std::ostream& fout,
                       const char* source,
                       const char* command,
                       const std::set<std::string>& depends,
                       const std::set<std::string>& outputs,
                       const char* flags);

  std::string CreateTargetRules(const cmTarget &target, 
                                const char *libName);
  std::string CombineCommands(const cmSourceGroup::Commands &commands,
                              cmSourceGroup::CommandFiles &totalCommand,
                              const char *source);

  std::string m_IncludeOptions;
  std::vector<std::string> m_Configurations;
  */
};

#endif

