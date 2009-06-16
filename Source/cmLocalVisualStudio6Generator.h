/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef cmLocalVisualStudio6Generator_h
#define cmLocalVisualStudio6Generator_h

#include "cmLocalVisualStudioGenerator.h"

class cmTarget;
class cmSourceFile;
class cmSourceGroup;
class cmCustomCommand;

/** \class cmLocalVisualStudio6Generator
 * \brief Write a LocalUnix makefiles.
 *
 * cmLocalVisualStudio6Generator produces a LocalUnix makefile from its
 * member this->Makefile.
 */
class cmLocalVisualStudio6Generator : public cmLocalVisualStudioGenerator
{
public:
  ///! Set cache only and recurse to false by default.
  cmLocalVisualStudio6Generator();

  virtual ~cmLocalVisualStudio6Generator();

  virtual void AddHelperCommands();

  /**
   * Generate the makefile for this directory. 
   */
  virtual void Generate();

  void OutputDSPFile();

  enum BuildType {STATIC_LIBRARY, DLL, EXECUTABLE, WIN32_EXECUTABLE, UTILITY};

  /**
   * Specify the type of the build: static, dll, or executable.
   */
  void SetBuildType(BuildType, const char* libName, cmTarget&);

  /**
   * Return array of created DSP names in a STL vector.
   * Each executable must have its own dsp.
   */
  std::vector<std::string> GetCreatedProjectNames() 
    {
    return this->CreatedProjectNames;
    }
  virtual std::string GetTargetDirectory(cmTarget const& target) const;
  void GetTargetObjectFileDirectories(cmTarget* target,
                                      std::vector<std::string>& 
                                      dirs);
private:
  std::string DSPHeaderTemplate;
  std::string DSPFooterTemplate;
  std::vector<std::string> CreatedProjectNames;
  
  void CreateSingleDSP(const char *lname, cmTarget &tgt);
  void WriteDSPFile(std::ostream& fout, const char *libName, 
                    cmTarget &tgt);
  void WriteDSPBeginGroup(std::ostream& fout, 
                          const char* group,
                          const char* filter);
  void WriteDSPEndGroup(std::ostream& fout);

  void WriteDSPHeader(std::ostream& fout, const char *libName,
                      cmTarget &tgt, std::vector<cmSourceGroup> &sgs);

  void WriteDSPFooter(std::ostream& fout);
  void AddDSPBuildRule(cmTarget& tgt);
  void WriteCustomRule(std::ostream& fout,
                       const char* source,
                       const cmCustomCommand& command,
                       const char* flags);
  void AddUtilityCommandHack(cmTarget& target, int count,
                             std::vector<std::string>& depends,
                             const cmCustomCommand& origCommand);
  void WriteGroup(const cmSourceGroup *sg, cmTarget& target,
                  std::ostream &fout, const char *libName);
  class EventWriter;
  friend class EventWriter;
  std::string CreateTargetRules(cmTarget &target, 
                                const char* configName, 
                                const char *libName);
  void ComputeLinkOptions(cmTarget& target, const char* configName,
                          const std::string extraOptions,
                          std::string& options);
  std::string IncludeOptions;
  std::vector<std::string> Configurations;

  std::string GetConfigName(std::string const& configuration) const;

  // Special definition check for VS6.
  virtual bool CheckDefinition(std::string const& define) const;
};

#endif

