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

#ifndef cmGlobalGenerator_h
#define cmGlobalGenerator_h

#include "cmStandardIncludes.h"

class cmake;
class cmMakefile;
class cmLocalGenerator;
class cmTarget;

/** \class cmGlobalGenerator
 * \brief Responable for overseeing the generation process for the entire tree
 *
 * Subclasses of this class generate makefiles for various
 * platforms.
 */
class cmGlobalGenerator
{
public:
  ///! Free any memory allocated with the GlobalGenerator
  cmGlobalGenerator();
  virtual ~cmGlobalGenerator();
  
  ///! Create a local generator appropriate to this Global Generator
  virtual cmLocalGenerator *CreateLocalGenerator();

  ///! Get the name for this generator
  virtual const char *GetName() const { return "Generic"; };
  
  /** Get the documentation entry for this generator.  */
  virtual void GetDocumentation(cmDocumentationEntry& entry) const;
  
  /**
   * Create LocalGenerators and process the CMakeLists files. This does not
   * actually produce any makefiles, DSPs, etc.  
   */
  virtual void Configure();

  /**
   * Generate the all required files for building this project/tree. This
   * basically creates a series of LocalGenerators for each directory and
   * requests that they Generate.  
   */
  virtual void Generate();

  /**
   * Generate the required files for building this directory. This
   * basically creates a single LocalGenerators and
   * requests that it Generate.  
   */
  virtual void LocalGenerate();

  /**
   * Set/Get and Clear the enabled languages.  
   */
  void SetLanguageEnabled(const char*, cmMakefile* mf);
  bool GetLanguageEnabled(const char*);
  void ClearEnabledLanguages();
  void GetEnabledLanguages(std::vector<std::string>& lang);
  /**
   * Try to determine system infomation such as shared library
   * extension, pthreads, byte order etc.  
   */
  virtual void EnableLanguage(std::vector<std::string>const& languages, cmMakefile *);

  /**
   * Try to determine system infomation, get it from another generator
   */
  virtual void EnableLanguagesFromGenerator(cmGlobalGenerator *gen);

  /**
   * Try running cmake and building a file. This is used for dynalically
   * loaded commands, not as part of the usual build process.
   */
  virtual int TryCompile(const char *srcdir, const char *bindir,
                         const char *projectName, const char *targetName,
                         std::string *output, cmMakefile* mf);

  ///! Set the CMake instance
  void SetCMakeInstance(cmake *cm) {
    this->m_CMakeInstance = cm; };
  
  ///! Get the CMake instance
  cmake *GetCMakeInstance() {
    return this->m_CMakeInstance; };

  void SetConfiguredFilesPath(const char* s){m_ConfiguredFilesPath = s;}
  void GetLocalGenerators(std::vector<cmLocalGenerator *>&g) { g = m_LocalGenerators;}
  static int s_TryCompileTimeout;
  
  bool GetForceUnixPaths() {return m_ForceUnixPaths;}
  ///! return the language for the given extension
  const char* GetLanguageFromExtension(const char* ext);
  ///! is an extension to be ignored
  bool IgnoreFile(const char* ext);
  ///! What is the preference for linkers and this language (None or Prefered)
  const char* GetLinkerPreference(const char* lang);
  ///! What is the output extension for a given language.
  const char* GetLanguageOutputExtensionForLanguage(const char* lang);
  ///! What is the output extension for a given source file extension.
  const char* GetLanguageOutputExtensionFromExtension(const char* lang);
protected:
  // Fill the m_ProjectMap, this must be called after m_LocalGenerators has been populated.
  void FillProjectMap();
  bool IsExcluded(cmLocalGenerator* root, cmLocalGenerator* gen);
  void FindMakeProgram(cmMakefile*);
  
  bool m_ForceUnixPaths;
  cmStdString m_FindMakeProgramFile;
  cmStdString m_ConfiguredFilesPath;
  cmake *m_CMakeInstance;
  std::vector<cmLocalGenerator *> m_LocalGenerators;
  // map from project name to vector of local generators in that project
  std::map<cmStdString, std::vector<cmLocalGenerator*> > m_ProjectMap;

  ///! used by Configure()
  void RecursiveConfigure(cmLocalGenerator *lg, float start, float end);
  
  ///! Find a target by name by searching the local generators.
  cmTarget* FindTarget(const char* name);
private:
  // If you add a new map here, make sure it is copied
  // in EnableLanguagesFromGenerator 
  std::map<cmStdString, bool> m_IgnoreExtensions;
  std::map<cmStdString, bool> m_LanguageEnabled;
  std::map<cmStdString, cmStdString> m_OutputExtensions;
  std::map<cmStdString, cmStdString> m_LanguageToOutputExtension;
  std::map<cmStdString, cmStdString> m_ExtensionToLanguage;
  std::map<cmStdString, cmStdString> m_LanguageToLinkerPreference; 
};

#endif
