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
#ifndef cmMakeDepend_h
#define cmMakeDepend_h

#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmRegularExpression.h"
#include "cmStandardIncludes.h"

/** \class cmDependInformation
 * \brief Store dependency information for a single source file.
 *
 * This structure stores the depend information for a single source file.
 */
class cmDependInformation
{
public:
  /**
   * Construct with dependency generation marked not done; instance
   * not placed in cmMakefile's list.
   */
  cmDependInformation() 
    {
    m_DependDone = false;
    m_ClassFileIndex = 0;
    }

  /**
   * A set of indices into the m_DependInformation array of cmMakeDepend.
   * The index represents the files that this file depends on.
   * This must be a "set" to keep indices unique.
   */
  typedef std::set<int> IndexSet;
  IndexSet m_IndexSet;	

  /**
   * Full path to this file.
   */
  std::string m_FullPath;	

  /**
   * Name that the include directive uses.
   */
  std::string m_IncludeName;

  /**
   * The index into the cmMakefile::m_Classes list.
   * The index value of 0 indicates that it is not in the list.
   */
  const cmSourceFile *m_ClassFileIndex;	
  
  /**
   * This flag indicates whether dependency checking has been
   * performed for this file.
   */
  bool m_DependDone;
  
  /**
   * This method adds the dependencies of another file to this one.
   */
  void MergeInfo(cmDependInformation*);
};


// cmMakeDepend is used to generate dependancy information for
// the classes in a makefile
class cmMakeDepend
{
public:
  /**
   * Construct the object with verbose turned off.
   */
  cmMakeDepend();

  /**
   * Destructor.
   */
  virtual ~cmMakeDepend();
  
  /** 
   * Set the makefile that is used as a source of classes.
   */
  virtual void SetMakefile(const cmMakefile* makefile); 

  /** 
   * Generate the depend information
   */
  virtual void GenerateDependInformation();

  /** 
   * Get the depend info struct for a source file
   */
  const cmDependInformation *GetDependInformationForSourceFile(const cmSourceFile &sf) const;
  const cmDependInformation *GetDependInformationForSourceFile(const char *) const;

  /** 
   * Get the depend info struct
   */
  typedef std::vector<cmDependInformation*> DependArray;
  const DependArray &GetDependInformation() const { 
    return m_DependInformation; }

  /**
   * Add a directory to the search path for include files.
   */
  virtual void AddSearchPath(const char*);

protected: 
  /**
   * Add a source file to the search path.
   */
  void AddFileToSearchPath(const char* filepath);

  /**
   * Find the index into the m_DependInformation array
   * that matches the given m_IncludeName.
   */
  virtual int FindInformation(const char* includeName);

  /**
   * Compute the depend information for this class.
   */
  virtual void Depend(cmDependInformation* info);

  /**
   * Compute the depend information for this class.
   */
  virtual void DependWalk(cmDependInformation* info, const char* file);
  
  /**
   * Add a dependency.  Possibly walk it for more dependencies.
   */
  virtual void AddDependency(cmDependInformation* info, const char* file);

  /** 
   * Find the full path name for the given file name.
   * This uses the include directories.
   */
  std::string FullPath(const char*);

  const cmMakefile* m_Makefile;
  bool m_Verbose;
  cmRegularExpression m_IncludeFileRegularExpression;
  cmRegularExpression m_ComplainFileRegularExpression;
  DependArray m_DependInformation;
  std::vector<std::string> m_IncludeDirectories;
};

#endif
