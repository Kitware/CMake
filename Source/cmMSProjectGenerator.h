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
 * cmMSProjectGenerator - class to write a microsoft DSW file.
 */
#ifndef cmMSProjectGenerator_h
#define cmMSProjectGenerator_h
#include "cmStandardIncludes.h"
#include "cmMakefileGenerator.h"

class cmDSPMakefile;
class cmDSWMakefile;


class cmMSProjectGenerator : public cmMakefileGenerator
{
public:
  cmMSProjectGenerator();
  ~cmMSProjectGenerator();
  virtual void GenerateMakefile();
  void SetBuildDSP() { m_BuildDSW = false;}
  void SetBuildDSW() { m_BuildDSW = true;}
  cmDSWMakefile* GetDSWMakefile() { return m_DSWMakefile;}
  cmDSPMakefile* GetDSPMakefile() { return m_DSPMakefile;}
private:
  cmDSWMakefile* m_DSWMakefile;
  cmDSPMakefile* m_DSPMakefile;
  bool m_BuildDSW;
};


#endif
