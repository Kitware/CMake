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
    return "Add a custom build rule to the generated build system.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  ADD_CUSTOM_COMMAND(TARGET target\n"
      "                     [SOURCE source]\n"
      "                     [COMMAND command]\n"
      "                     [ARGS [args...]]\n"
      "                     [DEPENDS [depends...]]\n"
      "                     [OUTPUTS [outputs...]]\n"
      "                     [COMMENT comment])\n"
      "This defines a new command that can be executed during the build "
      "process.  In makefile terms this creates a new target in the "
      "following form:\n"
      "  OUTPUT1: SOURCE DEPENDS\n"
      "           COMAND ARGS\n"
      "  OUTPUT2: SOURCE DEPENDS\n"
      "           COMAND ARGS\n"
      "The TARGET must be specified, but it is not the make target of the "
      "build rule.  It is the target (library, executable, or custom target) "
      "that will use the output generated from this rule.  This is necessary "
      "to choose a project file in which to generate the rule for Visual "
      "Studio.\n\n"
      "Example of usage:\n"
      "  ADD_CUSTOM_COMMAND(\n"
      "    TARGET tiff\n"
      "    SOURCE ${TIFF_FAX_EXE}\n"
      "    COMMAND ${TIFF_FAX_EXE}\n"
      "    ARGS -c const ${TIFF_BINARY_DIR}/tif_fax3sm.c\n"
      "    OUTPUTS ${TIFF_BINARY_DIR}/tif_fax3sm.c\n"
      "  )\n"
      "This will create custom target which will generate file tif_fax3sm.c "
      "using command ${TIFF_FAX_EXE}.  The rule will be executed as part of "
      "building the tiff library because it includes tif_fax3sm.c as a "
      "source file with the GENERATED property.";
    }
  
  cmTypeMacro(cmAddCustomCommandCommand, cmCommand);
};



#endif
