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

#ifndef cmUnixMakefile_h
#define cmUnixMakefile_h
#include "cmMakefile.h"


class cmUnixMakefile : public cmMakefile
{
public:
  /** 
   * Write the makefile to the named file
   */
  void OutputMakefile(const char* file);
protected:
  void OutputDepends(std::ostream&);
};

#endif
