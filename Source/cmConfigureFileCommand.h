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
#ifndef cmConfigureFileCommand_h
#define cmConfigureFileCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"

class cmConfigureFileCommand : public cmCommand
{
public:
  virtual cmCommand* Clone() 
    {
      return new cmConfigureFileCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the input file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "CONFIGURE_FILE";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Create a file from an autoconf style file.in file.";
    }
  
  /**
   * Longer documentation.
   */
  virtual const char* GetFullDocumentation()
    {
      return
        "CONFIGURE_FILE(InputFile OutputFile [COPYONLY] [ESCAPE_QUOTES] [IMMEDIATE] [@ONLY])\n"
	"The Input and Ouput files have to have full paths.\n"
	"They can also use variables like CMAKE_BINARY_DIR,CMAKE_SOURCE_DIR. "
        "This command replaces any variables in the input file with their "
        "values as determined by CMake. If a variables in not defined, it "
        "will be replaced with nothing.  If COPYONLY is passed in, then "
        "then no varible expansion will take place. If ESCAPE_QUOTES is "
        "passed in then any substitued quotes will be C style escaped. "
        "If IMMEDIATE is specified, then the file will be configured with "
        "the current values of CMake variables instead of waiting until the "
        "end of CMakeLists processing.  If @ONLY is present, only variables "
        "of the form @var@ will be replaces and ${var} will be ignored. "
        "This is useful for configuring tcl scripts that use ${var}.";
    }

  virtual void FinalPass();
private:
  void ConfigureFile();
  
  std::string m_InputFile;
  std::string m_OuputFile;
  bool m_CopyOnly;
  bool m_EscapeQuotes;
  bool m_Immediate;
  bool m_AtOnly;
};



#endif
