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
#ifndef cmUnixMakefileGenerator_h
#define cmUnixMakefileGenerator_h

#include "cmMakefile.h"
#include "cmMakefileGenerator.h"

/** \class cmUnixMakefileGenerator
 * \brief Write a Unix makefiles.
 *
 * cmUnixMakefileGenerator produces a Unix makefile from its
 * member m_Makefile.
 */
class cmUnixMakefileGenerator : public cmMakefileGenerator
{
public:
  ///! Set cache only and recurse to false by default.
  cmUnixMakefileGenerator();

  //! just sets the Cache Only and Recurse flags
  virtual void SetLocal(bool local);

  /**
   * If cache only is on.
   * and only stub makefiles are generated, and no depends, for speed.
   * The default is OFF.
   **/
  void SetCacheOnlyOn()  {m_CacheOnly = true;}
  void SetCacheOnlyOff()  {m_CacheOnly = false;}
  /**
   * If recurse is on, then all the makefiles below this one are parsed as well.
   */
  void SetRecurseOn() {m_Recurse = true;}
  void SetRecurseOff() {m_Recurse = false;}
  
  /**
   * Produce the makefile (in this case a Unix makefile).
   */
  virtual void GenerateMakefile();

  /**
   * Output the depend information for all the classes 
   * in the makefile.  These would have been generated
   * by the class cmMakeDepend.
   */
  void OutputObjectDepends(std::ostream&);

  /**
   * Try to determine system infomation such as shared library
   * extension, pthreads, byte order etc.  
   */
  virtual void ComputeSystemInfo();

private:
  void RecursiveGenerateCacheOnly();
  void ProcessDepends(const cmMakeDepend &md);
  void GenerateCacheOnly();
  void OutputMakefile(const char* file);
  void OutputMakeFlags(std::ostream&);
  void OutputTargetRules(std::ostream& fout);
  void OutputLinkLibraries(std::ostream&, const char*, const cmTarget &);
  void OutputTargets(std::ostream&);
  void OutputSubDirectoryRules(std::ostream&);
  void OutputDependInformation(std::ostream&);
  void OutputDependLibs(std::ostream&);
  void OutputLibDepend(std::ostream&, const char*);
  void OutputCustomRules(std::ostream&);
  void OutputMakeVariables(std::ostream&);
  void OutputMakeRules(std::ostream&);
  void OutputInstallRules(std::ostream&);
  void OutputSourceObjectBuildRules(std::ostream& fout);
  void OutputSubDirectoryVars(std::ostream& fout,
                              const char* var,
                              const char* target,
                              const char* target1,
                              const char* target2,
                              const std::vector<std::string>& SubDirectories);
  void OutputMakeRule(std::ostream&, 
                      const char* comment,
                      const char* target,
                      const char* depends, 
                      const char* command);
private:
  bool m_CacheOnly;
  bool m_Recurse;
  std::string m_ExecutableOutputPath;
  std::string m_LibraryOutputPath;
};

#endif
