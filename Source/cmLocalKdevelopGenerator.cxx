/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  Copyright (c) 2004 Alexander Neundorf, neundorf@kde.org. All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmGlobalGenerator.h"
#include "cmLocalKdevelopGenerator.h"
#include "cmMakefile.h"
#include "cmSystemTools.h"
#include "cmSourceFile.h"
#include "cmMakeDepend.h"
#include "cmCacheManager.h"
#include "cmGeneratedFileStream.h"

#include <cmsys/RegularExpression.hxx>

#include <iostream>


cmLocalKdevelopGenerator::cmLocalKdevelopGenerator()
  :cmLocalUnixMakefileGenerator()
{
}

cmLocalKdevelopGenerator::~cmLocalKdevelopGenerator()
{
}

#include <stdio.h>
#include <string>
#include <vector>

void cmLocalKdevelopGenerator::Generate(bool fromTheTop)
{
  cmLocalUnixMakefileGenerator::Generate(fromTheTop);

  bool containsTargets=false;
  std::string executable;
  cmTargets& targets=m_Makefile->GetTargets();
  for (cmTargets::const_iterator ti = targets.begin(); ti != targets.end(); ti++)
    {
    switch (ti->second.GetType())
      {
      case cmTarget::EXECUTABLE:
        executable=ti->first;
      case cmTarget::STATIC_LIBRARY:
      case cmTarget::SHARED_LIBRARY:
      case cmTarget::MODULE_LIBRARY:
      case cmTarget::UTILITY:
        containsTargets=true;
        break;
      case cmTarget::INSTALL_FILES:
      case cmTarget::INSTALL_PROGRAMS:
      default:
        break;
      }
    }

  if (containsTargets)
    {
    std::string projectFileDir=m_Makefile->GetStartOutputDirectory();
    std::string filelistDir=m_Makefile->GetDefinition("PROJECT_SOURCE_DIR");
    //build the project name by taking the subdir
    std::string projectName=m_Makefile->GetProjectName();
    projectName+=m_Makefile->GetStartOutputDirectory();
    cmSystemTools::ReplaceString(projectName, filelistDir.c_str(), "");
    cmSystemTools::ReplaceString(projectName, "/", "_");

    std::string cmakeFilePattern("*/CMakeLists.txt;*.cmake;");

    if (!this->CreateFilelistFile(filelistDir, projectName, cmakeFilePattern))
      {
      return;
      }

    this->CreateProjectFile(filelistDir, projectName, executable, cmakeFilePattern);
    }
}

void cmLocalKdevelopGenerator::CreateProjectFile(const std::string& dir,
                                                 const std::string& projectname, 
                                                 const std::string& executable,
                                                 const std::string& cmakeFilePattern)
{
  std::string filename=m_Makefile->GetStartOutputDirectory();
  filename+="/";
  filename+=projectname+".kdevelop";

  if (cmSystemTools::FileExists(filename.c_str()))
    {
    this->MergeProjectFiles(dir, filename, executable, cmakeFilePattern);
    }
  else
    {
    this->CreateNewProjectFile(dir, filename, executable, cmakeFilePattern);
    }
   
}

void cmLocalKdevelopGenerator::MergeProjectFiles(const std::string& dir, 
                                                 const std::string& filename, 
                                                 const std::string& executable, 
                                                 const std::string& cmakeFilePattern)
{
  std::ifstream oldProjectFile(filename.c_str());
  if (!oldProjectFile)
    {
    this->CreateNewProjectFile(dir, filename, executable, cmakeFilePattern);
    return;
    }

  std::string tmp;
  std::vector<std::string> lines;
  while (cmSystemTools::GetLineFromStream(oldProjectFile, tmp))
    {
    lines.push_back(tmp);
    }
  oldProjectFile.close();

  cmGeneratedFileStream tempFile(filename.c_str());
  tempFile.SetAlwaysCopy(true);
  std::ostream&  fout = tempFile.GetStream();
  if(!fout)
    {
    cmSystemTools::Error("Error can not open for write: ", filename.c_str());
    return;
    }

  for (std::vector<std::string>::const_iterator it=lines.begin(); 
       it!=lines.end(); it++)
    {
    const char* line=(*it).c_str();

    if ((strstr(line, "<projectdirectory>")!=0)
        || (strstr(line, "<projectmanagement>")!=0)
        || (strstr(line, "<absoluteprojectpath>")!=0)
        || (strstr(line, "<buildtool>")!=0)
        || (strstr(line, "<builddir>")!=0))
      {
      continue;
      }

    fout<<*it<<"\n";

    if (strstr(line, "<general>"))
      {
      fout<<"  <projectmanagement>KDevCustomProject</projectmanagement>\n";
      fout<<"  <projectdirectory>"<<dir.c_str()<<"</projectdirectory>\n";   //this one is important
      fout<<"  <absoluteprojectpath>true</absoluteprojectpath>\n";          //and this one
      }

    if (strstr(line, "<build>"))
      {
      fout<<"      <buildtool>make</buildtool>\n";                                        //this one is important
      fout<<"      <builddir>"<<m_Makefile->GetStartOutputDirectory()<<"</builddir>\n";   //and this one

      }
    }
}

void cmLocalKdevelopGenerator::CreateNewProjectFile(const std::string& dir, const std::string& filename,
                                                    const std::string& executable, const std::string& cmakeFilePattern)
{

  cmGeneratedFileStream tempFile(filename.c_str());
  tempFile.SetAlwaysCopy(true);

  std::ostream&  fout = tempFile.GetStream();
  if(!fout)
    {
    cmSystemTools::Error("Error can not open for write: ", filename.c_str());
    return;
    }

  fout<<"<?xml version = '1.0'?>\n";
  fout<<"<kdevelop>\n";
  fout<<"  <general>\n";
  fout<<"  <author></author>\n";
  fout<<"  <email></email>\n";
  fout<<"  <version>$VERSION$</version>\n";
  fout<<"  <projectmanagement>KDevCustomProject</projectmanagement>\n";
  fout<<"  <primarylanguage>C++</primarylanguage>\n";
  fout<<"  <ignoreparts/>\n";
  fout<<"  <projectdirectory>"<<dir.c_str()<<"</projectdirectory>\n";   //this one is important
  fout<<"  <absoluteprojectpath>true</absoluteprojectpath>\n";          //and this one
  fout<<"  <secondaryLanguages>\n";
  fout<<"     <language>C</language>\n";
  fout<<"  </secondaryLanguages>\n";
  fout<<"  </general>\n";
  fout<<"  <kdevcustomproject>\n";
  fout<<"    <run>\n";
  fout<<"      <mainprogram>"<<m_Makefile->GetStartOutputDirectory()<<"/"<<executable.c_str()<<"</mainprogram>\n";
  fout<<"      <directoryradio>custom</directoryradio>\n";
  fout<<"      <customdirectory>/</customdirectory>\n";
  fout<<"      <programargs></programargs>\n";
  fout<<"      <terminal>false</terminal>\n";
  fout<<"      <autocompile>true</autocompile>\n";
  fout<<"      <envvars/>\n";
  fout<<"    </run>\n";
  fout<<"    <build>\n";
  fout<<"      <buildtool>make</buildtool>\n";                                        //this one is important
  fout<<"      <builddir>"<<m_Makefile->GetStartOutputDirectory()<<"</builddir>\n";   //and this one
  fout<<"    </build>\n";
  fout<<"    <make>\n";
  fout<<"      <abortonerror>false</abortonerror>\n";
  fout<<"      <numberofjobs>1</numberofjobs>\n";
  fout<<"      <dontact>false</dontact>\n";
  fout<<"      <makebin></makebin>\n";
  fout<<"      <selectedenvironment>default</selectedenvironment>\n";
  fout<<"      <environments>\n";
  fout<<"        <default/>\n";
  fout<<"      </environments>\n";
  fout<<"    </make>\n";
  fout<<"  </kdevcustomproject>\n";
  fout<<"  <kdevfileCreate>\n";
  fout<<"    <filetypes/>\n";
  fout<<"    <useglobaltypes>\n";
  fout<<"      <type ext=\"ui\" />\n";
  fout<<"      <type ext=\"cpp\" />\n";
  fout<<"      <type ext=\"h\" />\n";
  fout<<"    </useglobaltypes>\n";
  fout<<"  </kdevfilecreate>\n";
  fout<<"  <kdevdoctreeview>\n";
  fout<<"    <projectdoc>\n";
  fout<<"      <userdocDir>html/</userdocDir>\n";
  fout<<"      <apidocDir>html/</apidocDir>\n";
  fout<<"    </projectdoc>\n";
  fout<<"    <ignoreqt_xml/>\n";
  fout<<"    <ignoredoxygen/>\n";
  fout<<"    <ignorekdocs/>\n";
  fout<<"    <ignoretocs/>\n";
  fout<<"    <ignoredevhelp/>\n";
  fout<<"  </kdevdoctreeview>\n";
  fout<<"  <cppsupportpart>\n";
  fout<<"    <filetemplates>\n";
  fout<<"      <interfacesuffix>.h</interfacesuffix>\n";
  fout<<"      <implementationsuffix>.cpp</implementationsuffix>\n";
  fout<<"    </filetemplates>\n";
  fout<<"  </cppsupportpart>\n";
  fout<<"  <kdevcppsupport>\n";
  fout<<"    <codecompletion>\n";
  fout<<"      <includeGlobalFunctions>true</includeGlobalFunctions>\n";
  fout<<"      <includeTypes>true</includeTypes>\n";
  fout<<"      <includeEnums>true</includeEnums>\n";
  fout<<"      <includeTypedefs>false</includeTypedefs>\n";
  fout<<"      <automaticCodeCompletion>true</automaticCodeCompletion>\n";
  fout<<"      <automaticArgumentsHint>true</automaticArgumentsHint>\n";
  fout<<"      <automaticHeaderCompletion>true</automaticHeaderCompletion>\n";
  fout<<"      <codeCompletionDelay>250</codeCompletionDelay>\n";
  fout<<"      <argumentsHintDelay>400</argumentsHintDelay>\n";
  fout<<"      <headerCompletionDelay>250</headerCompletionDelay>\n";
  fout<<"    </codecompletion>\n";
  fout<<"    <references/>\n";
  fout<<"  </kdevcppsupport>\n";
  fout<<"  <kdevfileview>\n";
  fout<<"    <groups>\n";
  fout<<"      <group pattern=\""<<cmakeFilePattern.c_str()<<"\" name=\"CMake\" />\n";
  fout<<"      <group pattern=\"*.h;*.hxx\" name=\"Header\" />\n";
  fout<<"      <group pattern=\"*.cpp;*.c;*.C;*.cxx\" name=\"Sources\" />\n";
  fout<<"      <group pattern=\"*.ui\" name=\"Qt Designer files\" />\n";
  fout<<"      <hidenonprojectfiles>true</hidenonprojectfiles>\n";
  fout<<"    </groups>\n";
  fout<<"    <tree>\n";
  fout<<"      <hidepatterns>*.o,*.lo,CVS,*~,cmake*</hidepatterns>\n";
  fout<<"      <hidenonprojectfiles>true</hidenonprojectfiles>\n";
  fout<<"    </tree>\n";
  fout<<"  </kdevfileview>\n";
  fout<<"</kdevelop>\n";
}

bool cmLocalKdevelopGenerator::CreateFilelistFile(const std::string& _dir, 
                                                  const std::string& projectname,
                                                  std::string& cmakeFilePattern)
{
  std::string filelistDir=_dir+"/";
  std::string filename=filelistDir+projectname+".kdevelop.filelist";

  std::set<cmStdString> files;

  //get all cmake files
  std::string tmp;
  const std::vector<std::string>& listFiles=m_Makefile->GetListFiles();
  for (std::vector<std::string>::const_iterator it=listFiles.begin(); it!=listFiles.end(); it++)
    {
    tmp=*it;
    cmSystemTools::ReplaceString(tmp, filelistDir.c_str(), "");
    if (tmp[0]!='/')
      {
      files.insert(tmp);
      tmp=cmSystemTools::GetFilenameName(tmp);
      //add all files which dont match the default */CMakeLists.txt;*cmake; to the file pattern
      if ((tmp!="CMakeLists.txt")
          && (strstr(tmp.c_str(), ".cmake")==0))
        {
        cmakeFilePattern+="*/"+tmp+";";
        }
      }
    }

  //get all sources
  cmTargets& targets=m_Makefile->GetTargets();
  for (cmTargets::const_iterator ti = targets.begin(); ti != targets.end(); ti++)
    {
    const std::vector<std::string>& sources=ti->second.GetSourceLists();
    for (std::vector<std::string>::const_iterator it=sources.begin(); it!=sources.end(); it++)
      {
      tmp=*it;

      if (tmp[0]!='/') //no absolute path
        {
        tmp=std::string(m_Makefile->GetDefinition("CMAKE_CURRENT_SOURCE_DIR"))+"/"+tmp;
        }

      tmp=cmSystemTools::CollapseFullPath(tmp.c_str());
      cmSystemTools::ReplaceString(tmp, filelistDir.c_str(), "");
      if (tmp[0]=='/')
        {
        std::string errorMessage("In order to get working KDevelop project files, you have to call "
                                 "PROJECT() in a directory which is a parent directory of all source files. The source file ");
        errorMessage+=tmp+" is not located beneath your current project directory "+filelistDir+" .";
        cmSystemTools::Error(errorMessage.c_str());
        return false;
        }
      files.insert(tmp);
      }
    }


  //check if the output file already exists and read it
  //insert all files which exist into the set of files
  std::ifstream oldFilelist(filename.c_str());
  if (oldFilelist)
    {
    while (cmSystemTools::GetLineFromStream(oldFilelist, tmp))
      {
      if (tmp[0]=='/')
        {
        continue;
        }
      std::string completePath=filelistDir+tmp;
      if (cmSystemTools::FileExists(completePath.c_str()))
        {
        files.insert(tmp);
        }
      }
    oldFilelist.close();
    }
   

  cmGeneratedFileStream tempFile(filename.c_str());
  tempFile.SetAlwaysCopy(true);
  std::ostream&  fout = tempFile.GetStream();
  if(!fout)
    {
    cmSystemTools::Error("Error can not open for write: ", filename.c_str());
    return false;
    }
   
  for (std::set<cmStdString>::const_iterator it=files.begin(); it!=files.end(); it++)
    {
    fout<<*it<<"\n";
    }
  return true;
}


