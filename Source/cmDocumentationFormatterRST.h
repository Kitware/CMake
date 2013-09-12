/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef _cmDocumentationFormatterRST_h
#define _cmDocumentationFormatterRST_h

#include "cmStandardIncludes.h"

#include "cmDocumentationFormatterText.h"

/** Class to print the documentation as reStructuredText.  */
class cmDocumentationFormatterRST : public cmDocumentationFormatterText
{
public:
  cmDocumentationFormatterRST();

  virtual cmDocumentationEnums::Form GetForm() const
                                      { return cmDocumentationEnums::RSTForm;}

  virtual void PrintSection(std::ostream& os,
                    const cmDocumentationSection& section,
                    const char* name);
  virtual void PrintPreformatted(std::ostream& os, const char* text);
};

#endif
