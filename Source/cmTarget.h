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
#ifndef cmTarget_h
#define cmTarget_h

#include "cmStandardIncludes.h"
#include "cmCustomCommand.h"
#include "cmSourceFile.h"

/** \class cmTarget
 * \brief Represent a library or executable target loaded from a makefile.
 *
 * cmTarget represents a target loaded from 
 * a makefile.
 */
class cmTarget
{
public:
  /**
   * is this target a library?
   */
  bool IsALibrary() const { return m_IsALibrary; }
  bool GetIsALibrary() const { return m_IsALibrary; }
  void SetIsALibrary(bool f) { m_IsALibrary = f; }
  
  /**
   * Get the list of the custom commands for this target
   */
  const std::vector<cmCustomCommand> &GetCustomCommands() const {return m_CustomCommands;}
  std::vector<cmCustomCommand> &GetCustomCommands() {return m_CustomCommands;}

  /**
   * Get the list of the source lists used by this target
   */
  const std::vector<std::string> &GetSourceLists() const 
    {return m_SourceLists;}
  std::vector<std::string> &GetSourceLists() {return m_SourceLists;}
  
  /**
   * Get the list of the source files used by this target
   */
  const std::vector<cmSourceFile> &GetSourceFiles() const 
    {return m_SourceFiles;}
  std::vector<cmSourceFile> &GetSourceFiles() {return m_SourceFiles;}

  /**
   * Generate the SourceFilesList from the SourceLists. This should only be
   * done once to be safe.  
   */
  void GenerateSourceFilesFromSourceLists(const cmMakefile &mf);

private:
  std::vector<cmCustomCommand> m_CustomCommands;
  std::vector<std::string> m_SourceLists;
  bool m_IsALibrary;
  std::vector<cmSourceFile> m_SourceFiles;
};

typedef std::map<std::string,cmTarget> cmTargets;

#endif
