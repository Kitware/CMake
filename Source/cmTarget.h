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
  enum TargetType { EXECUTABLE, WIN32_EXECUTABLE, LIBRARY, UTILITY, INSTALL };

  /**
   * Return the type of target.
   */
  TargetType GetType() const
    {
      return m_TargetType;
    }
  
  void SetType(TargetType f) { m_TargetType = f; }
  
  /**
   * Indicate whether the target is part of the all target
   */
  bool IsInAll() const { return m_InAll; }
  bool GetInAll() const { return m_InAll; }
  void SetInAll(bool f) { m_InAll = f; }

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
   * Get the list of the source files used by this target
   */
  enum LinkLibraryType {GENERAL, DEBUG, OPTIMIZED};
  typedef std::vector<std::pair<std::string,LinkLibraryType> > LinkLibraries;
  const LinkLibraries &GetLinkLibraries() const {return m_LinkLibraries;}
  LinkLibraries &GetLinkLibraries() {return m_LinkLibraries;}

  /**
   * Set the path where this target should be installed. This is relative to
   * INSTALL_PREFIX
   */
  std::string GetInstallPath() const {return m_InstallPath;}
  void SetInstallPath(const char *name) {m_InstallPath = name;}
  
  /**
   * Merge Link Libraries into this targets current list 
   */
  void MergeLibraries(const LinkLibraries &ll);
    
  /**
   * Generate the SourceFilesList from the SourceLists. This should only be
   * done once to be safe.  
   */
  void GenerateSourceFilesFromSourceLists(const cmMakefile &mf);

  /** Add a utility on which this project depends. A utility is an executable
   * name as would be specified to the ADD_EXECUTABLE or UTILITY_SOURCE
   * commands. It is not a full path nor does it have an extension.  
   */
  void AddUtility(const char* u) { m_Utilities.insert(u);}
  ///! Get the utilities used by this target
  std::set<std::string>const& GetUtilities() const { return m_Utilities; }

private:
  std::vector<cmCustomCommand> m_CustomCommands;
  std::vector<std::string> m_SourceLists;
  TargetType m_TargetType;
  std::vector<cmSourceFile> m_SourceFiles;
  LinkLibraries m_LinkLibraries;
  bool m_InAll;
  std::string m_InstallPath;
  std::set<std::string> m_Utilities;
};

typedef std::map<std::string,cmTarget> cmTargets;

#endif
