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
#ifndef cmVTKMakeInstantiatorCommand_h
#define cmVTKMakeInstantiatorCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

/** \class cmVTKMakeInstantiatorCommand
 * cmVTKMakeInstantiatorCommand implements the VTK_MAKE_INSTANTIATOR
 * command.  This generates a source file to add to a VTK library that
 * registers instance creation functions with vtkInstantiator for every
 * class in that library.
 */
class cmVTKMakeInstantiatorCommand : public cmCommand
{
public:
  /** This is a virtual constructor for the command. */
  virtual cmCommand* Clone()
    { return new cmVTKMakeInstantiatorCommand; }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /** The name of the command as specified in CMakeList.txt. */
  virtual const char* GetName() { return "VTK_MAKE_INSTANTIATOR"; }

  /** Succinct documentation.  */
  virtual const char* GetTerseDocumentation() 
    {
    return "Deprecated.  For use only in VTK 4.0.";
    }
  
  /** More documentation.  */
  virtual const char* GetFullDocumentation()
    {
    return
      "  VTK_MAKE_INSTANTIATOR(className outSourceList\n"
      "                        src-list1 [src-list2 ..]\n"
      "                        EXPORT_MACRO exportMacro\n"
      "                        [HEADER_LOCATION dir]\n"
      "                        [GROUP_SIZE groupSize]\n"
      "                        [INCLUDES [file1 file2 ..]])\n"
      "Generates a new class with the given name and adds its files to the "
      "given outSourceList.  It registers the classes from the other given "
      "source lists with vtkInstantiator when it is loaded.  The output "
      "source list should be added to the library with the classes it "
      "registers.  "
      "The EXPORT_MACRO argument must be given and followed by the export "
      "macro to use when generating the class (ex. VTK_COMMON_EXPORT).  "
      "The HEADER_LOCATION option must be followed by a path.  It specifies "
      "the directory in which to place the generated class's header file. "
      "The generated class implementation files always go in the build "
      "directory corresponding to the CMakeLists.txt file containing "
      "the command.  This is the default location for the header.  "
      "The INCLUDES option can be followed by a list of zero or more files.  "
      "These files will be #included by the generated instantiator header, "
      "and can be used to gain access to the specified exportMacro in the "
      "C++ code.";
    }
  
  cmTypeMacro(cmVTKMakeInstantiatorCommand, cmCommand);
  
protected:
  std::string m_ClassName;
  std::string m_ExportMacro;
  std::vector<cmStdString> m_Includes;
  std::vector<cmStdString> m_Classes;
  
  void GenerateHeaderFile(std::ostream&);
  void GenerateImplementationFile(std::ostream&);
  
  void OldGenerateHeaderFile(std::ostream&);
  void OldGenerateImplementationFile(std::ostream&);
  std::string OldGenerateCreationFileName(unsigned int group);
  void OldGenerateCreationFile(std::ostream&, unsigned int groupStart,
                               unsigned int groupSize);
};


#endif
