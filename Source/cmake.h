/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
// This class represents a cmake invocation. It is the top level class when
// running cmake. Most cmake based GUIS should primarily create an instance
// of this class and communicate with it.

#include "cmMakefile.h"
#include "cmStandardIncludes.h"

class cmake
{
 public:

  /**
   * Generate the SourceFilesList from the SourceLists. This should only be
   * done once to be safe.  
   */
  void Usage(const char *program);

  /**
   * Generate the SourceFilesList from the SourceLists. This should only be
   * done once to be safe.  The argument is a list of command line
   * arguments.  The first argument should be the name or full path
   * to the command line version of cmake.  For building a GUI,
   * you would pass in the following arguments:
   * /path/to/cmake -H/path/to/source -B/path/to/build 
   * If you only want to parse the CMakeLists.txt files,
   * but not actually generate the makefiles, use buildMakefiles = false.
   */
  int Generate(const std::vector<std::string>&, bool buildMakefiles = true);

  /** 
   * Execute commands during the build process. Supports options such
   * as echo, remove file etc.
   */
  static int CMakeCommand(std::vector<std::string>&);

  ///! Parse command line arguments
  void SetArgs(cmMakefile& builder, const std::vector<std::string>&);
  ///! Parse command line arguments that might set cache values
  void SetCacheArgs(cmMakefile& builder, const std::vector<std::string>&);

  /**
   * Generate CMAKE_ROOT and CMAKE_COMMAND cache entries
   */
  int AddCMakePaths(const std::vector<std::string>&);

  /**
   * constructor
   */
  cmake();
  ~cmake();

  ///! Create a named generator
  cmMakefileGenerator* CreateGenerator(const char* name);
  ///! Register a generator
  void RegisterGenerator(cmMakefileGenerator*);
  ///! Get the names of the current registered generators
  void GetRegisteredGenerators(std::vector<std::string>& names);

  ///! get the cmCachemManager used by this invocation of cmake
  cmCacheManager *GetCacheManager() {
    return &m_CacheManager; }
  
protected:
  std::map<cmStdString, cmMakefileGenerator*> m_RegisteredGenerators;
  cmCacheManager m_CacheManager;
private:
  bool m_Verbose;
  bool m_Local;
};

