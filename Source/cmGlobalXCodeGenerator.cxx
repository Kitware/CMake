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
cmXCodeObject* cmGlobalXCodeGenerator::CreateObject(cmXCodeObject::PBXType ptype,
                                                    cmXCodeObject::Type type)
{
  cmXCodeObject* obj = new cmXCodeObject(ptype, type);
  m_XCodeObjects.push_back(obj);
  return obj;
}

cmXCodeObject* cmGlobalXCodeGenerator::CreateString(const char* s)
{
  cmXCodeObject* obj = new cmXCodeObject(cmXCodeObject::None, cmXCodeObject::STRING);
  m_XCodeObjects.push_back(obj);
  obj->SetString(s);
  return obj;
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::CreateXCodeObjects(cmLocalGenerator* ,
                                                std::vector<cmLocalGenerator*>& 
                                                )
{
  delete m_RootObject;
  this->ClearXCodeObjects(); 
  cmXCodeObject* group = this->CreateObject(cmXCodeObject::None, cmXCodeObject::ATTRIBUTE_GROUP);
  group->AddAttribute("COPY_PHASE_STRIP", this->CreateString("NO"));
  cmXCodeObject* developBuildStyle = this->CreateObject(cmXCodeObject::PBXBuildStyle,
                                                        cmXCodeObject::OBJECT);
  developBuildStyle->AddAttribute("name", this->CreateString("Development"));
  developBuildStyle->AddAttribute("buildSettings", group);
  
  group = this->CreateObject(cmXCodeObject::None, cmXCodeObject::ATTRIBUTE_GROUP);
  group->AddAttribute("COPY_PHASE_STRIP", this->CreateString("YES"));
  cmXCodeObject* deployBuildStyle = this->CreateObject(cmXCodeObject::PBXBuildStyle,
                                                       cmXCodeObject::OBJECT);
  deployBuildStyle->AddAttribute("name", this->CreateString("Deployment"));
  deployBuildStyle->AddAttribute("buildSettings", group);

  cmXCodeObject* listObjs = this->CreateObject(cmXCodeObject::None,
                                               cmXCodeObject::OBJECT_LIST);
  listObjs->AddObject(developBuildStyle);
  listObjs->AddObject(deployBuildStyle);
  
  
  m_RootObject = this->CreateObject(cmXCodeObject::PBXProject, cmXCodeObject::OBJECT);
  group = this->CreateObject(cmXCodeObject::None, cmXCodeObject::ATTRIBUTE_GROUP);
  m_RootObject->AddAttribute("buildSettings", group);
  m_RootObject->AddAttribute("buildSyles", listObjs);
  m_RootObject->AddAttribute("hasScannedForEncodings", this->CreateString("0"));
  
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
