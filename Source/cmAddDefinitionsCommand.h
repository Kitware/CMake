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
#ifndef cmAddDefinitionsCommand_h
#define cmAddDefinitionsCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmAddDefinitionsCommand
 * \brief Specify a list of compiler defines
 *
 * cmAddDefinitionsCommand specifies a list of compiler defines. These defines will
 * be added to the compile command.  
 */
class cmAddDefinitionsCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmAddDefinitionsCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * This determines if the command gets propagated down
   * to makefiles located in subdirectories.
   */
  virtual bool IsInherited() {return true;}

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() {return "ADD_DEFINITIONS";}
  
  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Add -D define flags to command line for  environments.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "ADD_DEFINITIONS(-DFOO -DBAR ...)\n"
      "Add -D define flags to command line for  environments.";
    }
  
  cmTypeMacro(cmAddDefinitionsCommand, cmCommand);
};



#endif
