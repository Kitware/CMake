/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
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
  cmTypeMacro(cmUseMangledMesaCommand, cmCommand);

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
  virtual bool InitialPass(std::vector<std::string> const& args,
                           cmExecutionStatus &status);
  
  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "use_mangled_mesa";}

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
      "  use_mangled_mesa(PATH_TO_MESA OUTPUT_DIRECTORY)\n"
      "The path to mesa includes, should contain gl_mangle.h.  "
      "The mesa headers are copied to the specified output directory.  "
      "This allows mangled mesa headers to override other GL headers by "
      "being added to the include directory path earlier.";
    }

  /** This command is kept for compatibility with older CMake versions. */
  virtual bool IsDiscouraged()
    {
    return true;
    }

protected:
  void CopyAndFullPathMesaHeader(const char* source,
                                 const char* outdir);
};



#endif
