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
#ifndef cmUseMangledMesaCommand_h
#define cmUseMangledMesaCommand_h

#include "cmCommand.h"

#include "cmSourceFile.h"

/** \class cmUseMangledMesaCommand
 * \brief Create Tcl Wrappers for VTK classes.
 *
 * cmUseMangledMesaCommand is used to define a CMake variable include
 * path location by specifying a file and list of directories.
 */
class cmUseMangledMesaCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmUseMangledMesaCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);
  
  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "USE_MANGLED_MESA";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Copy mesa headers for use in combination with system GL.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  USE_MANGLED_MESA(PATH_TO_MESA OUTPUT_DIRECTORY)\n"
      "The path to mesa includes, should contain gl_mangle.h.  "
      "The mesa headers are copied to the specified output directory.  "
      "This allows mangled mesa headers to override other GL headers by "
      "being added to the include directory path earlier.";
    }

protected:
  void CopyAndFullPathMesaHeader(const char* source,
                                 const char* outdir);
private:
  std::vector<cmSourceFile> m_WrapClasses;
  std::vector<std::string> m_WrapHeaders;
  std::string m_LibraryName;
  std::string m_SourceList;
  std::vector<std::string> m_Commands;
};



#endif
