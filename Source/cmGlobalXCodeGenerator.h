/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
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
  cmGlobalXCodeGenerator(std::string const& version);
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
                              cmMakefile *, bool optional);
  /**
   * Try running cmake and building a file. This is used for dynalically
   * loaded commands, not as part of the usual build process.
   */
  virtual std::string GenerateBuildCommand(const char* makeProgram,
                                           const char *projectName,
                                           const char* additionalOptions, 
                                           const char *targetName,
                                           const char* config, 
                                           bool ignoreErrors,
                                           bool fast);

  /**
   * Generate the all required files for building this project/tree. This
   * basically creates a series of LocalGenerators for each directory and
   * requests that they Generate.  
   */
  virtual void Generate();

  /** Append the subdirectory for the given configuration.  */
  virtual void AppendDirectoryForConfig(const char* prefix,
                                        const char* config,
                                        const char* suffix,
                                        std::string& dir);

  ///! What is the configurations directory variable called?
  virtual const char* GetCMakeCFGIntDir() const;

  void SetCurrentLocalGenerator(cmLocalGenerator*);

  /** Return true if the generated build tree may contain multiple builds.
      i.e. "Can I build Debug and Release in the same tree?" */
  virtual bool IsMultiConfig();

private: 
  cmXCodeObject* CreateOrGetPBXGroup(cmTarget& cmtarget,
                                     cmSourceGroup* sg);
  cmXCodeObject* CreatePBXGroup(cmXCodeObject *parent,
                                cmStdString name);
  void CreateGroups(cmLocalGenerator* root,
                    std::vector<cmLocalGenerator*>&
                    generators);
  std::string XCodeEscapePath(const char* p);
  std::string RelativeToSource(const char* p);
  std::string RelativeToBinary(const char* p);
  std::string ConvertToRelativeForXCode(const char* p);
  std::string ConvertToRelativeForMake(const char* p);
  void CreateCustomCommands(cmXCodeObject* buildPhases,
                            cmXCodeObject* sourceBuildPhase,
                            cmXCodeObject* headerBuildPhase,
                            cmXCodeObject* resourceBuildPhase,
                            std::vector<cmXCodeObject*> contentBuildPhases,
                            cmXCodeObject* frameworkBuildPhase,
                            cmTarget& cmtarget);

  std::string ComputeInfoPListLocation(cmTarget& target);

  void AddCommandsToBuildPhase(cmXCodeObject* buildphase,
                               cmTarget& target,
                               std::vector<cmCustomCommand> 
                               const & commands,
                               const char* commandFileName);
  
  void CreateCustomRulesMakefile(const char* makefileBasename, 
                                 cmTarget& target,
                                 std::vector<cmCustomCommand> const & commands,
                                 const char* configName,
                                 const std::map<cmStdString, cmStdString>& 
                                     multipleOutputPairs
                                );
  
  cmXCodeObject* FindXCodeTarget(cmTarget*);
  std::string GetOrCreateId(const char* name, const char* id);

  // create cmXCodeObject from these functions so that memory can be managed
  // correctly.  All objects created are stored in this->XCodeObjects.
  cmXCodeObject* CreateObject(cmXCodeObject::PBXType ptype);
  cmXCodeObject* CreateObject(cmXCodeObject::Type type);
  cmXCodeObject* CreateString(const char* s);
  cmXCodeObject* CreateObjectReference(cmXCodeObject*);
  cmXCodeObject* CreateXCodeTarget(cmTarget& target,
                                   cmXCodeObject* buildPhases);
  void ForceLinkerLanguages();
  void ForceLinkerLanguage(cmTarget& cmtarget);
  const char* GetTargetFileType(cmTarget& cmtarget);
  const char* GetTargetProductType(cmTarget& cmtarget);
  std::string AddConfigurations(cmXCodeObject* target, cmTarget& cmtarget);
  void AppendOrAddBuildSetting(cmXCodeObject* settings, const char* attr, 
                               const char* value);
  void AppendBuildSettingAttribute(cmXCodeObject* target, const char* attr, 
                                   const char* value, const char* configName);
  cmXCodeObject* CreateUtilityTarget(cmTarget& target);
  void AddDependAndLinkInformation(cmXCodeObject* target);
  void CreateBuildSettings(cmTarget& target,
                           cmXCodeObject* buildSettings,
                           const char* buildType);
  std::string ExtractFlag(const char* flag, std::string& flags);
  // delete all objects in the this->XCodeObjects vector.
  void ClearXCodeObjects();
  void CreateXCodeObjects(cmLocalGenerator* root,
                          std::vector<cmLocalGenerator*>& generators);
  void OutputXCodeProject(cmLocalGenerator* root,
                          std::vector<cmLocalGenerator*>& generators);
  void WriteXCodePBXProj(std::ostream& fout, cmLocalGenerator* root,
                         std::vector<cmLocalGenerator*>& generators);
  cmXCodeObject* CreateXCodeFileReferenceFromPath(const std::string &fullpath,
                                                  cmTarget& cmtarget,
                                                  const std::string &lang);
  cmXCodeObject* CreateXCodeSourceFileFromPath(const std::string &fullpath,
                                               cmTarget& cmtarget,
                                               const std::string &lang);
  cmXCodeObject* CreateXCodeFileReference(cmSourceFile* sf,
                                          cmTarget& cmtarget);
  cmXCodeObject* CreateXCodeSourceFile(cmLocalGenerator* gen, 
                                       cmSourceFile* sf,
                                       cmTarget& cmtarget);
  void CreateXCodeTargets(cmLocalGenerator* gen, 
                          std::vector<cmXCodeObject*>&);
  bool IsHeaderFile(cmSourceFile*);
  void AddDependTarget(cmXCodeObject* target,
                       cmXCodeObject* dependTarget);
  void CreateXCodeDependHackTarget(std::vector<cmXCodeObject*>& targets);
  bool SpecialTargetEmitted(std::string const& tname);
  void SetGenerationRoot(cmLocalGenerator* root);
  void AddExtraTargets(cmLocalGenerator* root, 
                       std::vector<cmLocalGenerator*>& gens);
  cmXCodeObject* CreateBuildPhase(const char* name, 
                                  const char* name2,
                                  cmTarget& cmtarget,
                                  const std::vector<cmCustomCommand>&);
  void CreateReRunCMakeFile(cmLocalGenerator* root,
                            std::vector<cmLocalGenerator*> const& gens);

  std::string LookupFlags(const char* varNamePrefix,
                          const char* varNameLang,
                          const char* varNameSuffix,
                          const char* default_flags);

  class BuildObjectListOrString;
  friend class BuildObjectListOrString;

  void AppendDefines(BuildObjectListOrString& defs, const char* defines_list,
                     bool dflag = false);
  void AppendDefines(BuildObjectListOrString& defs,
                     std::vector<std::string> const& defines,
                     bool dflag = false);
  void AppendFlag(std::string& flags, std::string const& flag);

protected:
  virtual const char* GetInstallTargetName() const { return "install"; }
  virtual const char* GetPackageTargetName() const { return "package"; }

  unsigned int XcodeVersion;
  std::string VersionString;
  std::set<cmStdString> XCodeObjectIDs;
  std::vector<cmXCodeObject*> XCodeObjects;
  cmXCodeObject* RootObject;
private:
  void ComputeTargetObjects(cmGeneratorTarget* gt) const;

  std::string GetObjectsNormalDirectory(
    const std::string &projName,
    const std::string &configName,
    const cmTarget *t) const;

  void addObject(cmXCodeObject *obj);
  std::string PostBuildMakeTarget(std::string const& tName,
                                  std::string const& configName);
  cmXCodeObject* MainGroupChildren;
  cmXCodeObject* SourcesGroupChildren;
  cmXCodeObject* ResourcesGroupChildren;
  cmMakefile* CurrentMakefile;
  cmLocalGenerator* CurrentLocalGenerator;
  std::vector<std::string> CurrentConfigurationTypes;
  std::string CurrentReRunCMakeMakefile;
  std::string CurrentXCodeHackMakefile;
  std::string CurrentProject;
  std::set<cmStdString> TargetDoneSet;
  std::vector<std::string> CurrentOutputDirectoryComponents;
  std::vector<std::string> ProjectSourceDirectoryComponents;
  std::vector<std::string> ProjectOutputDirectoryComponents;
  std::map<cmStdString, cmXCodeObject* > GroupMap;
  std::map<cmStdString, cmXCodeObject* > GroupNameMap;
  std::map<cmStdString, cmXCodeObject* > TargetGroup;
  std::map<cmStdString, cmXCodeObject* > FileRefs;
  std::vector<std::string> Architectures;
};

#endif
