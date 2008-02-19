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
#ifndef _cmDocumentationFormatter_h
#define _cmDocumentationFormatter_h

#include "cmStandardIncludes.h"

/** This is just a helper class to make it build with MSVC 6.0.
Actually the enums and internal classes could directly go into
cmDocumentation, but then MSVC6 complains in RequestedHelpItem that 
cmDocumentation is an undefined type and so it doesn't know the enums.
Moving the enums to a class which is then already completely parsed helps
against this. */
class cmDocumentationEnums
{
public:
  /** Types of help provided.  */
  enum Type 
  { None, Usage, Single, SingleModule, SingleProperty, SingleVariable,
    List, ModuleList, PropertyList, VariableList,
    Full, Properties, Variables, Modules, CustomModules, Commands, 
    CompatCommands, Copyright, Version };

  /** Forms of documentation output.  */
  enum Form { TextForm, HTMLForm, ManForm, UsageForm, DocbookForm };
};

class cmDocumentationSection;

/** Base class for printing the documentation in the various supported 
   formats. */
class cmDocumentationFormatter
{
public:
  cmDocumentationFormatter();
  virtual ~cmDocumentationFormatter();
  void PrintFormatted(std::ostream& os, const char* text);

  virtual cmDocumentationEnums::Form GetForm() const = 0;
  
  virtual void PrintHeader(const char* /*name*/, std::ostream& /*os*/) {}
  virtual void PrintFooter(std::ostream& /*os*/) {}
  virtual void PrintSection(std::ostream& os,
                    const cmDocumentationSection& section,
                    const char* name) = 0;
  virtual void PrintPreformatted(std::ostream& os, const char* text) = 0;
  virtual void PrintParagraph(std::ostream& os, const char* text) = 0;
};

#endif
