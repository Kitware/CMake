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
 * cmDSWMakefile - class to write a microsoft DSW file.
 */
#ifndef cmDSWMakefile_h
#define cmDSWMakefile_h
#include "cmStandardIncludes.h"
#include "cmMakefile.h"

class cmDSPMakefile;
class cmMSProjectGenerator;

class cmDSWMakefile 
{
public:
  cmDSWMakefile(cmMakefile*);
  virtual void OutputDSWFile();
private:
  void FindAllCMakeListsFiles(const char* subdir,
			      std::vector<cmMSProjectGenerator*>&);
  void WriteDSWFile(std::ostream& fout);
  void WriteDSWHeader(std::ostream& fout);
  void WriteProject(std::ostream& fout, 
                    const char* name, const char* path,
                    cmDSPMakefile* project);
  void WriteDSWFooter(std::ostream& fout);
  cmMakefile* m_Makefile;
};

#endif
