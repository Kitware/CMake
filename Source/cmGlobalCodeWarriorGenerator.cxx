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
#include "cmGlobalCodeWarriorGenerator.h"
#include "cmLocalCodeWarriorGenerator.h"
#include "cmMakefile.h"
#include "cmake.h"
#include "cmTarget.h"
#include "windows.h"

void cmGlobalCodeWarriorGenerator::EnableLanguage(const char*, 
                                                  cmMakefile *mf)
{
  // now load the settings
  if(!mf->GetDefinition("CMAKE_ROOT"))
    {
    cmSystemTools::Error(
      "CMAKE_ROOT has not been defined, bad GUI or driver program");
    return;
    }
  if(!this->GetLanguageEnabled("CXX"))
    {
    std::string fpath = 
      mf->GetDefinition("CMAKE_ROOT");
    fpath += "/Templates/CMakeDotNetSystemConfig.cmake";
    mf->ReadListFile(NULL,fpath.c_str());
    this->SetLanguageEnabled("CXX");
    }
}

int cmGlobalCodeWarriorGenerator::TryCompile(const char *, 
                                             const char *bindir, 
                                             const char *projectName,
                                             const char *targetName,
                                             std::string *output)
{
	return 1;
}

///! Create a local generator appropriate to this Global Generator
cmLocalGenerator *cmGlobalCodeWarriorGenerator::CreateLocalGenerator()
{
  cmLocalGenerator *lg = new cmLocalCodeWarriorGenerator;
  lg->SetGlobalGenerator(this);
  return lg;
}

void cmGlobalCodeWarriorGenerator::Generate()
{
  // first do the superclass method
  this->cmGlobalGenerator::Generate();
  
  // Now write out the Project File
  this->OutputProject();
}

void cmGlobalCodeWarriorGenerator::OutputProject()
{
  // if this is an out of source build, create the output directory
  if(strcmp(m_CMakeInstance->GetStartOutputDirectory(),
            m_CMakeInstance->GetHomeDirectory()) != 0)
    {
    if(!cmSystemTools::MakeDirectory(m_CMakeInstance->GetStartOutputDirectory()))
      {
      cmSystemTools::Error("Error creating output directory for Project file",
                           m_CMakeInstance->GetStartOutputDirectory());
      }
    }
  // create the project file name
  std::string fname;
  fname = m_CMakeInstance->GetStartOutputDirectory();
  fname += "/";
  if(strlen(m_LocalGenerators[0]->GetMakefile()->GetProjectName()) == 0)
    {
    m_LocalGenerators[0]->GetMakefile()->SetProjectName("Project");
    }
  fname += m_LocalGenerators[0]->GetMakefile()->GetProjectName();
  fname += ".xml";
  std::ofstream fout(fname.c_str());
  if(!fout)
    {
    cmSystemTools::Error("Error can not open project file for write: "
                         ,fname.c_str());
    return;
    }
  this->WriteProject(fout);
}

void cmGlobalCodeWarriorGenerator::WriteProject(std::ostream& fout)
{
  // Write out the header for a SLN file
  this->WriteProjectHeader(fout);

  // start the project
  fout << "<PROJECT>\n";
  
  // write the target list
  this->WriteTargetList(fout);
  
  // write the target order
  this->WriteTargetOrder(fout);
  
  // write the group list
  this->WriteGroupList(fout);
  
  // close the project
  fout << "</PROJECT>\n";
}

void cmGlobalCodeWarriorGenerator::WriteProjectHeader(std::ostream& fout)
{
  fout << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?>\n";
  fout << "<?codewarrior exportversion=\"1.0.1\" ideversion=\"5.0\" ?>\n";

  fout << "<!DOCTYPE PROJECT [\n";
  fout << "<!ELEMENT PROJECT (TARGETLIST, TARGETORDER, GROUPLIST, DESIGNLIST?)>\n";
  fout << "<!ELEMENT TARGETLIST (TARGET+)>\n";
  fout << "<!ELEMENT TARGET (NAME, SETTINGLIST, FILELIST?, LINKORDER?, SEGMENTLIST?, OVERLAYGROUPLIST?, SUBTARGETLIST?, SUBPROJECTLIST?, FRAMEWORKLIST)>\n";
  fout << "<!ELEMENT NAME (#PCDATA)>\n";
  fout << "<!ELEMENT USERSOURCETREETYPE (#PCDATA)>\n";
  fout << "<!ELEMENT PATH (#PCDATA)>\n";
  fout << "<!ELEMENT FILELIST (FILE*)>\n";
  fout << "<!ELEMENT FILE (PATHTYPE, PATHROOT?, ACCESSPATH?, PATH, PATHFORMAT?, ROOTFILEREF?, FILEKIND?, FILEFLAGS?)>\n";
  fout << "<!ELEMENT PATHTYPE (#PCDATA)>\n";
  fout << "<!ELEMENT PATHROOT (#PCDATA)>\n";
  fout << "<!ELEMENT ACCESSPATH (#PCDATA)>\n";
  fout << "<!ELEMENT PATHFORMAT (#PCDATA)>\n";
  fout << "<!ELEMENT ROOTFILEREF (PATHTYPE, PATHROOT?, ACCESSPATH?, PATH, PATHFORMAT?)>\n";
  fout << "<!ELEMENT FILEKIND (#PCDATA)>\n";
  fout << "<!ELEMENT FILEFLAGS (#PCDATA)>\n";
  fout << "<!ELEMENT FILEREF (TARGETNAME?, PATHTYPE, PATHROOT?, ACCESSPATH?, PATH, PATHFORMAT?)>\n";
  fout << "<!ELEMENT TARGETNAME (#PCDATA)>\n";
  fout << "<!ELEMENT SETTINGLIST ((SETTING|PANELDATA)+)>\n";
  fout << "<!ELEMENT SETTING (NAME?, (VALUE|(SETTING+)))>\n";
  fout << "<!ELEMENT PANELDATA (NAME, VALUE)>\n";
  fout << "<!ELEMENT VALUE (#PCDATA)>\n";
  fout << "<!ELEMENT LINKORDER (FILEREF*)>\n";
  fout << "<!ELEMENT SEGMENTLIST (SEGMENT+)>\n";
  fout << "<!ELEMENT SEGMENT (NAME, ATTRIBUTES?, FILEREF*)>\n";
  fout << "<!ELEMENT ATTRIBUTES (#PCDATA)>\n";
  fout << "<!ELEMENT OVERLAYGROUPLIST (OVERLAYGROUP+)>\n";
  fout << "<!ELEMENT OVERLAYGROUP (NAME, BASEADDRESS, OVERLAY*)>\n";
  fout << "<!ELEMENT BASEADDRESS (#PCDATA)>\n";
  fout << "<!ELEMENT OVERLAY (NAME, FILEREF*)>\n";
  fout << "<!ELEMENT SUBTARGETLIST (SUBTARGET+)>\n";
  fout << "<!ELEMENT SUBTARGET (TARGETNAME, ATTRIBUTES?, FILEREF?)>\n";
  fout << "<!ELEMENT SUBPROJECTLIST (SUBPROJECT+)>\n";
  fout << "<!ELEMENT SUBPROJECT (FILEREF, SUBPROJECTTARGETLIST)>\n";
  fout << "<!ELEMENT SUBPROJECTTARGETLIST (SUBPROJECTTARGET*)>\n";
  fout << "<!ELEMENT SUBPROJECTTARGET (TARGETNAME, ATTRIBUTES?, FILEREF?)>\n";
  fout << "<!ELEMENT FRAMEWORKLIST (FRAMEWORK+)>\n";
  fout << "<!ELEMENT FRAMEWORK (FILEREF, LIBRARYFILE?, VERSION?)>\n";
  fout << "<!ELEMENT LIBRARYFILE (FILEREF)>\n";
  fout << "<!ELEMENT VERSION (#PCDATA)>\n";
  fout << "<!ELEMENT TARGETORDER (ORDEREDTARGET|ORDEREDDESIGN)*>\n";
  fout << "<!ELEMENT ORDEREDTARGET (NAME)>\n";
  fout << "<!ELEMENT ORDEREDDESIGN (NAME, ORDEREDTARGET+)>\n";
  fout << "<!ELEMENT GROUPLIST (GROUP|FILEREF)*>\n";
  fout << "<!ELEMENT GROUP (NAME, (GROUP|FILEREF)*)>\n";
  fout << "<!ELEMENT DESIGNLIST (DESIGN+)>\n";
  fout << "<!ELEMENT DESIGN (NAME, DESIGNDATA)>\n";
  fout << "<!ELEMENT DESIGNDATA (#PCDATA)>\n";
  fout << "]>\n\n";
}

void cmGlobalCodeWarriorGenerator::WriteTargetList(std::ostream& fout)
{
  fout << "<TARGETLIST>\n";
  
  unsigned int i;
  // for each local generator
  for (i = 0; i < m_LocalGenerators.size(); ++i)
    {
    static_cast<cmLocalCodeWarriorGenerator *>(m_LocalGenerators[i])->WriteTargets(fout);
    }
  fout << "</TARGETLIST>\n";
}

cmTarget *cmGlobalCodeWarriorGenerator::GetTargetFromName(const char *tgtName)
{
  // for each local generator, and each target
  unsigned int i;
  for(i = 0; i < m_LocalGenerators.size(); ++i)
    {
    cmMakefile* mf = m_LocalGenerators[i]->GetMakefile();
    cmTargets &tgts = mf->GetTargets();
    for(cmTargets::iterator l = tgts.begin(); l != tgts.end(); ++l)
      {
      if (l->first == tgtName)
        {
        return &(l->second);
        }
      }
    }
  return 0;
}

void cmGlobalCodeWarriorGenerator::ComputeTargetOrder(
  std::vector<std::string> &tgtOrder, const char *tgtName,
  cmTarget const *target)
{
  // if the target is already listed then we are done
  if (std::find(tgtOrder.begin(),tgtOrder.end(),tgtName) != tgtOrder.end())
    {
    return;
    }
  
  // otherwise find all this target depends on and add them first
  cmTarget::LinkLibraries::const_iterator j, jend;
  j = target->GetLinkLibraries().begin();
  jend = target->GetLinkLibraries().end();
  for(;j!= jend; ++j)
    {
    if(j->first != tgtName)
      {
      // is the library part of this Project ? 
      std::string libPath = j->first + "_CMAKE_PATH";
      const char* cacheValue
        = m_CMakeInstance->GetCacheDefinition(libPath.c_str());
      if(cacheValue)
        {
        // so add it to the tgtOrder vector if it isn't already there
        // to do this we need the actual target
        cmTarget *tgt = this->GetTargetFromName(j->first.c_str());
        this->ComputeTargetOrder(tgtOrder,j->first.c_str(), tgt);
        }
      }
    }
  // finally add the target
  tgtOrder.push_back(tgtName);
}

void cmGlobalCodeWarriorGenerator::WriteTargetOrder(std::ostream& fout)
{
  fout << "<TARGETORDER>\n";
  
  std::vector<std::string> tgtOrder;
  unsigned int i;
  // for each local generator, and each target
  for(i = 0; i < m_LocalGenerators.size(); ++i)
    {
    cmMakefile* mf = m_LocalGenerators[i]->GetMakefile();
    cmTargets &tgts = mf->GetTargets();
    for(cmTargets::iterator l = tgts.begin(); l != tgts.end(); ++l)
      {
      this->ComputeTargetOrder(tgtOrder, l->first.c_str(), &(l->second));
      }
    }

  // now write out the target order
  for(i = 0; i < tgtOrder.size(); ++i)
    {
    fout << "<ORDEREDTARGET><NAME>" << tgtOrder[i] 
         << "</NAME></ORDEREDTARGET>\n";
    }
  fout << "</TARGETORDER>\n";
}

void cmGlobalCodeWarriorGenerator::WriteGroupList(std::ostream& fout)
{
  fout << "<GROUPLIST>\n";
  
  fout << "</GROUPLIST>\n";
}

void cmGlobalCodeWarriorGenerator::LocalGenerate()
{
  this->cmGlobalGenerator::LocalGenerate();
}
