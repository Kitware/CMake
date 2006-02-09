#include "cmOrderLinkDirectories.h"
#include "cmSystemTools.h"
#include "cmsys/RegularExpression.hxx"
#include <ctype.h>


//-------------------------------------------------------------------
cmOrderLinkDirectories::cmOrderLinkDirectories()
{
  m_Debug = false;
}

//-------------------------------------------------------------------
bool cmOrderLinkDirectories::LibraryInDirectory(const char* dir, 
                                                const char* libIn)
{
  cmStdString path = dir;
  path += "/";
  path += libIn;
  // first look for the library as given
  if(cmSystemTools::FileExists(path.c_str()))
    {
    return true;
    }
  // next remove the extension (.a, .so ) and look for the library
  // under a different name as the linker can do either
  if(m_RemoveLibraryExtension.find(libIn))
    {
    cmStdString lib = m_RemoveLibraryExtension.match(1);
    cmStdString ext = m_RemoveLibraryExtension.match(2);
    for(std::vector<cmStdString>::iterator i = m_LinkExtensions.begin();
        i != m_LinkExtensions.end(); ++i)
      {
      if(ext != *i)
        {
        path = dir;
        path += "/";
        path += lib + *i;
        if(cmSystemTools::FileExists(path.c_str()))
          {
          return true;
          } 
        }
      }
    }
  return false;
}

//-------------------------------------------------------------------
void cmOrderLinkDirectories::FindLibrariesInSeachPaths()
{
  for(std::set<cmStdString>::iterator dir = m_LinkPathSet.begin();
      dir != m_LinkPathSet.end(); ++dir)
    {
    for(std::map<cmStdString, Library>::iterator lib
          = m_FullPathLibraries.begin();
        lib != m_FullPathLibraries.end(); ++lib)
      {
      if(lib->second.Path != *dir)
        {
        if(this->LibraryInDirectory(dir->c_str(), lib->second.File.c_str()))
          {
          m_LibraryToDirectories[lib->second.FullPath].push_back(*dir);
          }
        }
      }
    }
}
                 
//-------------------------------------------------------------------
void cmOrderLinkDirectories::FindIndividualLibraryOrders()
{
  for(std::vector<Library>::iterator lib = m_MultiDirectoryLibraries.begin();
      lib != m_MultiDirectoryLibraries.end(); ++lib)
    {
    std::vector<cmStdString>& dirs = m_LibraryToDirectories[lib->FullPath];
    m_DirectoryToAfterList[lib->Path] = dirs;
    }
}

//-------------------------------------------------------------------
std::string cmOrderLinkDirectories::NoCaseExpression(const char* str)
{
  std::string ret;
  const char* s = str;
  while(*s)
    {
    if(*s == '.')
      {
      ret += *s;
      }
    else
      {
      ret += "[";
      ret += tolower(*s);
      ret += toupper(*s);
      ret += "]";
      }
    s++;
    }
  return ret;
}
    
//-------------------------------------------------------------------
void cmOrderLinkDirectories::CreateRegularExpressions()
{
  m_SplitFramework.compile("(.*)/(.*)\\.framework$");
  cmStdString libext = "(";
  bool first = true;
  for(std::vector<cmStdString>::iterator i = m_LinkExtensions.begin();
      i != m_LinkExtensions.end(); ++i)
    {
    if(!first)
      {
      libext += "|";
      }
    first = false;
    libext += "\\";
#if defined(_WIN32) && !defined(__CYGWIN__)
    libext += this->NoCaseExpression(i->c_str());
#else
    libext += *i;
#endif
    }
  libext += ").*";
  cmStdString reg("(.*)");
  reg += libext;
  m_RemoveLibraryExtension.compile(reg.c_str());
  reg = "";
  if(m_LinkPrefix.size())
    {
    reg = "^";
    reg += m_LinkPrefix;
    }
  reg += "([^/]*)";
  reg += libext;
  m_ExtractBaseLibraryName.compile(reg.c_str());
  reg = "([^/]*)";
  reg += libext;
  m_ExtractBaseLibraryNameNoPrefix.compile(reg.c_str());
}


//-------------------------------------------------------------------
void cmOrderLinkDirectories::PrepareLinkTargets()
{
  for(std::vector<cmStdString>::iterator i = m_LinkItems.begin();
      i != m_LinkItems.end(); ++i)
    {
    // separate the library name from libfoo.a or foo.a
    if(m_ExtractBaseLibraryName.find(*i))
      {
      *i = m_ExtractBaseLibraryName.match(1);
      }
    else if(m_ExtractBaseLibraryNameNoPrefix.find(*i))
      {
      *i = m_ExtractBaseLibraryNameNoPrefix.match(1);
      }
    }
}

//-------------------------------------------------------------------
bool cmOrderLinkDirectories::FindPathNotInDirectoryToAfterList(
  cmStdString& path)
{
  for(std::map<cmStdString, std::vector<cmStdString> >::iterator i
        = m_DirectoryToAfterList.begin();
      i != m_DirectoryToAfterList.end(); ++i)
    {
    const cmStdString& p = i->first;
    bool found = false;
    for(std::map<cmStdString, std::vector<cmStdString> >::iterator j 
          = m_DirectoryToAfterList.begin(); j != m_DirectoryToAfterList.end() 
          && !found; ++j)
      {
      if(j != i)
        {
        found = (std::find(j->second.begin(), j->second.end(), p) != j->second.end());
        }
      }
    if(!found)
      {
      path = p;
      m_DirectoryToAfterList.erase(i);
      return true;
      }
    }
  path = "";
  return false;
}


//-------------------------------------------------------------------
void cmOrderLinkDirectories::OrderPaths(std::vector<cmStdString>&
                                        orderedPaths)
{
  cmStdString path;
  // This is a topological sort implementation
  // One at a time find paths that are not in any other paths after list
  // and put them into the orderedPaths vector in that order
  // FindPathNotInDirectoryToAfterList removes the path from the
  // m_DirectoryToAfterList once it is found
  while(this->FindPathNotInDirectoryToAfterList(path))
    {
    orderedPaths.push_back(path);
    }
  // at this point if there are still paths in m_DirectoryToAfterList
  // then there is a cycle and we are stuck
  if(m_DirectoryToAfterList.size())
    {
    for(std::map<cmStdString, std::vector<cmStdString> >::iterator i
          = m_DirectoryToAfterList.begin();
        i != m_DirectoryToAfterList.end(); ++i)
      {
      m_ImpossibleDirectories.insert(i->first);
      // still put it in the path list in the order we find them
      orderedPaths.push_back(i->first);
      }
    
    }
}

//-------------------------------------------------------------------
void cmOrderLinkDirectories::SetLinkInformation(
  const char* targetName,
  const std::vector<std::string>& linkLibraries,
  const std::vector<std::string>& linkDirectories
  )
{
  // Save the target name.
  m_TargetName = targetName;

  // Merge the link directory search path given into our path set.
  std::vector<cmStdString> empty;
  for(std::vector<std::string>::const_iterator p = linkDirectories.begin();
      p != linkDirectories.end(); ++p)
    {
    m_DirectoryToAfterList[*p] = empty;
    m_LinkPathSet.insert(*p);
    }

  // Append the link library list into our raw list.
  for(std::vector<std::string>::const_iterator l = linkLibraries.begin();
      l != linkLibraries.end(); ++l)
    {
    m_RawLinkItems.push_back(*l);
    }
}

//-------------------------------------------------------------------
bool cmOrderLinkDirectories::DetermineLibraryPathOrder()
{
  // set up all the regular expressions
  this->CreateRegularExpressions();
  std::vector<cmStdString> finalOrderPaths;
  // find all libs that are full paths
  Library aLib;
  cmStdString dir;
  cmStdString file;
  std::vector<cmStdString> empty;
  bool framework = false;
  for(unsigned int i=0; i < m_RawLinkItems.size(); ++i)
    {
    if(cmSystemTools::FileIsFullPath(m_RawLinkItems[i].c_str()))
      {
      if(cmSystemTools::FileIsDirectory(m_RawLinkItems[i].c_str()))
        {
        if(cmSystemTools::IsPathToFramework(m_RawLinkItems[i].c_str()))
          {
          m_SplitFramework.find(m_RawLinkItems[i]);
          cmStdString path = m_SplitFramework.match(1);
          // Add the -F path if we have not yet done so
          if(m_EmittedFrameworkPaths.insert(path).second)
            {
            std::string fpath = "-F";
            fpath += cmSystemTools::ConvertToOutputPath(path.c_str());
            m_LinkItems.push_back(fpath);
            }
          // now add the -framework option
          std::string frame = "-framework ";
          frame += m_SplitFramework.match(2);
          m_LinkItems.push_back(frame);
          framework = true;
          }
        else
          {
          std::string message = "Warning: Ignoring path found in link libraries for target: ";
          message += m_TargetName;
          message += ", path is: ";
          message += m_RawLinkItems[i];
          message += ". Expected a library name or a full path to a library name.";
          cmSystemTools::Message(message.c_str());
          continue;
          }
        }
      if(!framework)
        {
        cmSystemTools::SplitProgramPath(m_RawLinkItems[i].c_str(),
                                        dir, file);
#ifdef _WIN32
        // Avoid case problems for windows paths.
        if(dir.size() > 2 && dir[1] == ':')
          {
          if(dir[0] >= 'A' && dir[0] <= 'Z')
            {
            dir[0] += 'a' - 'A';
            }
          }
        dir = cmSystemTools::GetActualCaseForPath(dir.c_str());
#endif
        m_DirectoryToAfterList[dir] = empty;
        m_LinkPathSet.insert(dir);
        aLib.FullPath = m_RawLinkItems[i];
        aLib.File = file;
        aLib.Path = dir;
        m_FullPathLibraries[aLib.FullPath] = aLib;
        m_LinkItems.push_back(file);
        }
      }
    else
      {
      m_LinkItems.push_back(m_RawLinkItems[i]);
      }
    }
  this->FindLibrariesInSeachPaths();
  for(std::map<cmStdString, std::vector<cmStdString> >::iterator lib =
        m_LibraryToDirectories.begin(); lib!= m_LibraryToDirectories.end(); 
      ++lib)
    {
    if(lib->second.size() > 0)
      {
      m_MultiDirectoryLibraries.push_back(m_FullPathLibraries[lib->first]);
      }
    else
      {
      m_SingleDirectoryLibraries.push_back(m_FullPathLibraries[lib->first]);
      }
    }
  this->FindIndividualLibraryOrders();
  m_SortedSearchPaths.clear();
  if(m_Debug)
    {
    this->PrintMap("m_LibraryToDirectories", m_LibraryToDirectories);
    this->PrintMap("m_DirectoryToAfterList", m_DirectoryToAfterList);
    }
  this->OrderPaths(m_SortedSearchPaths); 
  // now turn libfoo.a into foo and foo.a into foo
  // This will prepare the link items for -litem 
  this->PrepareLinkTargets();
  if(m_ImpossibleDirectories.size())
    {
    cmSystemTools::Message(this->GetWarnings().c_str());
    return false;
    }
  return true;
}

std::string cmOrderLinkDirectories::GetWarnings()
{
  std::string warning = "It is impossible to order the linker search path in such a way that libraries specified as full paths will be picked by the linker.\nDirectories and libraries involved are:\n";
  for(std::set<cmStdString>::iterator i = m_ImpossibleDirectories.begin();
      i != m_ImpossibleDirectories.end(); ++i)
    {
    warning += "Directory: ";
    warning += *i;
    warning += " contains:\n";
    std::map<cmStdString, std::vector<cmStdString> >::iterator j;
    for(j = m_LibraryToDirectories.begin(); 
        j != m_LibraryToDirectories.end(); ++j)
      {
      if(std::find(j->second.begin(), j->second.end(), *i)
         != j->second.end())
        {
        warning += "Library: ";
        warning += j->first;
        warning += "\n";
        }
      }
    warning += "\n";
    }
  warning += "\n";
  return warning;
}

//-------------------------------------------------------------------
void
cmOrderLinkDirectories::PrintMap(const char* name,
                       std::map<cmStdString, std::vector<cmStdString> >& m)
{
  std::cout << name << "\n";
  for(std::map<cmStdString, std::vector<cmStdString> >::iterator i =
        m.begin(); i != m.end();
      ++i)
    {
    std::cout << i->first << ":  ";
    for(std::vector<cmStdString>::iterator l = i->second.begin();
        l != i->second.end(); ++l)
      {
      std::cout << *l << " ";
      }
    std::cout << "\n";
    }
}

void cmOrderLinkDirectories::GetFullPathLibraries(std::vector<cmStdString>& 
                                                  libs)
{
  for(std::map<cmStdString, Library>::iterator i = m_FullPathLibraries.begin();
      i != m_FullPathLibraries.end(); ++i)
    {
    libs.push_back(i->first);
    }
  
}
