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
#ifndef cmCableWrapTclCommand_h
#define cmCableWrapTclCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"
#include "cmCableClassSet.h"

/** \class cmCableWrapTclCommand
 * \brief Define a command that wraps a set of classes in Tcl.
 */
class cmCableWrapTclCommand : public cmCommand
{
public:
  cmCableWrapTclCommand();
  virtual ~cmCableWrapTclCommand();

  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
      return new cmCableWrapTclCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string>& args);  
  
  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "CABLE_WRAP_TCL";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "Wrap a set of classes in Tcl.";
    }
  
  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "CABLE_WRAP_TCL(target class1 class2 ...)\n"
      "Wrap the given set of classes in Tcl using the CABLE tool.  The set\n"
      "of source files produced for the given package name will be added to\n"
      "a source list with the given name.";
    }
  
  cmTypeMacro(cmCableWrapTclCommand, cmCommand);

protected:
  void GenerateCableFiles() const;
  void GenerateCableClassFiles(const char*, const cmCableClass&, unsigned int) const;
  std::string GetGccXmlFromCache() const;
  std::string GetGccXmlFlagsFromCache() const;
  std::string GetCableFromCache() const;
  
private:
  /**
   * The name of the package of wrappers to produce.
   */
  std::string m_TargetName;
  
  /**
   * The name of the source list into which the files needed for the package
   * will be placed.
   */
  std::string m_SourceListName;
  
  /**
   * The set of classes to be wrapped in the package.  This is also implicitly
   * added to the makefile as another set.
   */
  cmCableClassSet* m_CableClassSet;
};

#endif
