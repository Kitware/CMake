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
void cmGlobalXCodeGenerator::EnableLanguage(std::vector<std::string>const& ,
                                                    cmMakefile *)
{
  //this->cmGlobalGenerator::EnableLanguage(lang, mf);
}

//----------------------------------------------------------------------------
int cmGlobalXCodeGenerator::TryCompile(const char *, 
                                               const char *, 
                                               const char *,
                                               const char *,
                                               std::string *,
                                               cmMakefile* )
{
  // FIXME
  return 1;
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
cmXCodeObject* cmGlobalXCodeGenerator::CreateObject(cmXCodeObject::PBXType ptype)
{
  cmXCodeObject* obj = new cmXCodeObject(ptype, cmXCodeObject::OBJECT);
  m_XCodeObjects.push_back(obj);
  return obj;
}

//----------------------------------------------------------------------------
cmXCodeObject* cmGlobalXCodeGenerator::CreateObject(cmXCodeObject::Type type)
{
  cmXCodeObject* obj = new cmXCodeObject(cmXCodeObject::None, type);
  m_XCodeObjects.push_back(obj);
  return obj;
}

cmXCodeObject* cmGlobalXCodeGenerator::CreateString(const char* s)
{
  cmXCodeObject* obj = this->CreateObject(cmXCodeObject::STRING);
  obj->SetString(s);
  return obj;
}

cmXCodeObject* 
cmGlobalXCodeGenerator::CreateXCodeSourceFile(cmLocalGenerator* lg, 
                                              cmSourceFile* sf)
{
  cmXCodeObject* fileRef = this->CreateObject(cmXCodeObject::PBXFileReference);
  cmXCodeObject* fileRefPtr = this->CreateObject(cmXCodeObject::OBJECT_REF);
  fileRefPtr->SetObject(fileRef);
  cmXCodeObject* buildFile = this->CreateObject(cmXCodeObject::PBXBuildFile);
  buildFile->AddAttribute("fileRef", fileRefPtr);
  cmXCodeObject* settings = this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
  buildFile->AddAttribute("settings", settings);
  fileRef->AddAttribute("fileEncoding", this->CreateString("4"));
  fileRef->AddAttribute("lastKnownFileType", this->CreateString("sourcecode.cpp.cpp"));
  fileRef->AddAttribute("path", this->CreateString(
    lg->ConvertToRelativeOutputPath(sf->GetFullPath().c_str()).c_str()));
  fileRef->AddAttribute("refType", this->CreateString("4"));
  fileRef->AddAttribute("sourceTree", this->CreateString("\"<group>\""));
  return buildFile;
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::CreateXCodeTargets(cmLocalGenerator* gen,
                                                std::vector<cmXCodeObject*>& targets)
{
  cmTargets &tgts = gen->GetMakefile()->GetTargets();
  for(cmTargets::iterator l = tgts.begin(); l != tgts.end(); l++)
    { 
    // create source build phase
    cmXCodeObject* sourceBuildPhase = this->CreateObject(cmXCodeObject::PBXSourcesBuildPhase);
    sourceBuildPhase->AddAttribute("buildActionMask", this->CreateString("2147483647"));
    cmXCodeObject* buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
    sourceBuildPhase->AddAttribute("files", buildFiles);
    sourceBuildPhase->AddAttribute("runOnlyForDeploymentPostprocessing", this->CreateString("0"));
    std::vector<cmSourceFile*> &classes = l->second.GetSourceFiles();
    // add all the sources
    for(std::vector<cmSourceFile*>::iterator i = classes.begin(); 
        i != classes.end(); ++i)
      {
      buildFiles->AddObject(this->CreateXCodeSourceFile(gen, *i));
      }
    // create header build phase
    cmXCodeObject* headerBuildPhase = this->CreateObject(cmXCodeObject::PBXHeadersBuildPhase);
    headerBuildPhase->AddAttribute("buildActionMask", this->CreateString("2147483647"));
    buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
    headerBuildPhase->AddAttribute("files", buildFiles);
    headerBuildPhase->AddAttribute("runOnlyForDeploymentPostprocessing", this->CreateString("0"));
    
    // create framework build phase
    cmXCodeObject* frameworkBuildPhase = this->CreateObject(cmXCodeObject::PBXFrameworksBuildPhase);
    frameworkBuildPhase->AddAttribute("buildActionMask", this->CreateString("2147483647"));
    buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
    frameworkBuildPhase->AddAttribute("files", buildFiles);
    frameworkBuildPhase->AddAttribute("runOnlyForDeploymentPostprocessing", this->CreateString("0"));

    cmXCodeObject* buildPhases = this->CreateObject(cmXCodeObject::OBJECT_LIST);
    buildPhases->AddObject(sourceBuildPhase);
    buildPhases->AddObject(headerBuildPhase);
    buildPhases->AddObject(frameworkBuildPhase);
    
    if((l->second.GetType() == cmTarget::STATIC_LIBRARY) ||
       (l->second.GetType() == cmTarget::SHARED_LIBRARY) ||
       (l->second.GetType() == cmTarget::MODULE_LIBRARY))
      {
      }
    else if ( l->second.GetType() == cmTarget::EXECUTABLE )
      {
      cmXCodeObject* target = this->CreateObject(cmXCodeObject::PBXNativeTarget);
      targets.push_back(target);
      target->AddAttribute("buildPhases", buildPhases);
      cmXCodeObject* buildRules = this->CreateObject(cmXCodeObject::OBJECT_LIST);
      target->AddAttribute("buildRules", buildRules);
      cmXCodeObject* buildSettings = this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
      buildSettings->AddAttribute("INSTALL_PATH", this->CreateString("/usr/local/bin"));
      buildSettings->AddAttribute("OPTIMIZATION_CFLAGS", this->CreateString(""));
      buildSettings->AddAttribute("OTHER_CFLAGS", this->CreateString(""));
      buildSettings->AddAttribute("OTHER_LDFLAGS", this->CreateString(""));
      buildSettings->AddAttribute("OTHER_REZFLAGS", this->CreateString(""));
      buildSettings->AddAttribute("PRODUCT_NAME", this->CreateString(l->first.c_str()));
      buildSettings->AddAttribute("SECTORDER_FLAGS", this->CreateString(""));
      buildSettings->AddAttribute("WARNING_CFLAGS", 
                                  this->CreateString("-Wmost -Wno-four-char-constants -Wno-unknown-pragmas"));
      target->AddAttribute("buildSettings", buildSettings);
      cmXCodeObject* dependencies = this->CreateObject(cmXCodeObject::OBJECT_LIST);
      target->AddAttribute("dependencies", dependencies);
      target->AddAttribute("name", this->CreateString(l->first.c_str()));
      target->AddAttribute("productName",this->CreateString(l->first.c_str()));
      cmXCodeObject* fileRef = this->CreateObject(cmXCodeObject::PBXFileReference);
      fileRef->AddAttribute("explicitFileType", this->CreateString("compiled.mach-o.executable"));
      fileRef->AddAttribute("includedInIndex", this->CreateString("0"));
      fileRef->AddAttribute("path", this->CreateString(l->first.c_str()));
      fileRef->AddAttribute("refType", this->CreateString("3"));
      fileRef->AddAttribute("sourceTree", this->CreateString("BUILT_PRODUCTS_DIR"));
      cmXCodeObject* fileRefPtr = this->CreateObject(cmXCodeObject::OBJECT_REF);
      fileRefPtr->AddObject(fileRef);
      target->AddAttribute("productReference", fileRefPtr);
      target->AddAttribute("productReference", this->CreateString("com.apple.product-type.tool"));
      }
    else if (l->second.GetType() == cmTarget::UTILITY)
      {
      }
    }
  
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::CreateXCodeObjects(cmLocalGenerator* root,
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
  
  
  m_RootObject = this->CreateObject(cmXCodeObject::PBXProject);
  group = this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
  m_RootObject->AddAttribute("buildSettings", group);
  m_RootObject->AddAttribute("buildSyles", listObjs);
  m_RootObject->AddAttribute("hasScannedForEncodings", this->CreateString("0"));
  std::vector<cmXCodeObject*> targets;
  for(std::vector<cmLocalGenerator*>::iterator i = generators.begin();
      i != generators.end(); ++i)
    {
    this->CreateXCodeTargets(*i, targets);
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
