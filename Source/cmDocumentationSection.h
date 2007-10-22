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
#ifndef _cmDocumentationSection_h
#define _cmDocumentationSection_h

#include "cmStandardIncludes.h"
#include "cmDocumentationFormatter.h"

// Low-level interface for custom documents:
/** Internal class representing a section of the documentation.
 * Cares e.g. for the different section titles in the different
 * output formats.
 */
class cmDocumentationSection
{
public:
  /** Create a cmSection, with a special name for man-output mode. */
  cmDocumentationSection(const char* name, const char* manName)
    :Name(name), ManName(manName)       {}
  
  /** Has any content been added to this section or is it empty ? */
  bool IsEmpty() const { return this->Entries.empty(); }
  
  /** Clear contents. */
  void Clear() { this->Entries.clear(); }
  
  /** Return the name of this section for the given output form. */
  const char* GetName(cmDocumentationEnums::Form form) const
  { return (form==cmDocumentationEnums::ManForm ?
            this->ManName.c_str() : this->Name.c_str()); }
  
  /** Return a pointer to the first entry of this section. */
  const std::vector<cmDocumentationEntry> &GetEntries() const
  { return this->Entries; }
  
  /** Append an entry to this section. */
  void Append(const cmDocumentationEntry& entry)
  { this->Entries.push_back(entry); }
  void Append(const std::vector<cmDocumentationEntry> &entries)
  { this->Entries.insert(this->Entries.end(),entries.begin(),entries.end()); }
  
  /** Append an entry to this section using NULL terminated chars */
  void Append(const char *[][3]);
  void Append(const char *n, const char *b, const char *f);

  /** prepend some documentation to this section */
  void Prepend(const char *[][3]);

  
private:
  std::string Name;
  std::string ManName;
  std::vector<cmDocumentationEntry> Entries;
};

#endif
