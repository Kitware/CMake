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
#ifndef cmVariableRequiresCommand_h
#define cmVariableRequiresCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmVariableRequiresCommand
 * \brief Displays a message to the user
 *
 */
class cmVariableRequiresCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmVariableRequiresCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);
  
  ///! 
  virtual void FinalPass();
  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "VARIABLE_REQUIRES";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Display an error message .";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "VARIABLE_REQUIRES(TEST_VARIABLE RESULT_VARIABLE "
      "REQUIRED_VARIABLE1 REQUIRED_VARIABLE2 ...) "
      "The first argument (TEST_VARIABLE) is the name of the varible to be "
      "tested, if that varible is false nothing else is done. If "
      "TEST_VARIABLE is true, then "
      "the next arguemnt (RESULT_VARIABLE) is a vairable that is set to true "
      "if all the "
      "required variables are set." 
      "The rest of the arguments are varibles that must be true or not "
      "set to NOTFOUND to avoid an error.  ";
    }
  
  cmTypeMacro(cmVariableRequiresCommand, cmCommand);
private:
  std::string m_ErrorMessage;
  std::vector<std::string> m_Arguments;
  bool m_RequirementsMet;
};


#endif
