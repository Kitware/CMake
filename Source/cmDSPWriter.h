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
#ifndef cmDSPWriter_h
#define cmDSPWriter_h

#include "cmStandardIncludes.h"
#include "cmMakefile.h"

/** \class cmDSPWriter
 * \brief Generate a Microsoft DSP project file.
 *
 * cmDSPWriter generates a Microsoft DSP project file.
 * See the *.dsptemplate files for information on the templates
 * used for making the project files.
 */
class cmDSPWriter 
{
public:
  cmDSPWriter(cmMakefile*);
  ~cmDSPWriter();
  void OutputDSPFile();
  enum BuildType {STATIC_LIBRARY, DLL, EXECUTABLE, WIN32_EXECUTABLE, UTILITY};

  /**
   * Specify the type of the build: static, dll, or executable.
   */
  void SetBuildType(BuildType,const char *name);

  /**
   * Return array of created DSP names in a STL vector.
   * Each executable must have its own dsp.
   */
  std::vector<std::string> GetCreatedProjectNames() 
    {
    return m_CreatedProjectNames;
    }

  /**
   * Return the makefile.
   */
  cmMakefile* GetMakefile() 
    {
    return m_Makefile;
    }

private:
  std::string m_DSPHeaderTemplate;
  std::string m_DSPFooterTemplate;
  std::vector<std::string> m_CreatedProjectNames;
  
  void CreateSingleDSP(const char *lname, cmTarget &tgt);
  void WriteDSPFile(std::ostream& fout, const char *libName, 
                    cmTarget &tgt);
  void WriteDSPBeginGroup(std::ostream& fout, 
			  const char* group,
			  const char* filter);
  void WriteDSPEndGroup(std::ostream& fout);

  void WriteDSPHeader(std::ostream& fout, const char *libName,
                      const cmTarget &tgt, std::vector<cmSourceGroup> &sgs);

  void WriteDSPFooter(std::ostream& fout);
  void AddDSPBuildRule(cmSourceGroup&);
  void WriteCustomRule(std::ostream& fout,
                       const char* source,
                       const char* command,
                       const std::set<std::string>& depends,
                       const std::set<std::string>& outputs);

  std::string CreateTargetRules(const cmTarget &target, 
                                const char *libName);
  std::string CombineCommands(const cmSourceGroup::Commands &commands,
                              cmSourceGroup::CommandFiles &totalCommand,
                              const char *source);
  

  std::string m_IncludeOptions;
  cmMakefile* m_Makefile;
  std::vector<std::string> m_Configurations;
};

#endif
