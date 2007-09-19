/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
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
                    const cmDocumentationEntry* section,
                    const char* name);
};

#endif
