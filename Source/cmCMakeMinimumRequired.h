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
#ifndef cmCMakeMinimumRequired_h
#define cmCMakeMinimumRequired_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmCMakeMinimumRequired
 * \brief Build a CMAKE variable
 *
 * cmCMakeMinimumRequired sets a variable to a value with expansion.  
 */
class cmCMakeMinimumRequired : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmCMakeMinimumRequired;
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
  virtual const char* GetName() {return "CMAKE_MINIMUM_REQUIRED";}
  
  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Determine the command line that will build this project.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "CMAKE_MINIMUM_REQUIRED(NAME MAKECOMMAND)\n"
      "Within CMAKE set NAME to the command that will build this project from the command line using MAKECOMMAND.";
    }
  
  cmTypeMacro(cmCMakeMinimumRequired, cmCommand);
};



#endif
