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
#ifndef cmCableClassSetCommand_h
#define cmCableClassSetCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"
#include "cmCableClassSet.h"

/** \class cmCableClassSetCommand
 *
 */
class cmCableClassSetCommand : public cmCommand
{
public:
  cmCableClassSetCommand() {}
  virtual ~cmCableClassSetCommand() {}

  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
      return new cmCableClassSetCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);
  
  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "CABLE_CLASS_SET";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Define a set of classes for use in other CABLE commands.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "CABLE_CLASS_SET(set_name class1 class2 ...)\n"
      "Defines a set with the given name containing classes and their\n"
      "associated header files.  The set can later be used by other CABLE\n"
      "commands.";
    }  

  cmTypeMacro(cmCableClassSetCommand, cmCommand);
private:
  /**
   * The name of the class set.
   */
  std::string m_ClassSetName;  
};



#endif
