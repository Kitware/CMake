/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) 2001 Insight Consortium
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * The name of the Insight Consortium, nor the names of any consortium members,
   nor of any contributors, may be used to endorse or promote products derived
   from this software without specific prior written permission.

  * Modified source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
    return "Setup a library's classes to be created by vtkInstantiator";
    }
  
  /** More documentation.  */
  virtual const char* GetFullDocumentation()
    {
    return
      "VTK_MAKE_INSTANTIATOR(libName srcList exportMacro\n"
      "                      [HEADER_LOCATION dir] [GROUP_SIZE groupSize])\n"
      "Generates a new class for the given library to allow its other\n"
      "classes to be created by vtkInstantiator.  Functions to create\n"
      "classes listed in srcList are registered with vtkInstantiator, and\n"
      "the new class containing this code is added to the srcList for\n"
      "inclusion in the library.  The libName argument is used to generate\n"
      "the filename and name of the class used to register the functions\n"
      "when the library is loaded.  The exportMacro is the name of the\n"
      "DLL export macro to use in the class definition\n"
      "(ex. VTK_COMMON_EXPORT).\n"
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
