/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) 2000 National Library of Medicine
  All rights reserved.

  See COPYRIGHT.txt for copyright details.

=========================================================================*/
#ifndef cmDSPMakefile_h
#define cmDSPMakefile_h

#include "cmStandardIncludes.h"
#include "cmMakefile.h"

/** \class cmDSPMakefile
 * \brief Generate a Microsoft DSP project file.
 *
 * cmDSPMakefile generates a Microsoft DSP project file.
 * See the *.dsptemplate files for information on the templates
 * used for making the project files.
 */
class cmDSPMakefile 
{
public:
  cmDSPMakefile(cmMakefile*);
  ~cmDSPMakefile();
  void OutputDSPFile();
  enum BuildType {STATIC_LIBRARY, DLL, EXECUTABLE};

  /**
   * Specify the type of the build: static, dll, or executable.
   */
  void SetBuildType(BuildType,const char *name);

  BuildType GetLibraryBuildType()
    {
      return m_LibraryBuildType;
    }
  

  /**
   * Return array of created DSP names in a STL vector.
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
  std::string m_DSPHeaderTemplate;
  std::string m_DSPFooterTemplate;
  std::vector<std::string> m_CreatedProjectNames;
  
  void CreateSingleDSP(const char *lname, cmTarget &tgt, 
                       const std::string &libs);
  void WriteDSPFile(std::ostream& fout, const char *libName, 
                    cmTarget &tgt, const std::string &libs);
  void WriteDSPBeginGroup(std::ostream& fout, 
			  const char* group,
			  const char* filter);
  void WriteDSPEndGroup(std::ostream& fout);
  void WriteDSPHeader(std::ostream& fout, const char *libName,
                      const std::string &libs);
  void WriteDSPBuildRule(std::ostream& fout, const char*);
  void WriteDSPBuildRule(std::ostream& fout);
  void WriteDSPFooter(std::ostream& fout);
  void AddDSPBuildRule(cmSourceGroup&);
  void WriteCustomRule(std::ostream& fout,
                       const char* command,
                       const std::set<std::string>& depends,
                       const std::set<std::string>& outputs);

  std::string m_IncludeOptions;
  cmMakefile* m_Makefile;
  BuildType m_LibraryBuildType;
  std::vector<std::string> m_Configurations;
};

#endif
