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
#ifndef cmBorlandMakefileGenerator_h
#define cmBorlandMakefileGenerator_h

#include "cmNMakeMakefileGenerator.h"

/** \class cmBorlandMakefileGenerator
 * \brief Write an NMake makefile.
 *
 * cmBorlandMakefileGenerator produces a Unix makefile from its
 * member m_Makefile.
 */
class cmBorlandMakefileGenerator : public cmNMakeMakefileGenerator
{
public:
  ///! Set cache only and recurse to false by default.
  cmBorlandMakefileGenerator();

  virtual ~cmBorlandMakefileGenerator();
  
  ///! Get the name for the generator.
  virtual const char* GetName() {return "Borland Makefiles";}

  ///! virtual copy constructor
  virtual cmMakefileGenerator* CreateObject() 
    { return new cmBorlandMakefileGenerator;}

  ///! figure out about the current system information
  virtual void ComputeSystemInfo(); 
protected:
  virtual void OutputMakeVariables(std::ostream&);
  virtual void BuildInSubDirectory(std::ostream& fout,
                                   const char* directory,
                                   const char* target1,
                                   const char* target2);
  void OutputMakeRule(std::ostream& fout, 
                      const char* comment,
                      const char* target,
                      const char* depends, 
                      const char* command,
                      const char* command2=0,
                      const char* command3=0,
                      const char* command4=0); 
  
  
  virtual void OutputBuildObjectFromSource(std::ostream& fout,
                                           const char* shortName,
                                           const cmSourceFile& source,
                                           const char* extraCompileFlags,
                                           bool sharedTarget); 
  virtual void OutputSharedLibraryRule(std::ostream&, const char* name,
                                       const cmTarget &);
  virtual void OutputModuleLibraryRule(std::ostream&, const char* name, 
                                       const cmTarget &);
  virtual void OutputStaticLibraryRule(std::ostream&, const char* name,
                                       const cmTarget &);
  virtual void OutputExecutableRule(std::ostream&, const char* name,
                                    const cmTarget &);
  virtual std::string GetOutputExtension(const char* sourceExtension); 
  virtual void OutputBuildLibraryInDir(std::ostream& fout,
				       const char* path,
				       const char* library,
				       const char* fullpath); 
  ///! return true if the two paths are the same (checks short paths)
  virtual bool SamePath(const char* path1, const char* path2);
  virtual std::string ConvertToNativePath(const char* s);

private:
  bool m_QuoteNextCommand;      // if this is true, OutputMakeRule
                                // will not quote the next commands
                                // it is reset to false after each
                                // call to OutputMakeRule
};

#endif
