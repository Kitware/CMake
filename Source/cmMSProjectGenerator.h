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
#ifndef cmMSProjectGenerator_h
#define cmMSProjectGenerator_h

#include "cmStandardIncludes.h"
#include "cmMakefileGenerator.h"

class cmDSPMakefile;
class cmDSWMakefile;

/** \class cmMSProjectGenerator
 * \brief Write a Microsoft Visual C++ DSP (project) file.
 *
 * cmMSProjectGenerator produces a Microsoft Visual C++ DSP (project) file.
 */
class cmMSProjectGenerator : public cmMakefileGenerator
{
public:
  /**
   * Constructor sets the generation of DSW files on.
   */
  cmMSProjectGenerator();

  /**
   * Destructor.
   */
  ~cmMSProjectGenerator();

  /**
   * Produce the makefile (in this case a Microsoft Visual C++ project).
   */
  virtual void GenerateMakefile();

  /**
   * Turn off the generation of a Microsoft Visual C++ DSP file.
   */
  void BuildDSPOff() 
    {m_BuildDSW = false;}

  /**
   * Turn on the generation of a Microsoft Visual C++ DSW file.
   */
  void BuildDSWOn() 
    {m_BuildDSW = true;}

  /**
   * Retrieve a pointer to a cmDSWMakefile instance.
   */
  cmDSWMakefile* GetDSWMakefile() 
    {return m_DSWMakefile;}

  /**
   * Retrieve a pointer to a cmDSPMakefile instance.
   */
  cmDSPMakefile* GetDSPMakefile() 
    {return m_DSPMakefile;}

private:
  cmDSWMakefile* m_DSWMakefile;
  cmDSPMakefile* m_DSPMakefile;
  bool m_BuildDSW;
};


#endif
