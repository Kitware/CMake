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
#ifndef cmVCProjWriter_h
#define cmVCProjWriter_h

#include "cmStandardIncludes.h"
#include "cmMakefile.h"

/** \class cmVCProjWriter
 * \brief Generate a Microsoft VCProj project file.
 *
 * cmVCProjWriter generates a Microsoft VCProj project file.
 * See the *.dsptemplate files for information on the templates
 * used for making the project files.
 */
class cmVCProjWriter 
{
public:
  cmVCProjWriter(cmMakefile*);
  ~cmVCProjWriter();
  void OutputVCProjFile();
  enum BuildType {STATIC_LIBRARY, DLL, EXECUTABLE, WIN32_EXECUTABLE, UTILITY};

  /**
   * Specify the type of the build: static, dll, or executable.
   */
  void SetBuildType(BuildType,const char *name);

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
  std::string m_VCProjHeaderTemplate;
  std::string m_VCProjFooterTemplate;
  std::vector<std::string> m_CreatedProjectNames;
  
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
  

  std::string m_IncludeOptions;
  cmMakefile* m_Makefile;
  std::vector<std::string> m_Configurations;
};

#endif
