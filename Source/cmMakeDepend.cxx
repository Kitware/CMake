/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) 2001 Insight Consortium
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * The name of the Insight Consortium, nor the names of any consortium members,
   nor of any contributors, may be used to endorse or promote products derived
   from this software without specific prior written permission.

  * Modified source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "cmMakeDepend.h"
#include "cmStandardIncludes.h"
#include "cmSystemTools.h"


void cmDependInformation::AddDependencies(cmDependInformation* info)
{
  if(this != info)
    {
    m_DependencySet.insert(info);
    for (cmDependInformation::DependencySet::const_iterator
           d = info->m_DependencySet.begin(); 
         d != info->m_DependencySet.end(); ++d)
      {
      m_DependencySet.insert(*d);
      }
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
    this->AddSearchPath(j->c_str());
    }
}


const cmDependInformation* cmMakeDepend::FindDependencies(const char* file)
{
  cmDependInformation* info = this->GetDependInformation(file);
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

  // If the file exists, use it to find dependency information.
  if(cmSystemTools::FileExists(path))
    {
    // Use the real file to find its dependencies.
    this->DependWalk(info);
    return;
    }
  
  // The file doesn't exist.  See if the cmSourceFile for it has any files
  // specified as dependency hints.
  if(info->m_cmSourceFile != 0)
    {
    // Get the cmSourceFile corresponding to this.
    const cmSourceFile& cFile = *(info->m_cmSourceFile);
    // See if there are any hints for finding dependencies for the missing
    // file.
    if(!cFile.GetDepends().empty())
      {
      // Initial dependencies have been given.  Use them to begin the
      // recursion.
      for(std::vector<std::string>::const_iterator file =
            cFile.GetDepends().begin(); file != cFile.GetDepends().end(); 
          ++file)
        {
        this->AddDependency(info, file->c_str());
        }
      
      // Found dependency information.  We are done.
      return;
      }
    }
  
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

// This function actually reads the file specified and scans it for
// #include directives
void cmMakeDepend::DependWalk(cmDependInformation* info)
{
  cmRegularExpression includeLine("^[ \t]*#[ \t]*include[ \t]*[<\"]([^\">]+)[\">]");
  std::ifstream fin(info->m_FullPath.c_str());
  if(!fin)
    {
    cmSystemTools::Error("Cannot open ", info->m_FullPath.c_str());
    return;
    }

  // TODO: Write real read loop (see cmSystemTools::CopyFile).
  char line[255];
  for(fin.getline(line, 255); fin; fin.getline(line, 255))
    {
    if(includeLine.find(line))
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
  cmDependInformation* dependInfo = this->GetDependInformation(file);
  this->GenerateDependInformation(dependInfo);
  info->AddDependencies(dependInfo);
}

cmDependInformation* cmMakeDepend::GetDependInformation(const char* file)
{
  // Get the full path for the file so that lookup is unambiguous.
  std::string fullPath = this->FullPath(file);
  
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
    const std::vector<cmSourceFile> &classes = l->second.GetSourceFiles();
    for(std::vector<cmSourceFile>::const_iterator i = classes.begin(); 
        i != classes.end(); ++i)
      {
      if(!i->GetIsAHeaderFileOnly())
        {
        cmDependInformation* info =
          this->GetDependInformation(i->GetFullPath().c_str());
        this->AddFileToSearchPath(info->m_FullPath.c_str());
        info->m_cmSourceFile = &*i;
        this->GenerateDependInformation(info);
        }
      }
    }
}


// find the full path to fname by searching the m_IncludeDirectories array
std::string cmMakeDepend::FullPath(const char* fname)
{
  if(cmSystemTools::FileExists(fname))
    {
      return std::string(fname);
    }
  
  for(std::vector<std::string>::iterator i = m_IncludeDirectories.begin();
      i != m_IncludeDirectories.end(); ++i)
    {
    std::string path = *i;
    path = path + "/";
    path = path + fname;
    if(cmSystemTools::FileExists(path.c_str()))
      {
      return path;
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
