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
// cmCommandLineInfo.h : main header file for the command line arguments
//

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
  void ParseCommandLine(int argc, char* argv[]);

  // Set the valid arguments
  void SetValidArguments(const std::string& va) { this->m_ValidArguments = va; }

  // Retrieve the path of executable
  std::string GetPathToExecutable() { return this->m_ExecutablePath; }

  // Attributes
public:
  std::string m_WhereSource;
  std::string m_WhereBuild;
  bool m_AdvancedValues;
  std::string m_GeneratorChoiceString;
  std::string m_LastUnknownParameter;
  std::string m_ExecutablePath;

protected:
  // Parse one argument
  void ParseParam(const std::string& parameter, bool know_about, bool last);

  // Return boolean value of the string
  static int GetBoolValue(const std::string&);

  std::string m_ValidArguments;
};

#endif // !defined(CMAKECOMMANDLINEINFO_H)
