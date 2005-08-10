/*=========================================================================

  Program:   WXDialog - wxWidgets X-platform GUI Front-End for CMake
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Author:    Jorgen Bodde

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#if !defined(CMAKECOMMANDLINEINFO_H)
#define CMAKECOMMANDLINEINFO_H

#include "cmStandardIncludes.h"

///////////////////////////////////////////////////////////////
// cmCommandLineInfo:
// See cmCommandLineInfo.cxx for the implementation of this class
//

class cmCommandLineInfo
{ 
  // Construction
public:
  cmCommandLineInfo(); 
  virtual ~cmCommandLineInfo();

  // Parse the command line
  bool ParseCommandLine(int argc, char* argv[]);

  // Retrieve the path of executable
  wxString GetPathToExecutable() { return this->m_ExecutablePath; }

  // Attributes
public:
  wxString m_WhereSource;
  wxString m_WhereBuild;
  bool m_AdvancedValues;
  wxString m_GeneratorChoiceString;
  wxString m_LastUnknownParameter;
  wxString m_ExecutablePath;
  bool m_ExitAfterLoad;

private:
  // Parse one argument
  size_t ParseSwitch(char **argv, int arg_index, int argc);

  // Return boolean value of the string
  static int GetBoolValue(const wxString&);

  // on windows the argument with spaces SUCKS! So we need to 
  // incorporate it with quotes.
  wxString GetStringParam(const wxString &str);

  wxString m_ValidArguments;
};

#endif // !defined(CMAKECOMMANDLINEINFO_H)
