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
#ifndef cmMSProjectGenerator_h
#define cmMSProjectGenerator_h

#include "cmStandardIncludes.h"
#include "cmMakefileGenerator.h"

class cmDSPWriter;
class cmDSWWriter;

/** \class cmMSProjectGenerator
 * \brief Write a Microsoft Visual C++ DSP (project) file.
 *
 * cmMSProjectGenerator produces a Microsoft Visual C++ DSP (project) file.
 */
class cmMSProjectGenerator : public cmMakefileGenerator
{
public:
  ///! Constructor sets the generation of DSW files on.
  cmMSProjectGenerator();

  ///! Destructor.
  ~cmMSProjectGenerator();
  
  ///! Get the name for the generator.
  virtual const char* GetName() {return "Visual Studio 6";}

  ///! virtual copy constructor
  virtual cmMakefileGenerator* CreateObject() 
    { return new cmMSProjectGenerator;}
  
  ///! Produce the makefile (in this case a Microsoft Visual C++ project).
  virtual void GenerateMakefile();

  ///! controls the DSW/DSP settings
  virtual void SetLocal(bool);

  /**
   * Turn off the generation of a Microsoft Visual C++ DSW file.
   * This causes only the dsp file to be created.  This
   * is used to run as a command line program from inside visual
   * studio.
   */
  void BuildDSWOff()  {m_BuildDSW = false;}

  ///! Turn on the generation of a Microsoft Visual C++ DSW file.
  void BuildDSWOn() {m_BuildDSW = true;}

  ///! Retrieve a pointer to a cmDSWWriter instance.
  cmDSWWriter* GetDSWWriter() 
    {return m_DSWWriter;}

  ///! Retrieve a pointer to a cmDSPWriter instance.
  cmDSPWriter* GetDSPWriter() 
    {return m_DSPWriter;}

  /**
   * Try to determine system infomation such as shared library
   * extension, pthreads, byte order etc.  
   */
  virtual void EnableLanguage(const char*);

private:
  cmDSWWriter* m_DSWWriter;
  cmDSPWriter* m_DSPWriter;
  bool m_BuildDSW;
};


#endif
