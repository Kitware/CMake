#ifdef _MSC_VER
#pragma warning ( disable : 4786 )
#endif
#include "cmMakeDepend.h"
#include "cmSystemTools.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <functional>


cmMakeDepend::cmMakeDepend()
{
  m_Verbose = false;
  m_IncludeFileRegularExpression.compile("^itk|^vtk|^vnl|^vcl|^f2c");
}


// only files matching this regular expression with be considered
void cmMakeDepend::SetIncludeRegularExpression(const char* prefix)
{
  m_IncludeFileRegularExpression.compile(prefix);
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
// The pointer is kept so the cmClassFile array can
// be updated with the depend information in the cmMakefile.

void cmMakeDepend::SetMakefile(cmMakefile* makefile)
{
  m_Makefile = makefile;
  
  // Now extract any include paths from the makefile flags
  cmCollectFlags& flags = m_Makefile->GetBuildFlags();
  std::vector<std::string>& includes = flags.GetIncludeDirectories();
  std::vector<std::string>::iterator j;
  for(j = includes.begin(); j != includes.end(); ++j)
    {
    cmSystemTools::ReplaceString(*j, "${CMAKE_CONFIG_DIR}",
				 m_Makefile->GetOutputHomeDirectory() );
    cmSystemTools::ReplaceString(*j, "${srcdir}",
				 m_Makefile->GetHomeDirectory() );
    this->AddSearchPath(j->c_str());
    }
  // Now create cmDependInformation objects for files in the directory
  int index = 0;
  std::vector<cmClassFile>::iterator i = makefile->m_Classes.begin();
  while(i != makefile->m_Classes.end())
    {
    if(!(*i).m_HeaderFileOnly)
      {
      cmDependInformation* info = new cmDependInformation;
      info->m_FullPath = this->FullPath((*i).m_FullPath.c_str());
      this->AddFileToSearchPath(info->m_FullPath.c_str());
      info->m_IncludeName = (*i).m_FullPath;
      m_DependInformation.push_back(info);
      info->m_ClassFileIndex = index;
      }
    ++i;
    index++;
    }
}


// Compute the depends.
void cmMakeDepend::DoDepends()
{
  // The size of the m_DependInformation will change as
  // Depend is called so do not use an iterater but rather
  // depend on the size of the array.
  int j = 0;
  while(j != m_DependInformation.size())
    {
    cmDependInformation* info = m_DependInformation[j];
    // compute the depend information for the info object
    // this may add more objects to the m_DependInformation
    // array
    this->Depend(info);
    ++j;
    }
  // Now update the depend information for each cmClassFile
  // in the cmMakefile m_Makefile
  for(DependArray::iterator i = m_DependInformation.begin();
      i != m_DependInformation.end(); ++i)
    {
    cmDependInformation* info = *i;
    // Remove duplicate depends
    info->RemoveDuplicateIndices();
    std::vector<cmClassFile>::iterator j = m_Makefile->m_Classes.begin();
    // find the class 
    if(info->m_ClassFileIndex != -1)
      {
      cmClassFile& cfile = m_Makefile->m_Classes[info->m_ClassFileIndex];
      for( std::vector<int>::iterator indx = info->m_Indices.begin();
	   indx != info->m_Indices.end(); ++indx)
	{
	cfile.m_Depends.push_back(m_DependInformation[*indx]->m_FullPath);
	}
      }
    }
}

// This function actually reads the file
// and scans it for #include directives
void cmMakeDepend::Depend(cmDependInformation* info)
{
  const char* path = info->m_FullPath.c_str();
  if(!path)
    {
    std::cerr << "no full path for object"  << std::endl;
    return;
    }
  
  std::ifstream fin(path);
  if(!fin)
    {
    std::cerr << "error can not open " << info->m_FullPath << std::endl;
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
	  std::cerr << "unknown include directive " << currentline 
		    << std::endl;
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
	  std::cerr  << "skipping " << includeFile << " for file " << path << std::endl;
	  }
	continue;
	}
      // find the index of the include file in the
      // m_DependInformation array, if it is not
      // there then FindInformation will create it
      int index = this->FindInformation(includeFile.c_str());
      // add the index to the depends of the current 
      // depend info object
      info->m_Indices.push_back(index);
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
    }
  info->m_DependDone = true;
}


// Find the cmDependInformation array index of the 
// given include file.   Create a new cmDependInformation
// object if one is not found
int cmMakeDepend::FindInformation(const char* fname)
{
  int i = 0;
  
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

// remove duplicate indices from the depend information 
void cmDependInformation::RemoveDuplicateIndices()
{
  // sort the array
  std::sort(m_Indices.begin(), m_Indices.end(), std::less<int>());
  // remove duplicates
  std::vector<int>::iterator new_end = 
    std::unique(m_Indices.begin(), m_Indices.end());
  m_Indices.erase(new_end, m_Indices.end());
}

// add the depend information from info to the m_Indices varible of this class.
void cmDependInformation::MergeInfo(cmDependInformation* info)
{
  std::vector<int>::iterator i = info->m_Indices.begin();
  for(; i!= info->m_Indices.end(); ++i)
    {
    m_Indices.push_back(*i);
    }
}

// find the full path to fname by searching the m_IncludeDirectories array
std::string cmMakeDepend::FullPath(const char* fname)
{
  for(std::vector<std::string>::iterator i = m_IncludeDirectories.begin();
      i != m_IncludeDirectories.end(); ++i)
    {
    if(cmSystemTools::FileExists(fname))
      {
      return std::string(fname);
      }
    std::string path = *i;
    path = path + "/";
    path = path + fname;
    if(cmSystemTools::FileExists(path.c_str()))
      {
      return path;
      }
    }
  std::cerr << "Depend: File not found " << fname  << std::endl;
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
    if(std::find(m_IncludeDirectories.begin(), m_IncludeDirectories.end(), path)
       == m_IncludeDirectories.end())
      {
      m_IncludeDirectories.push_back(path);
      return;
      }
    }
}

  
