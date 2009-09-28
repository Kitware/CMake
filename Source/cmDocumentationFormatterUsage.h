/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef _cmDocumentationFormatterUsage_h
#define _cmDocumentationFormatterUsage_h

#include "cmDocumentationFormatterText.h"

/** Class to print the documentation as usage on the command line.  */
class cmDocumentationFormatterUsage : public cmDocumentationFormatterText
{
public:
  cmDocumentationFormatterUsage();

  virtual cmDocumentationEnums::Form GetForm() const
                                     { return cmDocumentationEnums::UsageForm;}

  virtual void PrintSection(std::ostream& os,
                    const cmDocumentationSection& section,
                    const char* name);
};

#endif
