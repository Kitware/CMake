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
#ifndef cmMSDotNETGenerator_h
#define cmMSDotNETGenerator_h

#include "cmStandardIncludes.h"
#include "cmMakefileGenerator.h"
#include "cmTarget.h"
#include "cmSourceGroup.h"

/** \class cmMSDotNETGenerator
 * \brief Write a Microsoft Visual C++ DSP (project) file.
 *
 * cmMSDotNETGenerator produces a Microsoft Visual C++ DSP (project) file.
 */
class cmMSDotNETGenerator : public cmMakefileGenerator
{
public:
  ///! Constructor sets the generation of SLN files on.
  cmMSDotNETGenerator();

  ///! Destructor.
  ~cmMSDotNETGenerator();
  
  ///! Get the name for the generator.
  virtual const char* GetName() {return "Visual Studio 7";}

  ///! virtual copy constructor
  virtual cmMakefileGenerator* CreateObject() 
    { return new cmMSDotNETGenerator;}
  
  ///! Produce the makefile (in this case a Microsoft Visual C++ project).
  virtual void GenerateMakefile();

  ///! controls the SLN/DSP settings
  virtual void SetLocal(bool);

  /**
   * Turn off the generation of a Microsoft Visual C++ SLN file.
   * This causes only the dsp file to be created.  This
   * is used to run as a command line program from inside visual
   * studio.
   */
  void BuildSLNOff()  {m_BuildSLN = false;}

  ///! Turn on the generation of a Microsoft Visual C++ SLN file.
  void BuildProjOn() {m_BuildSLN = true;}

  /**
   * Try to determine system infomation such as shared library
   * extension, pthreads, byte order etc.  
   */
  virtual void ComputeSystemInfo();

protected:  
  /**
   * Return array of created VCProj names in a STL vector.
   * Each executable must have its own dsp.
   */
  std::vector<std::string> GetCreatedProjectNames() 
    {
    return m_CreatedProjectNames;
    }

  /**
   * Return the makefile.
   */
  cmMakefile* GetMakefile() 
    {
    return m_Makefile;
    }

private:
  void CreateSingleVCProj(const char *lname, cmTarget &tgt);
  void WriteVCProjFile(std::ostream& fout, const char *libName, 
                    cmTarget &tgt);
  void WriteVCProjBeginGroup(std::ostream& fout, 
			  const char* group,
			  const char* filter);
  void WriteVCProjEndGroup(std::ostream& fout);

  void WriteProjectStart(std::ostream& fout, const char *libName,
                         const cmTarget &tgt, std::vector<cmSourceGroup> &sgs);
  void WriteConfigurations(std::ostream& fout,
                           const char *libName,
                           const cmTarget &tgt);
  void WriteConfiguration(std::ostream& fout,
                          const char* configName,
                          const char* libName,
                          const cmTarget &tgt); 

  void OutputDefineFlags(std::ostream& fout);
  void WriteVCProjFooter(std::ostream& fout);
  void AddVCProjBuildRule(cmSourceGroup&);
  void WriteCustomRule(std::ostream& fout,
                       const char* source,
                       const char* command,
                       const std::set<std::string>& depends,
                       const std::set<std::string>& outputs);

  std::string CreateTargetRules(const cmTarget &target, 
                                const char *libName);
  std::string CombineCommands(const cmSourceGroup::Commands &commands,
                              cmSourceGroup::CommandFiles &totalCommand,
                              const char *source);
  

  virtual void OutputSLNFile();
  void OutputVCProjFile();
  std::string CreateGUID(const char* project);
  void WriteSLNFile(std::ostream& fout);
  void WriteSLNHeader(std::ostream& fout);
  void WriteProject(std::ostream& fout, 
                    const char* name, const char* path,
                    cmMSDotNETGenerator* project, const cmTarget &t);
  void WriteProjectDepends(std::ostream& fout, 
                           const char* name, const char* path,
                           cmMSDotNETGenerator* project, const cmTarget &t);
  void WriteProjectConfigurations(std::ostream& fout, const char* name);
  void WriteExternalProject(std::ostream& fout, 
                    const char* name, const char* path,
                    const std::vector<std::string>& dependencies);
  void WriteSLNFooter(std::ostream& fout);
  void OutputBuildTool(std::ostream& fout, const char* configName,
                       const char* libname, const cmTarget& t);
  void OutputLibraryDirectories(std::ostream& fout,
                                const char* configName,
                                const char* libName,
                                const cmTarget &target);
  void OutputLibraries(std::ostream& fout,
                       const char* configName,
                       const char* libName,
                       const cmTarget &target);
  
private:
  std::map<cmStdString, cmStdString> m_GUIDMap;
  bool m_BuildSLN;
  std::string m_LibraryOutputPath;
  std::string m_ExecutableOutputPath;
  std::string m_IncludeOptions;
  std::vector<std::string> m_Configurations;
  std::string m_VCProjHeaderTemplate;
  std::string m_VCProjFooterTemplate;
  std::vector<std::string> m_CreatedProjectNames;
};


#endif
