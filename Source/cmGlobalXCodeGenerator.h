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
#ifndef cmGlobalXCodeGenerator_h
#define cmGlobalXCodeGenerator_h

#include "cmGlobalGenerator.h"
#include "cmXCodeObject.h"
#include "cmCustomCommand.h"
class cmTarget;
class cmSourceFile;
class cmSourceGroup;


/** \class cmGlobalXCodeGenerator
 * \brief Write a Unix makefiles.
 *
 * cmGlobalXCodeGenerator manages UNIX build process for a tree
 */
class cmGlobalXCodeGenerator : public cmGlobalGenerator
{
public:
  cmGlobalXCodeGenerator();
  static cmGlobalGenerator* New();
  
  ///! Get the name for the generator.
  virtual const char* GetName() const {
    return cmGlobalXCodeGenerator::GetActualName();}
  static const char* GetActualName() {return "Xcode";}

  /** Get the documentation entry for this generator.  */
  virtual void GetDocumentation(cmDocumentationEntry& entry) const;
  
  ///! Create a local generator appropriate to this Global Generator
  virtual cmLocalGenerator *CreateLocalGenerator();

  /**
   * Try to determine system infomation such as shared library
   * extension, pthreads, byte order etc.  
   */
  virtual void EnableLanguage(std::vector<std::string>const& languages, 
                              cmMakefile *);
  /**
   * Try running cmake and building a file. This is used for dynalically
   * loaded commands, not as part of the usual build process.
   */
  virtual std::string GenerateBuildCommand(const char* makeProgram,
    const char *projectName, const char *targetName, const char* config,
    bool ignoreErrors);

  /**
   * Generate the all required files for building this project/tree. This
   * basically creates a series of LocalGenerators for each directory and
   * requests that they Generate.  
   */
  virtual void Generate();

private: 
  cmXCodeObject* CreateOrGetPBXGroup(cmTarget& cmtarget,
                                     cmSourceGroup* sg);
  void CreateGroups(cmLocalGenerator* root,
                    std::vector<cmLocalGenerator*>&
                    generators);
  void SetCurrentLocalGenerator(cmLocalGenerator*);
  std::string XCodeEscapePath(const char* p);
  std::string ConvertToRelativeForXCode(const char* p);
  std::string ConvertToRelativeForMake(const char* p);
  void CreateCustomCommands(cmXCodeObject* buildPhases,
                            cmXCodeObject* sourceBuildPhase,
                            cmXCodeObject* headerBuildPhase,
                            cmXCodeObject* frameworkBuildPhase,
                            cmTarget& cmtarget);
  
  void AddCommandsToBuildPhase(cmXCodeObject* buildphase,
                               cmTarget& target,
                               std::vector<cmCustomCommand> 
                               const & commands,
                               const char* commandFileName);
  cmXCodeObject* FindXCodeTarget(cmTarget*);
  // create cmXCodeObject from these functions so that memory can be managed
  // correctly.  All objects created are stored in m_XCodeObjects.
  cmXCodeObject* CreateObject(cmXCodeObject::PBXType ptype);
  cmXCodeObject* CreateObject(cmXCodeObject::Type type);
  cmXCodeObject* CreateString(const char* s);
  cmXCodeObject* CreateObjectReference(cmXCodeObject*);
  cmXCodeObject* CreateXCodeTarget(cmTarget& target,
                                   cmXCodeObject* buildPhases);
  cmXCodeObject* CreateUtilityTarget(cmTarget& target);
  void AddDependAndLinkInformation(cmXCodeObject* target);
  void CreateBuildSettings(cmTarget& target,
                           cmXCodeObject* buildSettings,
                           std::string& fileType,
                           std::string& productType,
                           std::string& projectName);
  
  // delete all objects in the m_XCodeObjects vector.
  void ClearXCodeObjects();
  void CreateXCodeObjects(cmLocalGenerator* root,
                          std::vector<cmLocalGenerator*>& generators);
  void OutputXCodeProject(cmLocalGenerator* root,
                          std::vector<cmLocalGenerator*>& generators);
  void  WriteXCodePBXProj(std::ostream& fout,
                          cmLocalGenerator* root,
                          std::vector<cmLocalGenerator*>& generators);
  cmXCodeObject* CreateXCodeSourceFile(cmLocalGenerator* gen, 
                                       cmSourceFile* sf);
  void CreateXCodeTargets(cmLocalGenerator* gen, std::vector<cmXCodeObject*>&);
  void AddDependTarget(cmXCodeObject* target,
                       cmXCodeObject* dependTarget);
  void AddLinkLibrary(cmXCodeObject* target,
                      const char* lib, cmTarget* dtarget = 0);
  void ConfigureOutputPaths();
  void CreateXCodeDependHackTarget(std::vector<cmXCodeObject*>& targets);
  std::string GetTargetFullPath(cmTarget*);
  bool SpecialTargetEmitted(std::string const& tname);
  void AddExtraTargets(cmLocalGenerator* root, 
                       std::vector<cmLocalGenerator*>& gens);
  cmXCodeObject* CreateBuildPhase(const char* name, 
                                  const char* name2,
                                  cmTarget& cmtarget,
                                  const std::vector<cmCustomCommand>&);
  void CreateReRunCMakeFile(cmLocalGenerator* root);
protected:
  int m_XcodeVersion;
private:
  std::vector<cmXCodeObject*> m_XCodeObjects;
  cmXCodeObject* m_RootObject;
  cmXCodeObject* m_MainGroupChildren;
  cmXCodeObject* m_SourcesGroupChildren;
  cmMakefile* m_CurrentMakefile;
  cmLocalGenerator* m_CurrentLocalGenerator;
  std::string m_CurrentReRunCMakeMakefile;
  std::string m_CurrentXCodeHackMakefile;
  std::string m_CurrentProject;
  std::string m_OutputDir; 
  std::string m_LibraryOutputPath;
  std::string m_ExecutableOutputPath;
  std::set<cmStdString> m_TargetDoneSet;
  std::vector<std::string> m_CurrentOutputDirectoryComponents;
  std::vector<std::string> m_ProjectOutputDirectoryComponents;
  std::map<cmSourceFile*, cmXCodeObject* > m_GroupMap;
  std::map<cmStdString, cmXCodeObject* > m_GroupNameMap;
  std::map<cmStdString, cmXCodeObject* > m_TargetGroup;
};

#endif
