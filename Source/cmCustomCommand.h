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

/** \class cmCustomCommand
 * \brief A class to encapsulate a custom command
 *
 * cmCustomCommand encapsulates the properties of a custom command
 */
class cmCustomCommand
{
public:
  cmCustomCommand(const char *src, const char *command,
                  std::vector<std::string> dep,
                  std::vector<std::string> out);
  cmCustomCommand(const cmCustomCommand& r);
  
  /**
   * Use the cmMakefile's Expand commands to expand any variables in
   * this objects members.
   */
  void ExpandVariables(const cmMakefile &);

  /**
   * Return the name of the source file. I'm not sure if this is a full path or not.
   */
  std::string GetSourceName() const {return m_Source;}
  void SetSourceName(const char *name) {m_Source = name;}

  /**
   * Return the command to execute
   */
  std::string GetCommand() const {return m_Command;}
  void SetCommand(const char *cmd) {m_Command = cmd;}
  
  /**
   * Return the vector that holds the list of dependencies
   */
  const std::vector<std::string> &GetDepends() const {return m_Depends;}
  std::vector<std::string> &GetDepends() {return m_Depends;}
  
  /**
   * Return the vector that holds the list of outputs of this command
   */
  const std::vector<std::string> &GetOutputs() const {return m_Outputs;}
  std::vector<std::string> &GetOutputs() {return m_Outputs;}
  
private:
  std::string m_Source;
  std::string m_Command;
  std::vector<std::string> m_Depends;
  std::vector<std::string> m_Outputs;
};


#endif
