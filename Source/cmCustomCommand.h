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
#ifndef cmCustomCommand_h
#define cmCustomCommand_h

#include "cmStandardIncludes.h"
class cmMakefile;

/** \class cmCustomCommand
 * \brief A class to encapsulate a custom command
 *
 * cmCustomCommand encapsulates the properties of a custom command
 */
class cmCustomCommand
{
public:
  cmCustomCommand(const char *src, const char *command,
                  std::vector<std::string> dep,
                  std::vector<std::string> out);
  cmCustomCommand(const cmCustomCommand& r);
  
  /**
   * Use the cmMakefile's Expand commands to expand any variables in
   * this objects members.
   */
  void ExpandVariables(const cmMakefile &);

  /**
   * Return the name of the source file. I'm not sure if this is a full path or not.
   */
  std::string GetSourceName() const {return m_Source;}
  void SetSourceName(const char *name) {m_Source = name;}

  /**
   * Return the command to execute
   */
  std::string GetCommand() const {return m_Command;}
  void SetCommand(const char *cmd) {m_Command = cmd;}
  
  /**
   * Return the vector that holds the list of dependencies
   */
  const std::vector<std::string> &GetDepends() const {return m_Depends;}
  std::vector<std::string> &GetDepends() {return m_Depends;}
  
  /**
   * Return the vector that holds the list of outputs of this command
   */
  const std::vector<std::string> &GetOutputs() const {return m_Outputs;}
  std::vector<std::string> &GetOutputs() {return m_Outputs;}
  
private:
  std::string m_Source;
  std::string m_Command;
  std::vector<std::string> m_Depends;
  std::vector<std::string> m_Outputs;
};


#endif
