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
#ifndef cmCustomCommand_h
#define cmCustomCommand_h

#include "cmStandardIncludes.h"
class cmMakefile;

class cmCustomCommand
{
public:
  cmCustomCommand(const char *src, const char *command,
                  std::vector<std::string> dep,
                  std::vector<std::string> out);
  cmCustomCommand(const cmCustomCommand& r);
  void ExpandVariables(const cmMakefile &);
  
  std::string m_Source;
  std::string m_Command;
  std::vector<std::string> m_Depends;
  std::vector<std::string> m_Outputs;
};


#endif
