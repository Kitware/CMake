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
#include "cmMakeDepend.h"
#include "cmStandardIncludes.h"
#include "cmSystemTools.h"

#include <cmsys/RegularExpression.hxx>

void cmDependInformation::AddDependencies(cmDependInformation* info)
{
  if(this != info)
    {
    m_DependencySet.insert(info);
    }
}

cmMakeDepend::cmMakeDepend()
{
  m_Verbose = false;
  m_IncludeFileRegularExpression.compile("^.*$");
  m_ComplainFileRegularExpression.compile("^$");
}


cmMakeDepend::~cmMakeDepend()
{ 
  for(DependInformationMap::iterator i = m_DependInformationMap.begin();
      i != m_DependInformationMap.end(); ++i)
    {
    delete i->second;
    }
}


// Set the makefile that depends will be made from.
// The pointer is kept so the cmSourceFile array can
// be updated with the depend information in the cmMakefile.

void cmMakeDepend::SetMakefile(const cmMakefile* makefile)
{
  m_Makefile = makefile;

  // Now extract the include file regular expression from the makefile.
  m_IncludeFileRegularExpression.compile(
    m_Makefile->m_IncludeFileRegularExpression.c_str());
  m_ComplainFileRegularExpression.compile(
    m_Makefile->m_ComplainFileRegularExpression.c_str());
  
  // Now extract any include paths from the makefile flags
  const std::vector<std::string>& includes =
    m_Makefile->GetIncludeDirectories();
  for(std::vector<std::string>::const_iterator j = includes.begin();
      j != includes.end(); ++j)
    {
    std::string path = *j;
    m_Makefile->ExpandVariablesInString(path);
    this->AddSearchPath(path.c_str());
    }
}


const cmDependInformation* cmMakeDepend::FindDependencies(const char* file)
{
  cmDependInformation* info = this->GetDependInformation(file,0);
  this->GenerateDependInformation(info);
  return info;
}

void cmMakeDepend::GenerateDependInformation(cmDependInformation* info)
{
  // If dependencies are already done, stop now.
  if(info->m_DependDone)
    {
    return;
    }
  else
    {
    // Make sure we don't visit the same file more than once.
    info->m_DependDone = true;
    }
  const char* path = info->m_FullPath.c_str();
  if(!path)
    {
    cmSystemTools::Error("Attempt to find dependencies for file without path!");
    return;
    }

  bool found = false;

  // If the file exists, use it to find dependency information.
  if(cmSystemTools::FileExists(path))
    {
    // Use the real file to find its dependencies.
    this->DependWalk(info);
    found = true;
    }

 
  // See if the cmSourceFile for it has any files specified as
  // dependency hints.
  if(info->m_cmSourceFile != 0)
    {

    // Get the cmSourceFile corresponding to this.
    const cmSourceFile& cFile = *(info->m_cmSourceFile);
    // See if there are any hints for finding dependencies for the missing
    // file.
    if(!cFile.GetDepends().empty())
      {
      // Dependency hints have been given.  Use them to begin the
      // recursion.
      for(std::vector<std::string>::const_iterator file =
            cFile.GetDepends().begin(); file != cFile.GetDepends().end(); 
          ++file)
        {
        this->AddDependency(info, file->c_str());
        }
      
      // Found dependency information.  We are done.
      found = true;
      }
    }

  if(!found)
    {
    // Try to find the file amongst the sources
    cmSourceFile *srcFile = 
      m_Makefile->GetSource(cmSystemTools::GetFilenameWithoutExtension(path).c_str());
    if (srcFile)
      {
      if (srcFile->GetFullPath() == path)
        {
        found=true;
        }
      else
        {
        //try to guess which include path to use
        for(std::vector<std::string>::iterator t = 
              m_IncludeDirectories.begin();
            t != m_IncludeDirectories.end(); ++t)
          {
          std::string incpath = *t;
          if (incpath.size() && incpath[incpath.size() - 1] != '/')
            {
            incpath = incpath + "/";
            }
          incpath = incpath + path;
          if (srcFile->GetFullPath() == incpath)
            {
            // set the path to the guessed path
            info->m_FullPath = incpath; 
            found=true;
            }
          }
        }
      }
    }
  
  if(!found)
    {
    // Couldn't find any dependency information.
    if(m_ComplainFileRegularExpression.find(info->m_IncludeName.c_str()))
      {
      cmSystemTools::Error("error cannot find dependencies for ", path);
      }
    else
      {
      // Destroy the name of the file so that it won't be output as a
      // dependency.
      info->m_FullPath = "";
      }
    }
}

// This function actually reads the file specified and scans it for
// #include directives
void cmMakeDepend::DependWalk(cmDependInformation* info)
{
  cmsys::RegularExpression includeLine("^[ \t]*#[ \t]*include[ \t]*[<\"]([^\">]+)[\">]");
  std::ifstream fin(info->m_FullPath.c_str());
  if(!fin)
    {
    cmSystemTools::Error("Cannot open ", info->m_FullPath.c_str());
    return;
    }

  // TODO: Write real read loop (see cmSystemTools::CopyFile).
  std::string line;
  while( cmSystemTools::GetLineFromStream(fin, line) )
    {
    if(includeLine.find(line.c_str()))
      {
      // extract the file being included
      std::string includeFile = includeLine.match(1);
      // see if the include matches the regular expression
      if(!m_IncludeFileRegularExpression.find(includeFile))
        {
        if(m_Verbose)
          {
          std::string message = "Skipping ";
          message += includeFile;
          message += " for file ";
          message += info->m_FullPath.c_str();
          cmSystemTools::Error(message.c_str(), 0);
          }
        continue;
        }
      
      // Add this file and all its dependencies.
      this->AddDependency(info, includeFile.c_str());
      }
    }
}


void cmMakeDepend::AddDependency(cmDependInformation* info, const char* file)
{
  cmDependInformation* dependInfo = 
    this->GetDependInformation(file,
                               cmSystemTools::GetFilenamePath(
                                 cmSystemTools::CollapseFullPath(
                                   info->m_FullPath.c_str())).c_str());
  this->GenerateDependInformation(dependInfo);
  info->AddDependencies(dependInfo);
}

cmDependInformation* cmMakeDepend::GetDependInformation(const char* file,
                                                        const char *extraPath)
{
  // Get the full path for the file so that lookup is unambiguous.
  std::string fullPath = this->FullPath(file, extraPath);
  
  // Try to find the file's instance of cmDependInformation.
  DependInformationMap::const_iterator result =
    m_DependInformationMap.find(fullPath);
  if(result != m_DependInformationMap.end())
    {
    // Found an instance, return it.
    return result->second;
    }
  else
    {
    // Didn't find an instance.  Create a new one and save it.
    cmDependInformation* info = new cmDependInformation;
    info->m_FullPath = fullPath;
    info->m_IncludeName = file;
    m_DependInformationMap[fullPath] = info;
    return info;
    }
}


void cmMakeDepend::GenerateMakefileDependencies()
{
  // Now create cmDependInformation objects for files in the directory
  const cmTargets &tgts = m_Makefile->GetTargets();
  for(cmTargets::const_iterator l = tgts.begin(); 
      l != tgts.end(); l++)
    {
    const std::vector<cmSourceFile*> &classes = l->second.GetSourceFiles();
    for(std::vector<cmSourceFile*>::const_iterator i = classes.begin(); 
        i != classes.end(); ++i)
      {
      if(!(*i)->GetPropertyAsBool("HEADER_FILE_ONLY"))
        {
        cmDependInformation* info =
          this->GetDependInformation((*i)->GetFullPath().c_str(),0);
        this->AddFileToSearchPath(info->m_FullPath.c_str());
        info->m_cmSourceFile = *i;
        this->GenerateDependInformation(info);
        }
      }
    }
}


// find the full path to fname by searching the m_IncludeDirectories array
std::string cmMakeDepend::FullPath(const char* fname, const char *extraPath)
{
  if(cmSystemTools::FileExists(fname))
    {
      return std::string(cmSystemTools::CollapseFullPath(fname));
    }
  
  for(std::vector<std::string>::iterator i = m_IncludeDirectories.begin();
      i != m_IncludeDirectories.end(); ++i)
    {
    std::string path = *i;
    if (path.size() && path[path.size() - 1] != '/')
      {
      path = path + "/";
      }
    path = path + fname;
    if(cmSystemTools::FileExists(path.c_str()))
      {
      return cmSystemTools::CollapseFullPath(path.c_str());
      }
    }

  if (extraPath)
    {
    std::string path = extraPath;
    if (path.size() && path[path.size() - 1] != '/')
      {
      path = path + "/";
      }
    path = path + fname;
    if(cmSystemTools::FileExists(path.c_str()))
      {
      return cmSystemTools::CollapseFullPath(path.c_str());
      }
    }
  
  // Couldn't find the file.
  return std::string(fname);
}

// Add a directory to the search path
void cmMakeDepend::AddSearchPath(const char* path)
{
  m_IncludeDirectories.push_back(path);
}

// Add a directory to the search path
void cmMakeDepend::AddFileToSearchPath(const char* file)
{
  std::string filepath = file;
  std::string::size_type pos = filepath.rfind('/');
  if(pos != std::string::npos)
    {
    std::string path = filepath.substr(0, pos);
    if(std::find(m_IncludeDirectories.begin(),
                 m_IncludeDirectories.end(), path)
       == m_IncludeDirectories.end())
      {
      m_IncludeDirectories.push_back(path);
      return;
      }
    }
}

const cmDependInformation*
cmMakeDepend::GetDependInformationForSourceFile(const cmSourceFile &sf) const
{
  for(DependInformationMap::const_iterator i = m_DependInformationMap.begin();
      i != m_DependInformationMap.end(); ++i)
    {
    const cmDependInformation* info = i->second;
    if(info->m_cmSourceFile == &sf)
      {
      return info;
      }
    }
  return 0;
}
