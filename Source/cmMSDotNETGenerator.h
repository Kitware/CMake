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
#ifndef cmMSDotNETGenerator_h
#define cmMSDotNETGenerator_h

#include "cmStandardIncludes.h"
#include "cmMakefileGenerator.h"

class cmVCProjWriter;
class cmSLNWriter;

/** \class cmMSDotNETGenerator
 * \brief Write a Microsoft Visual C++ DSP (project) file.
 *
 * cmMSDotNETGenerator produces a Microsoft Visual C++ DSP (project) file.
 */
class cmMSDotNETGenerator : public cmMakefileGenerator
{
public:
  ///! Constructor sets the generation of SLN files on.
  cmMSDotNETGenerator();

  ///! Destructor.
  ~cmMSDotNETGenerator();
  
  ///! Get the name for the generator.
  virtual const char* GetName() {return "Visual Studio 7";}

  ///! virtual copy constructor
  virtual cmMakefileGenerator* CreateObject() 
    { return new cmMSDotNETGenerator;}
  
  ///! Produce the makefile (in this case a Microsoft Visual C++ project).
  virtual void GenerateMakefile();

  ///! controls the SLN/DSP settings
  virtual void SetLocal(bool);

  /**
   * Turn off the generation of a Microsoft Visual C++ SLN file.
   * This causes only the dsp file to be created.  This
   * is used to run as a command line program from inside visual
   * studio.
   */
  void BuildSLNOff()  {m_BuildSLN = false;}

  ///! Turn on the generation of a Microsoft Visual C++ SLN file.
  void BuildProjOn() {m_BuildSLN = true;}

  ///! Retrieve a pointer to a cmSLNWriter instance.
  cmSLNWriter* GetSLNWriter() 
    {return m_SLNWriter;}

  ///! Retrieve a pointer to a cmVCProjWriter instance.
  cmVCProjWriter* GetVCProjWriter() 
    {return m_VCProjWriter;}

  /**
   * Try to determine system infomation such as shared library
   * extension, pthreads, byte order etc.  
   */
  virtual void ComputeSystemInfo();

private:
  cmSLNWriter* m_SLNWriter;
  cmVCProjWriter* m_VCProjWriter;
  bool m_BuildSLN;
};


#endif
