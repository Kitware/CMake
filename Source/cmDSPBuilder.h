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
 * cmDSPBuilder is a Facade class for cmDSWMakefile 
 */

#ifndef __cmDSPBuilder_h
#define __cmDSPBuilder_h
#include "cmPCBuilder.h"
#include <vector>
#include <string>
class cmDSPMakefile;

class cmDSPBuilder : public cmPCBuilder
{
public:
  cmDSPBuilder();
  ~cmDSPBuilder();
  void CreateDSPFile();
  std::vector<std::string> GetCreatedProjectNames();  
  virtual cmMakefile* GetMakefile();
protected:
  cmDSPMakefile* m_Makefile;
};

#endif

