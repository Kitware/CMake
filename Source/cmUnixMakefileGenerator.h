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
#ifndef cmUnixMakefileGenerator_h
#define cmUnixMakefileGenerator_h

#include "cmMakefile.h"
#include "cmMakefileGenerator.h"

/** \class cmUnixMakefileGenerator
 * \brief Write a Unix makefiles.
 *
 * cmUnixMakefileGenerator produces a Unix makefile from its
 * member m_Makefile.
 */
class cmUnixMakefileGenerator : public cmMakefileGenerator
{
public:
  /**
   * Produce the makefile (in this case a Unix makefile).
   */
  virtual void GenerateMakefile();

  /**
   * Output the depend information for all the classes 
   * in the makefile.  These would have been generated
   * by the class cmMakeDepend.
   */
  void OutputObjectDepends(std::ostream&);

protected:
  void OutputMakefile(const char* file);
  void OutputMakeFlags(std::ostream&);
  void OutputVerbatim(std::ostream&);
  void OutputTargetRules(std::ostream& fout);
  void OutputLinkLibraries(std::ostream&, const char*);
  void OutputTargets(std::ostream&);
  void OutputSubDirectoryRules(std::ostream&);
  void OutputDependInformation(std::ostream&);
  void OutputDependencies(std::ostream&);
  void OutputCustomRules(std::ostream&);
};

#endif
