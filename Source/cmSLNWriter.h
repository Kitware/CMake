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
#ifndef cmSLNWriter_h
#define cmSLNWriter_h

#include "cmStandardIncludes.h"
#include "cmMakefile.h"

class cmVCProjWriter;

/** \class cmSLNWriter
 * \brief Write a Microsoft Visual C++ .NET SLN (workspace) file.
 *
 * cmSLNWriter produces a Microsoft Visual C++ .NET SLN (workspace) file.
 */
class cmSLNWriter 
{
public:
  /**
   * Constructor.
   */
  cmSLNWriter(cmMakefile*);
  
  /**
   * Generate the SLN workspace file.
   */
  virtual void OutputSLNFile();

private:
  std::string CreateGUID(const char* project);
  void WriteSLNFile(std::ostream& fout);
  void WriteSLNHeader(std::ostream& fout);
  void WriteProject(std::ostream& fout, 
                    const char* name, const char* path,
                    cmVCProjWriter* project, const cmTarget &t);
  void WriteProjectDepends(std::ostream& fout, 
                           const char* name, const char* path,
                           cmVCProjWriter* project, const cmTarget &t);
  void WriteProjectConfigurations(std::ostream& fout, const char* name);
  void WriteExternalProject(std::ostream& fout, 
                    const char* name, const char* path,
                    const std::vector<std::string>& dependencies);
  void WriteSLNFooter(std::ostream& fout);
  cmMakefile* m_Makefile;
  std::map<cmStdString, cmStdString> m_GUIDMap;
};

#endif
