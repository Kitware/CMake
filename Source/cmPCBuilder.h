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
 * cmPCBuilder 
 */

#ifndef __cmPCBuilder_h
#define __cmPCBuilder_h
class cmMakefile;
/**
 * cmPCBuilder is a supper class used for creating a build
 * file for PC's.  This class can parse an cm Makefile.in and
 * extract the classes that need to be compiled.
 */

class cmPCBuilder 
{
public:
  cmPCBuilder();
  ~cmPCBuilder();
  virtual cmMakefile* GetMakefile() = 0;
  void SetInputMakefilePath(const char*);
  void SetHomeDirectory(const char*);
  void SetMakefileDirectory(const char*);
  void SetOutputDirectory(const char*);
  void SetOutputHomeDirectory(const char*);
};

#endif

