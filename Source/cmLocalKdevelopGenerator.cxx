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
#include "cmake.h"
#include <cmsys/RegularExpression.hxx>

cmLocalKdevelopGenerator::cmLocalKdevelopGenerator()
  :cmLocalUnixMakefileGenerator()
{
}

cmLocalKdevelopGenerator::~cmLocalKdevelopGenerator()
{
}


void cmLocalKdevelopGenerator::Generate(bool fromTheTop)
{
  cmLocalUnixMakefileGenerator::Generate(fromTheTop);
  if ( m_GlobalGenerator->GetCMakeInstance()->GetLocal() )
    {
    return;
    }
  // Does this local generator contain a PROJECT command
  // if so, then generate a kdevelop project for it
  if (strcmp(m_Makefile->GetDefinition("PROJECT_BINARY_DIR"), m_Makefile->GetStartOutputDirectory())==0)
    {
    std::string outputDir=m_Makefile->GetStartOutputDirectory();
    std::string projectDir=m_Makefile->GetHomeDirectory();
    std::string projectName=m_Makefile->GetProjectName();
    
    std::string cmakeFilePattern("CMakeLists.txt;*.cmake;");
    
    if (!this->CreateFilelistFile(outputDir, projectDir, projectName, cmakeFilePattern))
      {
      return;
      }

    //try to find the name of an executable so we have something to run from kdevelop
    // for now just pick the first executable found
    std::string executable;
    cmTargets& targets=m_Makefile->GetTargets();
    for (cmTargets::const_iterator ti = targets.begin(); ti != targets.end(); ti++)
      {
      if (ti->second.GetType()==cmTarget::EXECUTABLE)
        {
        executable=ti->first;
        break;
        }
      }
    this->CreateProjectFile(outputDir, projectDir, projectName, executable, cmakeFilePattern);
    }
}

/* create the project file, if it already exists, merge it with the existing one,
otherwise create a new one */
void cmLocalKdevelopGenerator::CreateProjectFile(const std::string& outputDir,
                                                 const std::string& projectDir,
                                                 const std::string& projectname, 
                                                 const std::string& executable,
                                                 const std::string& cmakeFilePattern)
{
  std::string filename=outputDir+"/";
  filename+=projectname+".kdevelop";

  if (cmSystemTools::FileExists(filename.c_str()))
    {
    this->MergeProjectFiles(outputDir, projectDir, filename, executable, cmakeFilePattern);
    }
  else
    {
    this->CreateNewProjectFile(outputDir, projectDir, filename, executable, cmakeFilePattern);
    }
   
}

void cmLocalKdevelopGenerator::MergeProjectFiles(const std::string& outputDir, 
                                                 const std::string& projectDir, 
                                                 const std::string& filename, 
                                                 const std::string& executable, 
                                                 const std::string& cmakeFilePattern)
{
  std::ifstream oldProjectFile(filename.c_str());
  if (!oldProjectFile)
    {
    this->CreateNewProjectFile(outputDir, projectDir, filename, executable, cmakeFilePattern);
    return;
    }

   /* Read the existing project file (line by line), copy all lines into the
    new project file, except the ones which can be reliably set from contents
    of the CMakeLists.txt */    
  std::string tmp;
  std::vector<std::string> lines;
  while (cmSystemTools::GetLineFromStream(oldProjectFile, tmp))
    {
    lines.push_back(tmp);
    }
  oldProjectFile.close();

  cmGeneratedFileStream fout(filename.c_str());
  if(!fout)
    {
    return;
    }

  for (std::vector<std::string>::const_iterator it=lines.begin(); 
       it!=lines.end(); it++)
    {
    const char* line=(*it).c_str();
    // skip these tags as they are always replaced
    if ((strstr(line, "<projectdirectory>")!=0)
        || (strstr(line, "<projectmanagement>")!=0)
        || (strstr(line, "<absoluteprojectpath>")!=0)
        || (strstr(line, "<filelistdirectory>")!=0)
        || (strstr(line, "<buildtool>")!=0)
        || (strstr(line, "<builddir>")!=0))
      {
      continue;
      }

    // output the line from the file if it is not one of the above tags
    fout<<*it<<"\n";
    // if this is the <general> tag output the stuff that goes in the general tag
    if (strstr(line, "<general>"))
      {
      fout<<"  <projectmanagement>KDevCustomProject</projectmanagement>\n";
      fout<<"  <projectdirectory>"<<projectDir.c_str()<<"</projectdirectory>\n";   //this one is important
      fout<<"  <absoluteprojectpath>true</absoluteprojectpath>\n";                 //and this one
      }
    // inside kdevcustomproject the <filelistdirectory> must be put
    if (strstr(line, "<kdevcustomproject>"))
      {
      fout<<"    <filelistdirectory>"<<outputDir.c_str()<<"</filelistdirectory>\n";
      }
    // buildtool and builddir go inside <build>
    if (strstr(line, "<build>"))
      {
      fout<<"      <buildtool>make</buildtool>\n";                  //this one is important
      fout<<"      <builddir>"<<outputDir.c_str()<<"</builddir>\n"; //and this one
      }
    }
}

void cmLocalKdevelopGenerator::CreateNewProjectFile(const std::string& outputDir,
                                                    const std::string& projectDir,
                                                    const std::string& filename,
                                                    const std::string& executable,
                                                    const std::string& cmakeFilePattern)
{

  cmGeneratedFileStream fout(filename.c_str());
  if(!fout)
    {
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
  fout<<"  <projectdirectory>"<<projectDir.c_str()<<"</projectdirectory>\n";   //this one is important
  fout<<"  <absoluteprojectpath>true</absoluteprojectpath>\n";          //and this one
  fout<<"  <secondaryLanguages>\n";
  fout<<"     <language>C</language>\n";
  fout<<"  </secondaryLanguages>\n";
  fout<<"  </general>\n";
  fout<<"  <kdevcustomproject>\n";
  fout<<"    <filelistdirectory>"<<outputDir.c_str()<<"</filelistdirectory>\n";
  fout<<"    <run>\n";
  fout<<"      <mainprogram>"<<outputDir.c_str()<<"/"<<executable.c_str()<<"</mainprogram>\n";
  fout<<"      <directoryradio>custom</directoryradio>\n";
  fout<<"      <customdirectory>/</customdirectory>\n";
  fout<<"      <programargs></programargs>\n";
  fout<<"      <terminal>false</terminal>\n";
  fout<<"      <autocompile>true</autocompile>\n";
  fout<<"      <envvars/>\n";
  fout<<"    </run>\n";
  fout<<"    <build>\n";
  fout<<"      <buildtool>make</buildtool>\n";                                        //this one is important
  fout<<"      <builddir>"<<outputDir.c_str()<<"</builddir>\n";   //and this one
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
  fout<<"  <kdevfilecreate>\n";
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

bool cmLocalKdevelopGenerator::CreateFilelistFile(const std::string& outputDir, const std::string& _projectDir,
                                                  const std::string& projectname,
                                                  std::string& cmakeFilePattern)
{
  std::string projectDir=_projectDir+"/";
  std::string filename=outputDir+"/"+projectname+".kdevelop.filelist";

  std::set<cmStdString> files;
  std::string tmp;

  // loop over all local generators in the entire project
  // This should be moved into the global generator 
  // FIXME
  std::vector<cmLocalGenerator *> lgs;
  m_GlobalGenerator->GetLocalGenerators(lgs);
  for (std::vector<cmLocalGenerator*>::const_iterator it=lgs.begin(); it!=lgs.end(); it++)
    {
    cmMakefile* makefile=(*it)->GetMakefile();
    // if the makefile GetStartOutputDirectory is not a substring of the outputDir
    // then skip it
    if (strstr(makefile->GetStartOutputDirectory(), outputDir.c_str())==0)
      {
      continue;
      }
    // This means the makefile is a sub-makefile of the current project
    //get all cmake files
    const std::vector<std::string>& listFiles=makefile->GetListFiles();
    for (std::vector<std::string>::const_iterator lt=listFiles.begin(); lt!=listFiles.end(); lt++)
      {
      tmp=*lt;
      cmSystemTools::ReplaceString(tmp, projectDir.c_str(), "");
      // make sure the file is part of this source tree
      if (tmp[0]!='/')
        {
        files.insert(tmp);
        tmp=cmSystemTools::GetFilenameName(tmp);
        //add all files which dont match the default */CMakeLists.txt;*cmake; to the file pattern
        if ((tmp!="CMakeLists.txt")
            && (strstr(tmp.c_str(), ".cmake")==0))
          {
          cmakeFilePattern+=tmp+";";
          }
        }
      }
  
    //get all sources
    cmTargets& targets=makefile->GetTargets();
    for (cmTargets::const_iterator ti = targets.begin(); ti != targets.end(); ti++)
      {
      const std::vector<cmSourceFile*>& sources=ti->second.GetSourceFiles();
      for (std::vector<cmSourceFile*>::const_iterator si=sources.begin();
           si!=sources.end(); si++)
        {
        tmp=(*si)->GetFullPath();
        cmSystemTools::ReplaceString(tmp, projectDir.c_str(), "");
        if (tmp[0]!='/')
        {
            files.insert(tmp);
        }
        }
      for (std::vector<std::string>::const_iterator lt=listFiles.begin();
           lt!=listFiles.end(); lt++)
        {
        tmp=*lt;
        cmSystemTools::ReplaceString(tmp, projectDir.c_str(), "");
        if (tmp[0]!='/')
          {
          files.insert(tmp.c_str());
          }
        }
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
      std::string completePath=projectDir+tmp;
      if (cmSystemTools::FileExists(completePath.c_str()))
        {
        files.insert(tmp);
        }
      }
    oldFilelist.close();
    }

  //now write the new filename
  cmGeneratedFileStream fout(filename.c_str());
  if(!fout)
    {
    return false;
    }
   
  for (std::set<cmStdString>::const_iterator it=files.begin(); it!=files.end(); it++)
    {
    // get the full path to the file
    tmp=cmSystemTools::CollapseFullPath(it->c_str(), projectDir.c_str());
    // make it relative to the project dir
    cmSystemTools::ReplaceString(tmp, projectDir.c_str(), "");
    // only put relative paths
    if (tmp.size() && tmp[0] != '/')
      {
      fout << tmp.c_str() <<"\n";
      }
    }
  return true;
}
