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
#include "cmSystemTools.h"
#include "cmSourceFile.h"
#include "cmCacheManager.h"

cmLocalCodeWarriorGenerator::cmLocalCodeWarriorGenerator()
{
}

cmLocalCodeWarriorGenerator::~cmLocalCodeWarriorGenerator()
{
}


void cmLocalCodeWarriorGenerator::Generate(bool /* fromTheTop */)
{

}

void cmLocalCodeWarriorGenerator::WriteTargets(std::ostream& fout)
{
  cmTargets &tgts = m_Makefile->GetTargets();
  for(cmTargets::iterator l = tgts.begin(); 
      l != tgts.end(); l++)
    {
    this->WriteTarget(fout,l->first.c_str(),&(l->second));
    }
}

void cmLocalCodeWarriorGenerator::WriteTarget(std::ostream& fout,
                                              const char *tgtName,
                                              cmTarget const *l)
{
  fout << "<TARGET>\n";
  fout << "<NAME>" << tgtName << "</NAME>\n";
  
  this->WriteSettingList(fout,tgtName,l);
  this->WriteFileList(fout,tgtName,l);
  // this->WriteLinkOrder(fout,l);
  // this->WriteSubTargetList(fout,l);
  
  fout << "</TARGET>\n";
}

void cmLocalCodeWarriorGenerator::WriteSettingList(std::ostream& fout,
												   const char *tgtName,
                                                   cmTarget const *l)
{
  fout << "<SETTINGLIST>\n";

  // list the include paths
  fout << "<SETTING><NAME>UserSearchPaths</NAME>\n";
  std::vector<std::string>& includes = m_Makefile->GetIncludeDirectories();
  std::vector<std::string>::iterator i = includes.begin();
  for(;i != includes.end(); ++i)
    {
    fout << "<SETTING>\n";
    fout << "<SETTING><NAME>SearchPath</NAME>\n";
    fout << "<SETTING><NAME>Path</NAME><VALUE>" << i->c_str() << "</VALUE></SETTING>\n";
    fout << "<SETTING><NAME>PathFormat</NAME><VALUE>Generic</VALUE></SETTING>\n";
    fout << "<SETTING><NAME>PathRoot</NAME><VALUE>Absolute</VALUE></SETTING>\n";
    fout << "</SETTING>\n";
    fout << "<SETTING><NAME>Recursive</NAME><VALUE>false</VALUE></SETTING>\n";
    fout << "<SETTING><NAME>FrameworkPath</NAME><VALUE>false</VALUE></SETTING>\n";
    fout << "<SETTING><NAME>HostFlags</NAME><VALUE>All</VALUE></SETTING>\n";
    fout << "</SETTING>\n";
    }
  fout << "</SETTING>\n";

  fout << "<SETTING><NAME>Targetname</NAME><VALUE>" << tgtName 
       << "</VALUE></SETTING>\n";
  
  fout << "</SETTINGLIST>\n";
}

void cmLocalCodeWarriorGenerator::WriteFileList(std::ostream& fout,
												const char *tgtName,
                                                cmTarget const *l)
{
  fout << "<FILELIST>\n";

  // for each file
  std::vector<cmSourceFile*> const& classes = l->GetSourceFiles();
  for(std::vector<cmSourceFile*>::const_iterator i = classes.begin(); 
      i != classes.end(); i++)
    {
    // Add the file to the list of sources.
    std::string source = (*i)->GetFullPath();
    fout << "<FILE>\n";
    fout << "<PATHTYPE>PathAbsolute</PATHTYPE>\n";
    fout << "<PATHROOT>Absolute</PATHROOT>\n";
    //fout << "<ACCESSPATH>common</ACCESSPATH>\n";
    fout << "<PATH>" << source << "</PATH>\n";
    fout << "<PATHFORMAT>Generic</PATHFORMAT>\n";
    fout << "<FILEKIND>Text</FILEKIND>\n";
    fout << "<FILEFLAGS>Debug</FILEFLAGS>\n";
    fout << "</FILE>\n";
    }
  fout << "</FILELIST>\n";
}
