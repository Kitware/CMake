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
    return "Register classes for creation by vtkInstantiator";
    }
  
  /** More documentation.  */
  virtual const char* GetFullDocumentation()
    {
    return
      "VTK_MAKE_INSTANTIATOR(className outSourceList\n"
      "                      src-list1 [src-list2 ..]\n"
      "                      EXPORT_MACRO exportMacro\n"
      "                      [HEADER_LOCATION dir] [GROUP_SIZE groupSize])\n"
      "Generates a new class with the given name and adds its files to the\n"
      "given outSourceList.  It registers the classes from the other given\n"
      "source lists with vtkInstantiator when it is loaded.  The output\n"
      "source list should be added to the library with the classes it\n"
      "registers.\n"
      "The EXPORT_MACRO argument must be given and followed by the export\n"
      "macro to use when generating the class (ex. VTK_COMMON_EXPORT).\n"
      "The HEADER_LOCATION option must be followed by a path.  It specifies\n"
      "the directory in which to place the generated class's header file.\n"
      "The generated class implementation files always go in the build\n"
      "directory corresponding to the CMakeLists.txt file containing\n"
      "the command.  This is the default location for the header.\n"
      "The GROUP_SIZE option must be followed by a positive integer.\n"
      "As an implementation detail, the registered creation functions may\n"
      "be split up into multiple files.  The groupSize option specifies\n"
      "the number of classes per file.  Its default is 10.";
    }
  
  cmTypeMacro(cmVTKMakeInstantiatorCommand, cmCommand);
  
protected:
  std::string m_ClassName;
  std::string m_ExportMacro;
  std::vector<cmStdString> m_Classes;
  
  std::string GenerateCreationFileName(unsigned int group);
  
  void GenerateHeaderFile(std::ostream&);
  void GenerateImplementationFile(std::ostream&);
  void GenerateCreationFile(std::ostream&, unsigned int groupStart,
                            unsigned int groupSize);
};


#endif
