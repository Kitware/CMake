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
#ifndef cmGlob_h
#define cmGlob_h

#include "cmStandardIncludes.h"

class cmGlobInternal;

/** \class cmGlob
 * \brief Helper class for performing globbing searches.
 *
 * Finds all files that match a given globbing expression.
 */
class cmGlob
{
public:
  cmGlob();
  ~cmGlob();

  //! Find all files that match the pattern.
  bool FindFiles(const std::string& inexpr);

  //! Return the list of files that matched.
  std::vector<std::string>& GetFiles();

  //! Set recurse to true to match subdirectories.
  void RecurseOn() { this->SetRecurse(true); }
  void RecurseOff() { this->SetRecurse(false); }
  void SetRecurse(bool i) { m_Recurse = i; }
  bool GetRecurse() { return m_Recurse; }

protected:
  //! Process directory
  void ProcessDirectory(std::string::size_type start, 
    const std::string& dir, bool dir_only);

  //! Process last directory, but only when recurse flags is on. That is
  // effectively like saying: /path/to/file/**/file
  void RecurseDirectory(std::string::size_type start,
    const std::string& dir, bool dir_only);

  //! Escape all non-alphanumeric characters in pattern.
  void Escape(int ch, char* buffer);

  //!
  // Translate a shell PATTERN to a regular expression.
  // There is no way to quote meta-characters.
  std::string ConvertExpression(const std::string& expr);

  //! Add regular expression
  void AddExpression(const char* expr);

  cmGlobInternal* m_Internals;
  bool m_Recurse;
};


#endif
