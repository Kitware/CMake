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
#ifndef cmITKWrapTclCommand_h
#define cmITKWrapTclCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

class cmMakeDepend;

/** \class cmITKWrapTclCommand
 * \brief Run CABLE to generate Tcl wrappers.
 *
 * cmITKWrapTclCommand runs CABLE on the specified configuration files
 * and combines them into a package on a given target.
 */
class cmITKWrapTclCommand : public cmCommand
{
public:
  cmITKWrapTclCommand();
  ~cmITKWrapTclCommand();
  
  /** This is a virtual constructor for the command.  */
  virtual cmCommand* Clone() { return new cmITKWrapTclCommand; }

  /** This is called when the command is first encountered in
   * the CMakeLists.txt file.  */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /** The name of the command as specified in CMakeList.txt. */
  virtual const char* GetName() {return "ITK_WRAP_TCL";}
  
  /** Succinct documentation.  */
  virtual const char* GetTerseDocumentation() 
    { return "Run CABLE to generate Tcl wrappers."; }
  
  /** More documentation. */
  virtual const char* GetFullDocumentation()
    {
    return
      "ITK_WRAP_TCL(target-name config-file1 [config-file2 ...])\n"
      "Run CABLE on all the configuration files to generate Tcl wrappers.\n"
      "The generated sources are added to a target of the given name.";
    }
  
  cmTypeMacro(cmITKWrapTclCommand, cmCommand);
protected:
  cmStdString m_TargetName;
  cmTarget* m_Target;
  
  bool CreateCableRule(const char* configFile);
  std::string GetCableFromCache() const;
  
  cmMakeDepend* m_MakeDepend;
};

#endif
