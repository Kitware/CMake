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
#ifndef cmDSWWriter_h
#define cmDSWWriter_h

#include "cmStandardIncludes.h"
#include "cmMakefile.h"

class cmDSPWriter;
class cmMSProjectGenerator;

/** \class cmDSWWriter
 * \brief Write a Microsoft Visual C++ DSW (workspace) file.
 *
 * cmDSWWriter produces a Microsoft Visual C++ DSW (workspace) file.
 */
class cmDSWWriter 
{
public:
  /**
   * Constructor.
   */
  cmDSWWriter(cmMakefile*);
  
  /**
   * Generate the DSW workspace file.
   */
  virtual void OutputDSWFile();

private:
  void WriteDSWFile(std::ostream& fout);
  void WriteDSWHeader(std::ostream& fout);
  void WriteProject(std::ostream& fout, 
                    const char* name, const char* path,
                    cmDSPWriter* project, const cmTarget &t);
  void WriteExternalProject(std::ostream& fout, 
                    const char* name, const char* path,
                    const std::vector<std::string>& dependencies);
  void WriteDSWFooter(std::ostream& fout);
  cmMakefile* m_Makefile;
};

#endif
