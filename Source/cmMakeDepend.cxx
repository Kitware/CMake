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

cmMakeDepend::cmMakeDepend()
{
  m_Verbose = false;
  m_IncludeFileRegularExpression.compile("");
}


cmMakeDepend::~cmMakeDepend()
{ 
  for(DependArray::iterator i = m_DependInformation.begin();
      i != m_DependInformation.end(); ++i)
    {
    delete *i;
    }
  m_DependInformation.clear();
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
  
  // Now extract any include paths from the makefile flags
  const std::vector<std::string>& includes = m_Makefile->GetIncludeDirectories();
  std::vector<std::string>::const_iterator j;
  for(j = includes.begin(); j != includes.end(); ++j)
    {
    this->AddSearchPath(j->c_str());
    }

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
        cmDependInformation* info = new cmDependInformation;
        info->m_FullPath = this->FullPath(i->GetFullPath().c_str());
        this->AddFileToSearchPath(info->m_FullPath.c_str());
        info->m_IncludeName = i->GetFullPath();
        info->m_ClassFileIndex = &*i;
        m_DependInformation.push_back(info);
        }
      }
    }
}


// Compute the depends.
void cmMakeDepend::GenerateDependInformation()
{
  // The size of the m_DependInformation will change as
  // Depend is called so do not use an iterater but rather
  // depend on the size of the array.
  unsigned int j = 0;
  while(j != m_DependInformation.size())
    {
    cmDependInformation* info = m_DependInformation[j];
    // compute the depend information for the info object
    // this may add more objects to the m_DependInformation
    // array
    this->Depend(info);
    ++j;
    }
}

const cmDependInformation *
cmMakeDepend::GetDependInformationForSourceFile(const cmSourceFile &sf) const
{
  // Now update the depend information for each cmSourceFile
  // in the cmMakefile m_Makefile
  for(DependArray::const_iterator i = m_DependInformation.begin();
      i != m_DependInformation.end(); ++i)
    {
    cmDependInformation* info = *i;
    // find the class 
    if(info->m_ClassFileIndex == &sf)
      {
      return info;
      }
    }
  return 0;
}

const cmDependInformation *
cmMakeDepend::GetDependInformationForSourceFile(const char *fname) const
{
  // Now update the depend information for each cmSourceFile
  // in the cmMakefile m_Makefile
  for(DependArray::const_iterator i = m_DependInformation.begin();
      i != m_DependInformation.end(); ++i)
    {
    cmDependInformation* info = *i;
    // find the class 
    if(info->m_FullPath == fname)
      {
      return info;
      }
    }
  return 0;
}

void cmMakeDepend::Depend(cmDependInformation* info)
{
  const char* path = info->m_FullPath.c_str();
  if(!path)
    {
    cmSystemTools::Error("no full path for object", 0);
    return;
    }
  
  // If the file exists, use it to find dependency information.
  if(cmSystemTools::FileExists(path))
    {
    // Use the real file to find its dependencies.
    this->DependWalk(info, path);
    info->m_DependDone = true;
    return;
    }
  
  // The file doesn't exist.  See if the cmSourceFile for it has any files
  // specified as dependency hints.
  if(info->m_ClassFileIndex != 0)
    {
    // Get the cmSourceFile corresponding to this.
    const cmSourceFile& cFile = *(info->m_ClassFileIndex);
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
  cmSystemTools::Error("error cannot find dependencies for ", path);
}


// This function actually reads the file specified and scans it for
// #include directives
void cmMakeDepend::DependWalk(cmDependInformation* info, const char* file)
{
  std::ifstream fin(file);
  if(!fin)
    {
    cmSystemTools::Error("error can not open ", file);
    return;
    }
  
  char line[255];
  while(!fin.eof() && !fin.fail())
    {
    fin.getline(line, 255);
    if(!strncmp(line, "#include", 8))
      {
      // if it is an include line then create a string class
      std::string currentline = line;
      size_t qstart = currentline.find('\"', 8);
      size_t qend;
      // if a quote is not found look for a <
      if(qstart == std::string::npos)
	{
	qstart = currentline.find('<', 8);
	// if a < is not found then move on
	if(qstart == std::string::npos)
	  {
	  cmSystemTools::Error("unknown include directive ", 
                               currentline.c_str() );
	  continue;
	  }
	else
	  {
	  qend = currentline.find('>', qstart+1);
	  }
	}
      else
	{
	qend = currentline.find('\"', qstart+1);
	}
      // extract the file being included
      std::string includeFile = currentline.substr(qstart+1, qend - qstart-1);
      // see if the include matches the regular expression
      if(!m_IncludeFileRegularExpression.find(includeFile))
	{
	if(m_Verbose)
	  {
          std::string message = "Skipping ";
          message += includeFile;
          message += " for file ";
          message += file;
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
  // find the index of the include file in the
  // m_DependInformation array, if it is not
  // there then FindInformation will create it
  int index = this->FindInformation(file);
  // add the index to the depends of the current 
  // depend info object
  info->m_IndexSet.insert(index);
  // Get the depend information object for the include file
  cmDependInformation* dependInfo = m_DependInformation[index];
  // if the depends are not known for an include file, then compute them
  // recursively 
  if(!dependInfo->m_DependDone)
    {
    // stop the recursion here
    dependInfo->m_DependDone = true;
    this->Depend(dependInfo);
    }
  // add the depends of the included file to the includer
  info->MergeInfo(dependInfo);
}


// Find the cmDependInformation array index of the 
// given include file.   Create a new cmDependInformation
// object if one is not found
int cmMakeDepend::FindInformation(const char* fname)
{
  unsigned int i = 0;
  
  while(i < m_DependInformation.size())
    {
    if(m_DependInformation[i]->m_IncludeName == fname)
      {
      return i;
      }    
    ++i;
    }
  cmDependInformation* newinfo = new cmDependInformation;
  newinfo->m_FullPath = this->FullPath(fname);
  // Add the directory where this file was found to the search path
  // may have been foo/bar.h, but bar may include files from the foo
  // directory without the foo prefix
  this->AddFileToSearchPath(newinfo->m_FullPath.c_str());
  newinfo->m_IncludeName = fname;
  m_DependInformation.push_back(newinfo);
  return m_DependInformation.size()-1;
}


// add the depend information from info to the m_IndexSet varible of this class.
void cmDependInformation::MergeInfo(cmDependInformation* info)
{
  if(this != info)
    {
    for (std::set<int>::const_iterator p = info->m_IndexSet.begin(); 
         p != info->m_IndexSet.end(); ++p)
      {
      m_IndexSet.insert(*p);
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

  
