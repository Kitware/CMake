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
#ifndef cmIfCommand_h
#define cmIfCommand_h

#include "cmStandardIncludes.h"
#include "cmCommand.h"
#include "cmFunctionBlocker.h"

/** \class cmIfFunctionBlocker
 * \brief subclass of function blocker
 *
 * 
 */
class cmIfFunctionBlocker : public cmFunctionBlocker
{
public:
  cmIfFunctionBlocker() {m_Not = false;}
  virtual ~cmIfFunctionBlocker() {}
  virtual bool IsFunctionBlocked(const char *name, 
                                 const std::vector<std::string> &args, 
                                 const cmMakefile &mf) const;
  virtual bool ShouldRemove(const char *name, 
                            const std::vector<std::string> &args, 
                            const cmMakefile &mf) const;
  virtual void ScopeEnded(const cmMakefile &mf) const;
  
  std::string m_Define;
  bool m_Not;
};

/** \class cmIfCommand
 * \brief starts an if block
 *
 * cmIfCommand starts an if block
 */
class cmIfCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone() 
    {
    return new cmIfCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool Invoke(std::vector<std::string>& args);

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "IF";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation() 
    {
    return "start an if block";
    }
  
  /**
   * This determines if the command gets propagated down
   * to makefiles located in subdirectories.
   */
  virtual bool IsInherited() {return true;}

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "IF (define) Starts an if block. Optionally there it can be invoked as\n"
      "IF (NOT Define) the matching ELSE and ENDIF require the NOT as well.";
    }
  
  cmTypeMacro(cmIfCommand, cmCommand);
};


#endif
