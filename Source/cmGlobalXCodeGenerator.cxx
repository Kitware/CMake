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
#include "cmGlobalXCodeGenerator.h"
#include "cmLocalXCodeGenerator.h"
#include "cmMakefile.h"
#include "cmXCodeObject.h"
#include "cmake.h"
#include "cmGeneratedFileStream.h"
#include "cmSourceFile.h"

//----------------------------------------------------------------------------
cmGlobalXCodeGenerator::cmGlobalXCodeGenerator()
{
  m_FindMakeProgramFile = "CMakeFindXCode.cmake";
  m_RootObject = 0;
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::EnableLanguage(std::vector<std::string>const& lang,
                                            cmMakefile * mf)
{ 
  mf->AddDefinition("CMAKE_GENERATOR_CC", "gcc");
  mf->AddDefinition("CMAKE_GENERATOR_CXX", "g++");
  mf->AddDefinition("CMAKE_GENERATOR_NO_COMPILER_ENV", "1");
  this->cmGlobalGenerator::EnableLanguage(lang, mf);
}

//----------------------------------------------------------------------------
int cmGlobalXCodeGenerator::TryCompile(const char *, 
                                       const char * bindir, 
                                       const char * projectName,
                                       const char * targetName,
                                       std::string * output,
                                       cmMakefile*)
{
  // now build the test
  std::string makeCommand = 
    m_CMakeInstance->GetCacheManager()->GetCacheValue("CMAKE_MAKE_PROGRAM");
  if(makeCommand.size() == 0)
    {
    cmSystemTools::Error(
      "Generator cannot find the appropriate make command.");
    return 1;
    }
  makeCommand = cmSystemTools::ConvertToOutputPath(makeCommand.c_str());
  std::string lowerCaseCommand = makeCommand;
  cmSystemTools::LowerCase(lowerCaseCommand);

  /**
   * Run an executable command and put the stdout in output.
   */
  std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
  cmSystemTools::ChangeDirectory(bindir);
  std::cerr << "ChangeDirectory " << bindir << "\n";
//  Usage: xcodebuild [-project <projectname>] [-activetarget] 
//         [-alltargets] [-target <targetname>]... [-activebuildstyle] 
//         [-buildstyle <buildstylename>] [-optionalbuildstyle <buildstylename>] 
//         [<buildsetting>=<value>]... [<buildaction>]...
//         xcodebuild [-list]

  makeCommand += " -project ";
  makeCommand += projectName;
  makeCommand += ".xcode";
  makeCommand += " build ";
  if (targetName)
    {
    makeCommand += "-target ";
    makeCommand += targetName;
    }
  makeCommand += " -buildstyle Development ";
  int retVal;
  int timeout = cmGlobalGenerator::s_TryCompileTimeout;
  if (!cmSystemTools::RunSingleCommand(makeCommand.c_str(), output, &retVal, 
                                       0, false, timeout))
    {
    cmSystemTools::Error("Generator: execution of xcodebuild failed.");
    // return to the original directory
    cmSystemTools::ChangeDirectory(cwd.c_str());
    return 1;
    }
  std::cerr << makeCommand << "\n";
  std::cerr << "build worked " << retVal << "\n";;
  cmSystemTools::ChangeDirectory(cwd.c_str());
  return retVal;
}

//----------------------------------------------------------------------------
///! Create a local generator appropriate to this Global Generator
cmLocalGenerator *cmGlobalXCodeGenerator::CreateLocalGenerator()
{
  cmLocalGenerator *lg = new cmLocalXCodeGenerator;
  lg->SetGlobalGenerator(this);
  return lg;
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::Generate()
{
  this->cmGlobalGenerator::Generate();
  std::map<cmStdString, std::vector<cmLocalGenerator*> >::iterator it;
  for(it = m_ProjectMap.begin(); it!= m_ProjectMap.end(); ++it)
    {
    this->OutputXCodeProject(it->second[0], it->second);
    }
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::ClearXCodeObjects()
{
  for(unsigned int i = 0; i < m_XCodeObjects.size(); ++i)
    {
    delete m_XCodeObjects[i];
    }
  m_XCodeObjects.clear();
}

//----------------------------------------------------------------------------
cmXCodeObject* 
cmGlobalXCodeGenerator::CreateObject(cmXCodeObject::PBXType ptype)
{
  cmXCodeObject* obj = new cmXCodeObject(ptype, cmXCodeObject::OBJECT);
  m_XCodeObjects.push_back(obj);
  return obj;
}

//----------------------------------------------------------------------------
cmXCodeObject* 
cmGlobalXCodeGenerator::CreateObject(cmXCodeObject::Type type)
{
  cmXCodeObject* obj = new cmXCodeObject(cmXCodeObject::None, type);
  m_XCodeObjects.push_back(obj);
  return obj;
}

cmXCodeObject*
cmGlobalXCodeGenerator::CreateString(const char* s)
{
  cmXCodeObject* obj = this->CreateObject(cmXCodeObject::STRING);
  obj->SetString(s);
  return obj;
}
cmXCodeObject* cmGlobalXCodeGenerator::CreateObjectReference(cmXCodeObject* ref)
{
  cmXCodeObject* obj = this->CreateObject(cmXCodeObject::OBJECT_REF);
  obj->SetObject(ref);
  return obj;
}

cmXCodeObject* 
cmGlobalXCodeGenerator::CreateXCodeSourceFile(cmLocalGenerator* lg, 
                                              cmSourceFile* sf,
                                              cmXCodeObject* mainGroupChildren)
{
  cmXCodeObject* fileRef = this->CreateObject(cmXCodeObject::PBXFileReference);
  mainGroupChildren->AddObject(fileRef);
  cmXCodeObject* buildFile = this->CreateObject(cmXCodeObject::PBXBuildFile);
  buildFile->AddAttribute("fileRef", this->CreateObjectReference(fileRef));
  cmXCodeObject* settings = this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
  buildFile->AddAttribute("settings", settings);
  fileRef->AddAttribute("fileEncoding", this->CreateString("4"));
  std::string lang = 
    this->GetLanguageFromExtension(sf->GetSourceExtension().c_str());
  std::string sourcecode = "sourcecode";
  if(lang == "C")
    {
      sourcecode += ".c.c";
    }
  // default to c++
  else
    {
      sourcecode += ".cpp.cpp";
    }

  fileRef->AddAttribute("lastKnownFileType", 
                        this->CreateString(sourcecode.c_str()));
  fileRef->AddAttribute("path", this->CreateString(
    lg->ConvertToRelativeOutputPath(sf->GetFullPath().c_str()).c_str()));
  fileRef->AddAttribute("refType", this->CreateString("4"));
  fileRef->AddAttribute("sourceTree", this->CreateString("\"<absolute>\""));
  return buildFile;
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::CreateXCodeTargets(cmLocalGenerator* gen,
                                                std::vector<cmXCodeObject*>& targets,
                                                cmXCodeObject* mainGroupChildren)
{
  cmTargets &tgts = gen->GetMakefile()->GetTargets();
  for(cmTargets::iterator l = tgts.begin(); l != tgts.end(); l++)
    { 
    // create source build phase
    cmXCodeObject* sourceBuildPhase = 
      this->CreateObject(cmXCodeObject::PBXSourcesBuildPhase);
    sourceBuildPhase->AddAttribute("buildActionMask", 
                                   this->CreateString("2147483647"));
    cmXCodeObject* buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
    sourceBuildPhase->AddAttribute("files", buildFiles);
    sourceBuildPhase->AddAttribute("runOnlyForDeploymentPostprocessing", 
                                   this->CreateString("0"));
    std::vector<cmSourceFile*> &classes = l->second.GetSourceFiles();
    // add all the sources
    for(std::vector<cmSourceFile*>::iterator i = classes.begin(); 
        i != classes.end(); ++i)
      {
      buildFiles->AddObject(this->CreateXCodeSourceFile(gen, *i, 
                                                        mainGroupChildren));
      }
    // create header build phase
    cmXCodeObject* headerBuildPhase = 
      this->CreateObject(cmXCodeObject::PBXHeadersBuildPhase);
    headerBuildPhase->AddAttribute("buildActionMask",
                                   this->CreateString("2147483647"));
    buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
    headerBuildPhase->AddAttribute("files", buildFiles);
    headerBuildPhase->AddAttribute("runOnlyForDeploymentPostprocessing",
                                   this->CreateString("0"));
    
    // create framework build phase
    cmXCodeObject* frameworkBuildPhase =
      this->CreateObject(cmXCodeObject::PBXFrameworksBuildPhase);
    frameworkBuildPhase->AddAttribute("buildActionMask",
                                      this->CreateString("2147483647"));
    buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
    frameworkBuildPhase->AddAttribute("files", buildFiles);
    frameworkBuildPhase->AddAttribute("runOnlyForDeploymentPostprocessing", 
                                      this->CreateString("0"));

    cmXCodeObject* buildPhases = this->CreateObject(cmXCodeObject::OBJECT_LIST);
    buildPhases->AddObject(sourceBuildPhase);
    buildPhases->AddObject(headerBuildPhase);
    buildPhases->AddObject(frameworkBuildPhase);
    switch(l->second.GetType())
      {
      case cmTarget::STATIC_LIBRARY:
        targets.push_back(this->CreateStaticLibrary(l->second, buildPhases));
        break;
      case cmTarget::SHARED_LIBRARY:
        targets.push_back(this->CreateSharedLibrary(l->second, buildPhases));
        break;
      case cmTarget::MODULE_LIBRARY:
        targets.push_back(this->CreateSharedLibrary(l->second, buildPhases));
        break;
      case cmTarget::EXECUTABLE:  
        targets.push_back(this->CreateExecutable(l->second, buildPhases));
        break;
      case cmTarget::UTILITY:
        break;
      case cmTarget::INSTALL_FILES:
        break;
      case cmTarget::INSTALL_PROGRAMS:
        break;
      }
    }
}

// to force the location of a target
//6FE4372B07AAF276004FB461 = {
//buildSettings = {
//COPY_PHASE_STRIP = NO;
//SYMROOT = "/Users/kitware/Bill/CMake-build/test/build/bin";
//};
//isa = PBXBuildStyle;
//name = Development;
//};
cmXCodeObject* cmGlobalXCodeGenerator::CreateExecutable(cmTarget& cmtarget,
                                                        cmXCodeObject* buildPhases)
{
  cmXCodeObject* target = this->CreateObject(cmXCodeObject::PBXNativeTarget);
  target->AddAttribute("buildPhases", buildPhases);
  cmXCodeObject* buildRules = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  target->AddAttribute("buildRules", buildRules);
  cmXCodeObject* buildSettings =
    this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
  buildSettings->AddAttribute("INSTALL_PATH", 
                              this->CreateString("/usr/local/bin"));
  buildSettings->AddAttribute("OPTIMIZATION_CFLAGS", 
                              this->CreateString(""));
  buildSettings->AddAttribute("OTHER_CFLAGS", 
                              this->CreateString(""));
  buildSettings->AddAttribute("OTHER_LDFLAGS",
                              this->CreateString(""));
  buildSettings->AddAttribute("OTHER_REZFLAGS", 
                              this->CreateString(""));
  buildSettings->AddAttribute("PRODUCT_NAME", 
                              this->CreateString(cmtarget.GetName()));
  buildSettings->AddAttribute("SECTORDER_FLAGS",
                              this->CreateString(""));
  buildSettings->AddAttribute("WARNING_CFLAGS", 
                              this->CreateString("-Wmost -Wno-four-char-constants -Wno-unknown-pragmas"));
  target->AddAttribute("buildSettings", buildSettings);
  cmXCodeObject* dependencies = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  target->AddAttribute("dependencies", dependencies);
  target->AddAttribute("name", this->CreateString(cmtarget.GetName()));
  target->AddAttribute("productName",this->CreateString(cmtarget.GetName()));
  cmXCodeObject* fileRef = this->CreateObject(cmXCodeObject::PBXFileReference);
  fileRef->AddAttribute("explicitFileType", 
                        this->CreateString("\"compiled.mach-o.executable\""));
  fileRef->AddAttribute("includedInIndex", this->CreateString("0"));
  fileRef->AddAttribute("path", this->CreateString(cmtarget.GetName()));
  fileRef->AddAttribute("refType", this->CreateString("3"));
  fileRef->AddAttribute("sourceTree", this->CreateString("BUILT_PRODUCTS_DIR"));
  target->AddAttribute("productReference", this->CreateObjectReference(fileRef));
  target->AddAttribute("productType", 
                       this->CreateString("\"com.apple.product-type.tool\""));
  return target;
}

//----------------------------------------------------------------------------
cmXCodeObject* cmGlobalXCodeGenerator::CreateStaticLibrary(cmTarget& cmtarget,
                                                           cmXCodeObject* buildPhases)
{
  cmXCodeObject* target = this->CreateObject(cmXCodeObject::PBXNativeTarget);
  target->AddAttribute("buildPhases", buildPhases);
  cmXCodeObject* buildRules = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  target->AddAttribute("buildRules", buildRules);
  cmXCodeObject* buildSettings =
    this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
  buildSettings->AddAttribute("INSTALL_PATH", 
                              this->CreateString("/usr/local/bin"));
  buildSettings->AddAttribute("LIBRARY_STYLE", 
                              this->CreateString("STATIC"));
  buildSettings->AddAttribute("OPTIMIZATION_CFLAGS", 
                              this->CreateString(""));
  buildSettings->AddAttribute("OTHER_CFLAGS", 
                              this->CreateString(""));
  buildSettings->AddAttribute("OTHER_LDFLAGS",
                              this->CreateString(""));
  buildSettings->AddAttribute("OTHER_REZFLAGS", 
                              this->CreateString(""));
  buildSettings->AddAttribute("PRODUCT_NAME", 
                              this->CreateString(cmtarget.GetName()));
  buildSettings->AddAttribute("SECTORDER_FLAGS",
                              this->CreateString(""));
  buildSettings->AddAttribute("WARNING_CFLAGS", 
                              this->CreateString("-Wmost -Wno-four-char-constants -Wno-unknown-pragmas"));
  target->AddAttribute("buildSettings", buildSettings);
  cmXCodeObject* dependencies = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  target->AddAttribute("dependencies", dependencies);
  target->AddAttribute("name", this->CreateString(cmtarget.GetName()));
  target->AddAttribute("productName",this->CreateString(cmtarget.GetName()));
  cmXCodeObject* fileRef = this->CreateObject(cmXCodeObject::PBXFileReference);
  fileRef->AddAttribute("explicitFileType", 
                        this->CreateString("\"compiled.mach-o.archive.ar\""));
  fileRef->AddAttribute("includedInIndex", this->CreateString("0"));
  fileRef->AddAttribute("path", this->CreateString(cmtarget.GetName()));
  fileRef->AddAttribute("refType", this->CreateString("3"));
  fileRef->AddAttribute("sourceTree", this->CreateString("BUILT_PRODUCTS_DIR"));
  target->AddAttribute("productReference", this->CreateObjectReference(fileRef));
  target->AddAttribute("productType", 
                       this->CreateString("\"com.apple.product-type.library.static\""));
  return target;
}

//----------------------------------------------------------------------------
cmXCodeObject* cmGlobalXCodeGenerator::CreateSharedLibrary(cmTarget& cmtarget,
                                                           cmXCodeObject* buildPhases)
{
  cmXCodeObject* target = this->CreateObject(cmXCodeObject::PBXNativeTarget);
  target->AddAttribute("buildPhases", buildPhases);
  cmXCodeObject* buildRules = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  target->AddAttribute("buildRules", buildRules);
  cmXCodeObject* buildSettings =
    this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
  buildSettings->AddAttribute("DYLIB_COMPATIBILITY_VERSION", 
                              this->CreateString("1"));
  buildSettings->AddAttribute("DYLIB_CURRENT_VERSION", 
                              this->CreateString("1"));
  buildSettings->AddAttribute("INSTALL_PATH", 
                              this->CreateString("/usr/local/lib"));
  buildSettings->AddAttribute("LIBRARY_STYLE", 
                              this->CreateString("DYNAMIC"));
  buildSettings->AddAttribute("OPTIMIZATION_CFLAGS", 
                              this->CreateString(""));
  buildSettings->AddAttribute("OTHER_CFLAGS", 
                              this->CreateString(""));
  const char* libFlag = "-dynamiclib";
  if(cmtarget.GetType() == cmTarget::MODULE_LIBRARY)
    {
    libFlag = "-bundle";
    }
  buildSettings->AddAttribute("OTHER_LDFLAGS",
                              this->CreateString(libFlag));
  
  buildSettings->AddAttribute("OTHER_REZFLAGS", 
                              this->CreateString(""));
  buildSettings->AddAttribute("PRODUCT_NAME", 
                              this->CreateString(cmtarget.GetName()));
  buildSettings->AddAttribute("SECTORDER_FLAGS",
                              this->CreateString(""));
  buildSettings->AddAttribute("WARNING_CFLAGS", 
                              this->CreateString("-Wmost -Wno-four-char-constants -Wno-unknown-pragmas"));
  target->AddAttribute("buildSettings", buildSettings);
  cmXCodeObject* dependencies = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  target->AddAttribute("dependencies", dependencies);
  target->AddAttribute("name", this->CreateString(cmtarget.GetName()));
  target->AddAttribute("productName",this->CreateString(cmtarget.GetName()));
  cmXCodeObject* fileRef = this->CreateObject(cmXCodeObject::PBXFileReference);
  fileRef->AddAttribute("explicitFileType", 
                        this->CreateString("\"compiled.mach-o.dylib\""));
  fileRef->AddAttribute("includedInIndex", this->CreateString("0"));
  std::string path = cmtarget.GetName();
  path += ".dylib";
  fileRef->AddAttribute("path", this->CreateString(path.c_str()));
  fileRef->AddAttribute("refType", this->CreateString("3"));
  fileRef->AddAttribute("sourceTree", this->CreateString("BUILT_PRODUCTS_DIR"));
  target->AddAttribute("productReference", this->CreateObjectReference(fileRef));
  target->AddAttribute("productType", 
                       this->CreateString("\"com.apple.product-type.library.dynamic\""));
  return target;
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::CreateXCodeObjects(cmLocalGenerator* ,
                                                std::vector<cmLocalGenerator*>&
                                                generators
  )
{
  delete m_RootObject;
  this->ClearXCodeObjects(); 
  cmXCodeObject* group = this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
  group->AddAttribute("COPY_PHASE_STRIP", this->CreateString("NO"));
  cmXCodeObject* developBuildStyle = this->CreateObject(cmXCodeObject::PBXBuildStyle);
  developBuildStyle->AddAttribute("name", this->CreateString("Development"));
  developBuildStyle->AddAttribute("buildSettings", group);
  
  group = this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
  group->AddAttribute("COPY_PHASE_STRIP", this->CreateString("YES"));
  cmXCodeObject* deployBuildStyle = this->CreateObject(cmXCodeObject::PBXBuildStyle);
  deployBuildStyle->AddAttribute("name", this->CreateString("Deployment"));
  deployBuildStyle->AddAttribute("buildSettings", group);

  cmXCodeObject* listObjs = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  listObjs->AddObject(developBuildStyle);
  listObjs->AddObject(deployBuildStyle);
  
  cmXCodeObject* mainGroup = this->CreateObject(cmXCodeObject::PBXGroup);
  cmXCodeObject* mainGroupChildren = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  mainGroup->AddAttribute("children", mainGroupChildren);
  mainGroup->AddAttribute("refType", this->CreateString("4"));
  mainGroup->AddAttribute("sourceTree", this->CreateString("\"<group>\""));

  m_RootObject = this->CreateObject(cmXCodeObject::PBXProject);
  group = this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
  m_RootObject->AddAttribute("mainGroup", this->CreateObjectReference(mainGroup));
  m_RootObject->AddAttribute("buildSettings", group);
  m_RootObject->AddAttribute("buildSyles", listObjs);
  m_RootObject->AddAttribute("hasScannedForEncodings", this->CreateString("0"));
  std::vector<cmXCodeObject*> targets;
  for(std::vector<cmLocalGenerator*>::iterator i = generators.begin();
      i != generators.end(); ++i)
    {
    this->CreateXCodeTargets(*i, targets, mainGroupChildren);
    }
  cmXCodeObject* allTargets = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  for(std::vector<cmXCodeObject*>::iterator i = targets.begin();
      i != targets.end(); ++i)
    {
    allTargets->AddObject(*i);
    }
  m_RootObject->AddAttribute("targets", allTargets);
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::OutputXCodeProject(cmLocalGenerator* root,
                                                std::vector<cmLocalGenerator*>& 
                                                generators)
{
  if(generators.size() == 0)
    {
    return;
    }
  this->CreateXCodeObjects(root,
                           generators);
  std::string xcodeDir = root->GetMakefile()->GetStartOutputDirectory();
  xcodeDir += "/";
  xcodeDir += root->GetMakefile()->GetProjectName();
  xcodeDir += ".xcode";
  cmSystemTools::MakeDirectory(xcodeDir.c_str());
  xcodeDir += "/project.pbxproj";
  cmGeneratedFileStream fout(xcodeDir.c_str());
  fout.SetCopyIfDifferent(true);
  if(!fout)
    {
    return;
    }
  this->WriteXCodePBXProj(fout, root, generators);
  this->ClearXCodeObjects();
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::WriteXCodePBXProj(std::ostream& fout,
                                               cmLocalGenerator* ,
                                               std::vector<cmLocalGenerator*>& 
  )
{
  fout << "// !$*UTF8*$!\n";
  fout << "{\n";
  cmXCodeObject::Indent(1, fout);
  fout << "archiveVersion = 1;\n";
  cmXCodeObject::Indent(1, fout);
  fout << "classes = {\n";
  cmXCodeObject::Indent(1, fout);
  fout << "};\n";
  cmXCodeObject::Indent(1, fout);
  fout << "objectVersion = 39;\n";
  cmXCodeObject::PrintList(m_XCodeObjects, fout);
  cmXCodeObject::Indent(1, fout);
  fout << "rootObject = " << m_RootObject->GetId() << ";\n";
  fout << "}\n";
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::GetDocumentation(cmDocumentationEntry& entry) const
{
  entry.name = this->GetName();
  entry.brief = "NOT YET WORKING, Will generates XCode project files.";
  entry.full = "";
}
