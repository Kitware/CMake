#include "cmOrderLinkDirectories.h"
#include "cmSystemTools.h"
#include "cmsys/RegularExpression.hxx"
#include <ctype.h>


//-------------------------------------------------------------------
cmOrderLinkDirectories::cmOrderLinkDirectories()
{
  this->Debug = false;
}

//-------------------------------------------------------------------
bool cmOrderLinkDirectories::LibraryInDirectory(const char* desiredLib,
                                                const char* dir,
                                                const char* libIn)
{
  // first look for the library as given
  if(this->LibraryMayConflict(desiredLib, dir, libIn))
    {
    return true;
    }
  // next remove the extension (.a, .so ) and look for the library
  // under a different name as the linker can do either
  if(this->RemoveLibraryExtension.find(libIn))
    {
    cmStdString lib = this->RemoveLibraryExtension.match(1);
    cmStdString ext = this->RemoveLibraryExtension.match(2);
    for(std::vector<cmStdString>::iterator i = this->LinkExtensions.begin();
        i != this->LinkExtensions.end(); ++i)
      {
      if(ext != *i)
        {
        std::string fname = lib;
        lib += *i;
        if(this->LibraryMayConflict(desiredLib, dir, fname.c_str()))
          {
          return true;
          } 
        }
      }
    }
  return false;
}

//-------------------------------------------------------------------
void cmOrderLinkDirectories::FindLibrariesInSearchPaths()
{
  for(std::set<cmStdString>::iterator dir = this->LinkPathSet.begin();
      dir != this->LinkPathSet.end(); ++dir)
    {
    for(std::map<cmStdString, Library>::iterator lib
          = this->FullPathLibraries.begin();
        lib != this->FullPathLibraries.end(); ++lib)
      {
      if(lib->second.Path != *dir)
        {
        if(this->LibraryInDirectory(lib->second.FullPath.c_str(),
                                    dir->c_str(), lib->second.File.c_str()))
          {
          this->LibraryToDirectories[lib->second.FullPath].push_back(*dir);
          }
        }
      }
    }
}
                 
//-------------------------------------------------------------------
void cmOrderLinkDirectories::FindIndividualLibraryOrders()
{
  for(std::vector<Library>::iterator lib = 
        this->MultiDirectoryLibraries.begin();
      lib != this->MultiDirectoryLibraries.end(); ++lib)
    {
    std::vector<cmStdString>& dirs = 
      this->LibraryToDirectories[lib->FullPath];
    std::vector<std::pair<cmStdString, std::vector<cmStdString> > 
      >::iterator i;
    for(i = this->DirectoryToAfterList.begin(); 
        i != this->DirectoryToAfterList.end(); ++i)
      {
      if(i->first == lib->Path)
        {
        break;
        }
      }
    if(i == this->DirectoryToAfterList.end())
      {
      std::cerr << "ERROR: should not happen\n";
      }
    else
      {
      for(std::vector<cmStdString>::iterator d = dirs.begin(); 
          d != dirs.end(); ++d)
        {
        i->second.push_back(*d);
        }
      }
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
  this->SplitFramework.compile("(.*)/(.*)\\.framework$");
  cmStdString libext = "(";
  bool first = true;
  for(std::vector<cmStdString>::iterator i = this->LinkExtensions.begin();
      i != this->LinkExtensions.end(); ++i)
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
  this->RemoveLibraryExtension.compile(reg.c_str());
  reg = "";
  if(this->LinkPrefix.size())
    {
    reg = "^";
    reg += this->LinkPrefix;
    }
  reg += "([^/]*)";
  reg += libext;
  this->ExtractBaseLibraryName.compile(reg.c_str());
  reg = "([^/]*)";
  reg += libext;
  this->ExtractBaseLibraryNameNoPrefix.compile(reg.c_str());
}


//-------------------------------------------------------------------
void cmOrderLinkDirectories::PrepareLinkTargets()
{
  for(std::vector<cmStdString>::iterator i = this->LinkItems.begin();
      i != this->LinkItems.end(); ++i)
    {
    // separate the library name from libfoo.a or foo.a
    if(this->ExtractBaseLibraryName.find(*i))
      {
      *i = this->ExtractBaseLibraryName.match(1);
      }
    else if(this->ExtractBaseLibraryNameNoPrefix.find(*i))
      {
      *i = this->ExtractBaseLibraryNameNoPrefix.match(1);
      }
    }
}

//-------------------------------------------------------------------
bool cmOrderLinkDirectories::FindPathNotInDirectoryToAfterList(
  cmStdString& path)
{   
  for(std::vector<std::pair<cmStdString, std::vector<cmStdString> > 
        >::iterator i = this->DirectoryToAfterList.begin();
      i != this->DirectoryToAfterList.end(); ++i)
    {
    const cmStdString& p = i->first;
    bool found = false;
    for(std::vector<std::pair<cmStdString, std::vector<cmStdString> > 
          >::iterator j = this->DirectoryToAfterList.begin(); 
        j != this->DirectoryToAfterList.end() && !found; ++j)
      {
      if(j != i)
        {
        found = (std::find(j->second.begin(), j->second.end(), p) 
                 != j->second.end());
        }
      }
    if(!found)
      {
      path = p;
      this->DirectoryToAfterList.erase(i);
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
  // this->DirectoryToAfterList once it is found
  while(this->FindPathNotInDirectoryToAfterList(path))
    {
    orderedPaths.push_back(path);
    }
  // at this point if there are still paths in this->DirectoryToAfterList
  // then there is a cycle and we are stuck
  if(this->DirectoryToAfterList.size())
    {
    for(std::vector<std::pair<cmStdString, std::vector<cmStdString> > 
          >::iterator i = this->DirectoryToAfterList.begin();
        i != this->DirectoryToAfterList.end(); ++i)
      {
      this->ImpossibleDirectories.insert(i->first);
      // still put it in the path list in the order we find them
      orderedPaths.push_back(i->first);
      }
    
    }
}

//-------------------------------------------------------------------
void cmOrderLinkDirectories::SetLinkInformation(
  const char* targetName,
  const std::vector<std::string>& linkLibraries,
  const std::vector<std::string>& linkDirectories,
  const cmTargetManifest& manifest,
  const char* configSubdir
  )
{
  // Save the target name.
  this->TargetName = targetName;

  // Save the subdirectory used for linking in this configuration.
  this->ConfigSubdir = configSubdir? configSubdir : "";

  // Merge the link directory search path given into our path set.
  std::vector<cmStdString> empty;
  for(std::vector<std::string>::const_iterator p = linkDirectories.begin();
      p != linkDirectories.end(); ++p)
    {
    std::string dir = *p;
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
    if(this->DirectoryToAfterListEmitted.insert(dir).second)
      {
      std::pair<cmStdString, std::vector<cmStdString> > dp;
      dp.first = dir;
      this->DirectoryToAfterList.push_back(dp);
      this->LinkPathSet.insert(dir);
      }
    }

  // Append the link library list into our raw list.
  for(std::vector<std::string>::const_iterator l = linkLibraries.begin();
      l != linkLibraries.end(); ++l)
    {
    this->RawLinkItems.push_back(*l);
    }

  // Construct a set of files that will exist after building.
  for(cmTargetManifest::const_iterator i = manifest.begin();
      i != manifest.end(); ++i)
    {
    for(cmTargetSet::const_iterator j = i->second.begin();
        j != i->second.end(); ++j)
      {
      this->ManifestFiles.insert(*j);
      }
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
  // do not add a -F for the system frameworks
  this->EmittedFrameworkPaths.insert("/System/Library/Frameworks");
  for(unsigned int i=0; i < this->RawLinkItems.size(); ++i)
    {
    bool framework = false;
    if(cmSystemTools::FileIsFullPath(this->RawLinkItems[i].c_str()))
      {
      if(cmSystemTools::FileIsDirectory(this->RawLinkItems[i].c_str()))
        {
        if(cmSystemTools::IsPathToFramework(this->RawLinkItems[i].c_str()))
          {
          this->SplitFramework.find(this->RawLinkItems[i]);
          cmStdString path = this->SplitFramework.match(1);
          // Add the -F path if we have not yet done so
          if(this->EmittedFrameworkPaths.insert(path).second)
            {
            std::string fpath = "-F";
            fpath += cmSystemTools::ConvertToOutputPath(path.c_str());
            this->LinkItems.push_back(fpath);
            }
          // now add the -framework option
          std::string frame = "-framework ";
          frame += this->SplitFramework.match(2);
          this->LinkItems.push_back(frame);
          framework = true;
          }
        else
          {
          std::string message = 
            "Warning: Ignoring path found in link libraries for target: ";
          message += this->TargetName;
          message += ", path is: ";
          message += this->RawLinkItems[i];
          message += 
            ". Expected a library name or a full path to a library name.";
          cmSystemTools::Message(message.c_str());
          continue;
          }
        }
      if(!framework)
        {
        dir = cmSystemTools::GetFilenamePath(this->RawLinkItems[i]);
        file = cmSystemTools::GetFilenameName(this->RawLinkItems[i]);
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
        if(this->DirectoryToAfterListEmitted.insert(dir).second)
          {
          std::pair<cmStdString, std::vector<cmStdString> > dp;
          dp.first = dir;
          this->DirectoryToAfterList.push_back(dp);
          }
        this->LinkPathSet.insert(dir);
        aLib.FullPath = this->RawLinkItems[i];
        aLib.File = file;
        aLib.Path = dir;
        this->FullPathLibraries[aLib.FullPath] = aLib;
        this->LinkItems.push_back(file);
        }
      }
    else
      {
      this->LinkItems.push_back(this->RawLinkItems[i]);
      }
    }
  this->FindLibrariesInSearchPaths();
  for(std::map<cmStdString, std::vector<cmStdString> >::iterator lib =
        this->LibraryToDirectories.begin(); 
      lib!= this->LibraryToDirectories.end(); 
      ++lib)
    {
    if(lib->second.size() > 0)
      {
      this->MultiDirectoryLibraries.push_back
        (this->FullPathLibraries[lib->first]);
      }
    else
      {
      this->SingleDirectoryLibraries.push_back
        (this->FullPathLibraries[lib->first]);
      }
    }
  this->FindIndividualLibraryOrders();
  this->SortedSearchPaths.clear();
  if(this->Debug)
    {
    this->PrintMap("this->LibraryToDirectories", this->LibraryToDirectories);
    this->PrintVector("this->DirectoryToAfterList", 
                      this->DirectoryToAfterList);
    }
  this->OrderPaths(this->SortedSearchPaths); 
  // now turn libfoo.a into foo and foo.a into foo
  // This will prepare the link items for -litem 
  this->PrepareLinkTargets();
  if(this->ImpossibleDirectories.size())
    {
    cmSystemTools::Message(this->GetWarnings().c_str());
    return false;
    }
  return true;
}

std::string cmOrderLinkDirectories::GetWarnings()
{
  std::string warning = 
    "It is impossible to order the linker search path in such a way "
    "that libraries specified as full paths will be picked by the "
    "linker.\nDirectories and libraries involved are:\n";

  for(std::set<cmStdString>::iterator i = this->ImpossibleDirectories.begin();
      i != this->ImpossibleDirectories.end(); ++i)
    {
    warning += "Directory: ";
    warning += *i;
    warning += " contains:\n";
    std::map<cmStdString, std::vector<cmStdString> >::iterator j;
    for(j = this->LibraryToDirectories.begin(); 
        j != this->LibraryToDirectories.end(); ++j)
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
//-------------------------------------------------------------------
void
cmOrderLinkDirectories::PrintVector(const char* name,
                                    std::vector<std::pair<cmStdString, 
                                    std::vector<cmStdString> > >& m)
{
  std::cout << name << "\n";
  for(std::vector<std::pair<cmStdString, std::vector<cmStdString> > 
        >::iterator i = m.begin(); i != m.end(); ++i)
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
  for(std::map<cmStdString, Library>::iterator i = 
        this->FullPathLibraries.begin();
      i != this->FullPathLibraries.end(); ++i)
    {
    libs.push_back(i->first);
    }
  
}

//----------------------------------------------------------------------------
bool cmOrderLinkDirectories::LibraryMayConflict(const char* desiredLib,
                                                const char* dir,
                                                const char* fname)
{
  // We need to check whether the given file may be picked up by the
  // linker.  This will occur if it exists as given or may be built
  // using the name given.
  bool found = false;
  std::string path = dir;
  path += "/";
  path += fname;
  if(this->ManifestFiles.find(path) != this->ManifestFiles.end())
    {
    found = true;
    }
  else if(cmSystemTools::FileExists(path.c_str()))
    {
    found = true;
    }

  // When linking with a multi-configuration build tool the
  // per-configuration subdirectory is added to each link path.  Check
  // this subdirectory too.
  if(!found && !this->ConfigSubdir.empty())
    {
    path = dir;
    path += "/";
    path += this->ConfigSubdir;
    path += "/";
    path += fname;
    if(this->ManifestFiles.find(path) != this->ManifestFiles.end())
      {
      found = true;
      }
    else if(cmSystemTools::FileExists(path.c_str()))
      {
      found = true;
      }
    }

  // A library conflicts if it is found and is not a symlink back to
  // the desired library.
  if(found)
    {
    return !cmSystemTools::SameFile(desiredLib, path.c_str());
    }
  return false;
}
