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
/**
 * cmDSPMakefile generate a microsoft DSP project file.
 * see the *.dsptemplate files for information on the templates
 * used for making the project files.
 */
#ifndef cmDSPMakefile_h
#define cmDSPMakefile_h
#include "cmStandardIncludes.h"
#include "cmMakefile.h"

class cmDSPMakefile 
{
public:
  cmDSPMakefile(cmMakefile*);
  ~cmDSPMakefile();
  void OutputDSPFile();
  enum BuildType { STATIC_LIBRARY, DLL, EXECUTABLE };
  void SetBuildType(BuildType );
  // return array of created DSP names
  // Each executable must have its own dsp
  std::vector<std::string> GetCreatedProjectNames() 
    {
      return m_CreatedProjectNames;
    }
  cmMakefile* GetMakefile() 
    {
      return m_Makefile;
    }
  
private:
  std::string m_DSPHeaderTemplate;
  std::string m_DSPFooterTemplate;
  std::vector<std::string> m_CreatedProjectNames;
  
  void CreateExecutableDSPFiles();
  void CreateSingleDSP();
  void WriteDSPFile(std::ostream& fout);
  void WriteDSPBeginGroup(std::ostream& fout, 
			  const char* group,
			  const char* filter);
  void WriteDSPEndGroup(std::ostream& fout);
  void WriteDSPHeader(std::ostream& fout);
  void WriteDSPBuildRules(std::ostream& fout);
  void WriteDSPBuildRule(std::ostream& fout, const char*);
  void WriteDSPFooter(std::ostream& fout);
  void WriteDSPBuildRule(std::ostream& fout);
  void WriteCustomRule(std::ostream& fout,
		       const char* source,
		       const char* result,
		       const char* command);
private:
  std::string m_IncludeOptions;
  std::string m_DebugLibraryOptions;
  std::string m_ReleaseLibraryOptions;
  cmMakefile* m_Makefile;
};

#endif
