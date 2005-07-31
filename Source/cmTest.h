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
#ifndef cmTest_h
#define cmTest_h

#include "cmCustomCommand.h"

/** \class cmTest
 * \brief Represent a test
 *
 * cmTest is representation of a test.
 */
class cmTest
{
public:
  /**
   */
  cmTest();
  ~cmTest();

  ///! Set the test name
  void SetName(const char* name);
  const char* GetName() const { return m_Name.c_str(); }
  void SetCommand(const char* command);
  const char* GetCommand() const { return m_Command.c_str(); }
  void SetArguments(const std::vector<cmStdString>& args);
  const std::vector<cmStdString>& GetArguments() const
    {
    return m_Args;
    }

  /**
   * Print the structure to std::cout.
   */
  void Print() const;

  ///! Set/Get a property of this source file
  void SetProperty(const char *prop, const char *value);
  const char *GetProperty(const char *prop) const;
  bool GetPropertyAsBool(const char *prop) const;
    
private:
  std::map<cmStdString,cmStdString> m_Properties;
  cmStdString m_Name;
  cmStdString m_Command;
  std::vector<cmStdString> m_Args;
};

#endif

