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
 * itkUnixMakefile is used generate unix makefiles.
 */

#ifndef cmUnixMakefileGenerator_h
#define cmUnixMakefileGenerator_h
#include "cmMakefile.h"
#include "cmMakefileGenerator.h"

class cmUnixMakefileGenerator : public cmMakefileGenerator
{
public:
  /** 
   * Write the makefile to the named file
   */
  virtual void GenerateMakefile();
  void OutputDepends(std::ostream&);
protected:
  void OutputMakefile(const char* file);
  void OutputDependLibraries(std::ostream&);
};

#endif
