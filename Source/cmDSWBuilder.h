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
 * cmDSWBuilder 
 */

#ifndef __cmDSWBuilder_h
#define __cmDSWBuilder_h

#include "cmPCBuilder.h"
class cmDSWMakefile;

class cmDSWBuilder : public cmPCBuilder
{
public:
  cmDSWBuilder();
  ~cmDSWBuilder();
  void CreateDSWFile();
  virtual cmMakefile* GetMakefile();
protected:
  cmDSWMakefile* m_Makefile;
};

#endif

