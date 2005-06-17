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
#ifndef cmCommand_h
#define cmCommand_h

#include "cmObject.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"

/** \class cmCommand
 * \brief Superclass for all commands in CMake.
 *
 * cmCommand is the base class for all commands in CMake. A command
 * manifests as an entry in CMakeLists.txt and produces one or
 * more makefile rules. Commands are associated with a particular
 * makefile. This base class cmCommand defines the API for commands 
 * to support such features as enable/disable, inheritance, 
 * documentation, and construction.
 */
class cmCommand : public cmObject
{
public:
  cmTypeMacro(cmCommand, cmObject);

  /**
   * Construct the command. By default it is enabled with no makefile.
   */
  cmCommand()  
    {m_Makefile = 0; m_Enabled = true;}

  /**
   * Need virtual destructor to destroy real command type.
   */
  virtual ~cmCommand() {}
  
  /**
   * Specify the makefile.
   */
  void SetMakefile(cmMakefile*m) 
    {m_Makefile = m; }
  cmMakefile* GetMakefile() { return m_Makefile; }

  /**
   * This is called by the cmMakefile when the command is first
   * encountered in the CMakeLists.txt file.  It expands the command's
   * arguments and then invokes the InitialPass.
   */
  virtual bool InvokeInitialPass(const std::vector<cmListFileArgument>& args)
    {
    std::vector<std::string> expandedArguments;
    m_Makefile->ExpandArguments(args, expandedArguments);
    return this->InitialPass(expandedArguments);
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args) = 0;

  /**
   * This is called at the end after all the information
   * specified by the command is accumulated. Most commands do
   * not implement this method.  At this point, reading and
   * writing to the cache can be done.
   */
  virtual void FinalPass() {};
  
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() = 0;
  
  /**
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable()
    {
    return false;
    }

  /**
   * This determines if the method is deprecated or not. 
   */
  virtual bool IsDeprecated(int /*major*/, int /*minor*/)
    {
    return false;
    }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() = 0;

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() = 0;

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation() = 0;

  /**
   * Enable the command.
   */
  void EnabledOn() 
    {m_Enabled = true;}

  /**
   * Disable the command.
   */
  void EnabledOff() 
    {m_Enabled = false;}

  /**
   * Query whether the command is enabled.
   */
  bool GetEnabled()  
    {return m_Enabled;}

  /**
   * Disable or enable the command.
   */
  void SetEnabled(bool enabled)  
    {m_Enabled = enabled;}

  /**
   * Return the last error string.
   */
  const char* GetError() 
    {
      if(m_Error.length() == 0)
        {
        m_Error = this->GetName();
        m_Error += " unknown error.";
        }
      return m_Error.c_str();
    }

  /**
   * Set the error message
   */
  void SetError(const char* e)
    {
    m_Error = this->GetName();
    m_Error += " ";
    m_Error += e;
    }

protected:
  cmMakefile* m_Makefile;

private:
  bool m_Enabled;
  std::string m_Error;
};

#endif
