#include "cmOrderLinkDirectories.h"
#include "cmSystemTools.h"
#include "cmsys/RegularExpression.hxx"



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
        if(LibraryInDirectory(dir->c_str(), lib->second.File.c_str()))
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
void cmOrderLinkDirectories::CreateRegularExpressions()
{
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
    libext += *i;
    }
  libext += ").*";
  cmStdString reg("(.*)");
  reg += libext;
  m_RemoveLibraryExtension.compile(reg.c_str());
  reg = "^lib([^/]*)";
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
bool cmOrderLinkDirectories::CanBeBefore(const cmStdString& d1,
                                         const cmStdString& d2)
{
  if(m_DirectoryToAfterList.count(d2) == 0)
    {
    return true;
    }
  std::vector<cmStdString>& d2dirs = m_DirectoryToAfterList[d2];
  // is d1 in the d2's list of directories that d2 must be before
  // if so, then d1 can not come before d2
  for(std::vector<cmStdString>::iterator i = d2dirs.begin();
      i != d2dirs.end(); ++i)
    {
    if(*i == d1)
      {
      return false;
      }
    }
  return true;
}

// This is a stl function object used to sort
// the vector of library paths.  It returns true
// if left directory can be before right directory (no swap).
// It also checks for the impossible case of two libraries and
// two directories that have both libraries.
struct cmOrderLinkDirectoriesCompare
  : public std::binary_function <cmStdString, cmStdString, bool> 
{
  cmOrderLinkDirectoriesCompare() 
  {
    This = 0;
  }
  bool operator()(
                  const cmStdString& left, 
                  const cmStdString& right
                  ) const
  {
    bool ret = This->CanBeBefore(left, right);
    if(!ret)
      {
      // check for the case when both libraries have to come
      // before each other 
      if(!This->CanBeBefore(right, left))
        {
        This->AddImpossible(right, left);
        }
      }
    return ret;
  }
  cmOrderLinkDirectories* This;
};

//-------------------------------------------------------------------
void cmOrderLinkDirectories::AddImpossible(const cmStdString& d1,
                                           const cmStdString& d2)
{
  m_ImposibleDirectories.insert(d1);
  m_ImposibleDirectories.insert(d2);
}

//-------------------------------------------------------------------
void cmOrderLinkDirectories::OrderPaths(std::vector<cmStdString>&
                                        orderedPaths)
{
  cmOrderLinkDirectoriesCompare comp;
  comp.This = this;
  // for some reason when cmake is run on InsightApplication 
  // if std::sort is used here cmake crashes, but stable_sort works
  std::stable_sort(orderedPaths.begin(), orderedPaths.end(), comp);
}

//-------------------------------------------------------------------
void cmOrderLinkDirectories::SetLinkInformation(const cmTarget& target,
                                                cmTarget::LinkLibraryType 
                                                linktype,
                                                const char* targetLibrary)
{
    // collect the search paths from the target into paths set
  const std::vector<std::string>& searchPaths = target.GetLinkDirectories();
  for(std::vector<std::string>::const_iterator p = searchPaths.begin();
      p != searchPaths.end(); ++p)
    {
    m_LinkPathSet.insert(*p);
    }
  // collect the link items from the target and put it into libs
  const cmTarget::LinkLibraries& tlibs = target.GetLinkLibraries();
  std::vector<cmStdString>  libs;
  for(cmTarget::LinkLibraries::const_iterator lib = tlibs.begin();
      lib != tlibs.end(); ++lib)
    {
    // skip zero size library entries, this may happen
    // if a variable expands to nothing.
    if (lib->first.size() == 0)
      {
      continue;
      }
    // Don't link the library against itself!
    if(targetLibrary && (lib->first == targetLibrary))
      {
      continue;
      }  
    // use the correct lib for the current configuration
    if (lib->second == cmTarget::DEBUG && linktype != cmTarget::DEBUG)
      {
      continue;
      }
    if (lib->second == cmTarget::OPTIMIZED && 
        linktype != cmTarget::OPTIMIZED)
      {
      continue;
      }
    m_RawLinkItems.push_back(lib->first);
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
  for(unsigned int i=0; i < m_RawLinkItems.size(); ++i)
    {
    if(cmSystemTools::FileIsFullPath(m_RawLinkItems[i].c_str()))
      {
      cmSystemTools::SplitProgramPath(m_RawLinkItems[i].c_str(),
                                      dir, file);
      m_LinkPathSet.insert(dir);
      aLib.FullPath = m_RawLinkItems[i];
      aLib.File = file;
      aLib.Path = dir;
      m_FullPathLibraries[aLib.FullPath] = aLib;
      m_LinkItems.push_back(file);
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
  for(std::set<cmStdString>::iterator i = m_LinkPathSet.begin();
      i != m_LinkPathSet.end(); ++i)
    {
    m_SortedSearchPaths.push_back(*i);
    }
  
  this->OrderPaths(m_SortedSearchPaths);
  // now turn libfoo.a into foo and foo.a into foo
  // This will prepare the link items for -litem 
  this->PrepareLinkTargets();
  if(m_ImposibleDirectories.size())
    {
    cmSystemTools::Message(this->GetWarnings().c_str());
    return false;
    }
  return true;
}

std::string cmOrderLinkDirectories::GetWarnings()
{
  std::string warning = "It is impossible to order the linker search path in such a way that libraries specified as full paths will be picked by the linker.\nDirectories and libraries involvied are:\n";
  for(std::set<cmStdString>::iterator i = m_ImposibleDirectories.begin();
      i != m_ImposibleDirectories.end(); ++i)
    {
    warning += "Directory: ";
    warning += *i;
    warning += " contains ";
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
    }
  warning += "\n";
  return warning;
}

//-------------------------------------------------------------------
void
cmOrderLinkDirectories::PrintMap(const char* name,
                       std::map<cmStdString, std::vector<cmStdString> >& m)
{
  std::cerr << name << "\n";
  for(std::map<cmStdString, std::vector<cmStdString> >::iterator i =
        m.begin(); i != m.end();
      ++i)
    {
    std::cerr << i->first << ":  ";
    for(std::vector<cmStdString>::iterator l = i->second.begin();
        l != i->second.end(); ++l)
      {
      std::cerr << *l << " ";
      }
    std::cerr << "\n";
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
