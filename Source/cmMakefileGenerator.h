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
#ifndef cmMakefileGenerator_h
#define cmMakefileGenerator_h

#include "cmStandardIncludes.h"

class cmMakefile;
class cmClassFile;

/** \class cmMakefileGenerator
 * \brief Provide an abstract interface for classes generating makefiles.
 *
 * Subclasses of this abstract class generate makefiles for various
 * platforms.
 */
class cmMakefileGenerator
{
public:
  /**
   * Set the cmMakefile instance from which to generate the makefile.
   */
  void SetMakefile(cmMakefile*);

  /**
   * Generate the makefile using the m_Makefile, m_CustomRules, 
   * and m_ExtraSourceFiles. All subclasses of cmMakefileGenerator
   * must implement this method.
   */
  virtual void GenerateMakefile() = 0;

protected:
  cmMakefile* m_Makefile;
};

#endif
