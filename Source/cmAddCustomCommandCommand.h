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
#ifndef cmAddCustomCommandCommand_h
#define cmAddCustomCommandCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmAddCustomCommandCommand
 * \brief 
 *
 *  cmAddCustomCommandCommand defines a new command that can
 *  be executed within the CMake
 *
 *  In makefile terms this creates new target in the following form:
 *  OUTPUT1: SOURCE DEPENDS
 *           COMMAND ARGS
 *  OUTPUT2: SOURCE DEPENDS
 *           COMMAND ARGS
 *  ...
 *  Example of usage:
 *  ADD_CUSTOM_COMMAND(
 *             SOURCE ${VTK_TIFF_FAX_EXE} 
 *             COMMAND ${VTK_TIFF_FAX_EXE} 
 *             ARGS -c const ${VTK_BINARY_DIR}/Utilities/tiff/tif_fax3sm.c 
 *             TARGET vtktiff 
 *             OUTPUTS ${VTK_BINARY_DIR}/Utilities/tiff/tif_fax3sm.c
 *                    )
 *  This will create custom target which will generate file tif_fax3sm.c
 *  using command ${VTK_TIFF_FAX_EXE}.
 */

class cmAddCustomCommandCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmAddCustomCommandCommand;
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
  virtual const char* GetName() {return "ADD_CUSTOM_COMMAND";}
  
  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Create new command within CMake.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "ADD_CUSTOM_COMMAND(SOURCE source COMMAND command TARGET target "
      "[ARGS [args...]] [DEPENDS [depends...]] [OUTPUTS [outputs...]])\n"
      "Add a custom command.";
    }
  
  cmTypeMacro(cmAddCustomCommandCommand, cmCommand);
};



#endif
