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
  void SetBuildType(BuildType);

  BuildType GetBuildType()
    {
      return m_BuildType;
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
  
  void CreateExecutableDSPFiles();
  void CreateSingleDSP();
  void WriteDSPFile(std::ostream& fout);
  void WriteDSPBeginGroup(std::ostream& fout, 
			  const char* group,
			  const char* filter);
  void WriteDSPEndGroup(std::ostream& fout);
  void WriteDSPHeader(std::ostream& fout);
  void WriteDSPBuildRules(std::ostream& fout, const char *extensions);
  void WriteDSPBuildRule(std::ostream& fout, const char*);
  void WriteDSPFooter(std::ostream& fout);
  void WriteDSPBuildRule(std::ostream& fout);
  void WriteCustomRule(std::ostream& fout,
		       const char* source,
		       const char* result,
		       const char* command,
                       std::vector<std::string>& depends);

  std::string m_IncludeOptions;
  std::string m_DebugLibraryOptions;
  std::string m_ReleaseLibraryOptions;
  std::string m_ReleaseMinSizeLibraryOptions;
  std::string m_DebugDLLLibraryOptions;
  std::string m_ReleaseDLLLibraryOptions;
  cmMakefile* m_Makefile;
  BuildType m_BuildType;
  std::vector<std::string> m_Configurations;
};

#endif
