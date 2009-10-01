/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmFindLibraryCommand.h"
#include "cmCacheManager.h"
#include <cmsys/Directory.hxx>
#include <cmsys/stl/algorithm>

cmFindLibraryCommand::cmFindLibraryCommand()
{ 
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "FIND_XXX", "find_library");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "CMAKE_XXX_PATH", "CMAKE_LIBRARY_PATH");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "CMAKE_XXX_MAC_PATH",
                               "CMAKE_FRAMEWORK_PATH");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "CMAKE_SYSTEM_XXX_MAC_PATH",
                               "CMAKE_SYSTEM_FRAMEWORK_PATH");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "XXX_SYSTEM", "LIB");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "CMAKE_SYSTEM_XXX_PATH", 
                               "CMAKE_SYSTEM_LIBRARY_PATH");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "SEARCH_XXX_DESC", "library");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "SEARCH_XXX", "library");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "XXX_SUBDIR", "lib");
  cmSystemTools::ReplaceString(this->GenericDocumentation,
                               "CMAKE_FIND_ROOT_PATH_MODE_XXX", 
                               "CMAKE_FIND_ROOT_PATH_MODE_LIBRARY");

  this->EnvironmentPath = "LIB";
  this->GenericDocumentation += 
    "\n"
    "If the library found is a framework, then VAR will be set to "
    "the full path to the framework <fullPath>/A.framework. "
    "When a full path to a framework is used as a library, "
    "CMake will use a -framework A, and a -F<fullPath> to "
    "link the framework to the target. ";
}

// cmFindLibraryCommand
bool cmFindLibraryCommand
::InitialPass(std::vector<std::string> const& argsIn, cmExecutionStatus &)
{
  this->VariableDocumentation = "Path to a library.";
  this->CMakePathName = "LIBRARY";
  if(!this->ParseArguments(argsIn))
    {
    return false;
    }
  if(this->AlreadyInCache)
    {
    // If the user specifies the entry on the command line without a
    // type we should add the type and docstring but keep the original
    // value.
    if(this->AlreadyInCacheWithoutMetaInfo)
      {
      this->Makefile->AddCacheDefinition(this->VariableName.c_str(), "",
                                         this->VariableDocumentation.c_str(),
                                         cmCacheManager::FILEPATH);
      }
    return true;
    }

  if(const char* abi_name =
     this->Makefile->GetDefinition("CMAKE_INTERNAL_PLATFORM_ABI"))
    {
    std::string abi = abi_name;
    if(abi.find("ELF N32") != abi.npos)
      {
      // Convert lib to lib32.
      this->AddArchitecturePaths("32");
      }
    }

  if(this->Makefile->GetCMakeInstance()
     ->GetPropertyAsBool("FIND_LIBRARY_USE_LIB64_PATHS"))
    {
    // add special 64 bit paths if this is a 64 bit compile.
    this->AddLib64Paths();
    }

  std::string library = this->FindLibrary();
  if(library != "")
    {
    // Save the value in the cache
    this->Makefile->AddCacheDefinition(this->VariableName.c_str(),
                                       library.c_str(),
                                       this->VariableDocumentation.c_str(),
                                       cmCacheManager::FILEPATH);
    return true;
    }
  std::string notfound = this->VariableName + "-NOTFOUND";
  this->Makefile->AddCacheDefinition(this->VariableName.c_str(),
                                     notfound.c_str(),
                                     this->VariableDocumentation.c_str(),
                                     cmCacheManager::FILEPATH);
  return true;
}

//----------------------------------------------------------------------------
void cmFindLibraryCommand::AddArchitecturePaths(const char* suffix)
{
  std::vector<std::string> newPaths;
  bool found = false;
  std::string subpath = "lib";
  subpath += suffix;
  subpath += "/";
  for(std::vector<std::string>::iterator i = this->SearchPaths.begin();
      i != this->SearchPaths.end(); ++i)
    {
    // Try replacing lib/ with lib<suffix>/
    std::string s = *i;
    cmSystemTools::ReplaceString(s, "lib/", subpath.c_str());
    if((s != *i) && cmSystemTools::FileIsDirectory(s.c_str()))
      {
      found = true;
      newPaths.push_back(s);
      }

    // Now look for lib<suffix>
    s = *i;
    s += suffix;
    if(cmSystemTools::FileIsDirectory(s.c_str()))
      {
      found = true;
      newPaths.push_back(s);
      }
    // now add the original unchanged path
    if(cmSystemTools::FileIsDirectory(i->c_str()))
      {
      newPaths.push_back(*i);
      }
    }

  // If any new paths were found replace the original set.
  if(found)
    {
    this->SearchPaths = newPaths;
    }
}

void cmFindLibraryCommand::AddLib64Paths()
{  
  if(!this->Makefile->GetLocalGenerator()->GetGlobalGenerator()->
     GetLanguageEnabled("C"))
    {
    return;
    }
  std::string voidsize =
    this->Makefile->GetSafeDefinition("CMAKE_SIZEOF_VOID_P");
  int size = atoi(voidsize.c_str());
  if(size != 8)
    {
    return;
    }
  std::vector<std::string> path64;
  bool found64 = false;
  for(std::vector<std::string>::iterator i = this->SearchPaths.begin(); 
      i != this->SearchPaths.end(); ++i)
    {
    std::string s = *i;
    std::string s2 = *i;
    cmSystemTools::ReplaceString(s, "lib/", "lib64/");
    // try to replace lib with lib64 and see if it is there,
    // then prepend it to the path
    // Note that all paths have trailing slashes.
    if((s != *i) && cmSystemTools::FileIsDirectory(s.c_str()))
      {
      path64.push_back(s);
      found64 = true;
      }  
    // now just add a 64 to the path name and if it is there,
    // add it to the path
    s2 += "64/";
    if(cmSystemTools::FileIsDirectory(s2.c_str()))
      {
      found64 = true;
      path64.push_back(s2);
      } 
    // now add the original unchanged path
    if(cmSystemTools::FileIsDirectory(i->c_str()))
      {
      path64.push_back(*i);
      }
    }
  // now replace the SearchPaths with the 64 bit converted path
  // if any 64 bit paths were discovered
  if(found64)
    {
    this->SearchPaths = path64;
    }
}

//----------------------------------------------------------------------------
std::string cmFindLibraryCommand::FindLibrary()
{
  std::string library;
  if(this->SearchFrameworkFirst || this->SearchFrameworkOnly)
    {
    library = this->FindFrameworkLibrary();
    }
  if(library.empty() && !this->SearchFrameworkOnly)
    {
    library = this->FindNormalLibrary();
    }
  if(library.empty() && this->SearchFrameworkLast)
    {
    library = this->FindFrameworkLibrary();
    }
  return library;
}

//----------------------------------------------------------------------------
struct cmFindLibraryHelper
{
  cmFindLibraryHelper(cmMakefile* mf);

  // Context information.
  cmMakefile* Makefile;
  cmGlobalGenerator* GG;

  // List of valid prefixes and suffixes.
  std::vector<std::string> Prefixes;
  std::vector<std::string> Suffixes;
  std::string PrefixRegexStr;
  std::string SuffixRegexStr;

  // Keep track of the best library file found so far.
  typedef std::vector<std::string>::size_type size_type;
  std::string BestPath;
  size_type BestPrefix;
  size_type BestSuffix;

  // Support for OpenBSD shared library naming: lib<name>.so.<major>.<minor>
  bool OpenBSD;
  unsigned int BestMajor;
  unsigned int BestMinor;

  // Current name under consideration.
  cmsys::RegularExpression NameRegex;
  bool TryRawName;
  std::string RawName;

  // Current full path under consideration.
  std::string TestPath;

  void RegexFromLiteral(std::string& out, std::string const& in);
  void RegexFromList(std::string& out, std::vector<std::string> const& in);
  size_type GetPrefixIndex(std::string const& prefix)
    {
    return cmsys_stl::find(this->Prefixes.begin(), this->Prefixes.end(),
                           prefix) - this->Prefixes.begin();
    }
  size_type GetSuffixIndex(std::string const& suffix)
    {
    return cmsys_stl::find(this->Suffixes.begin(), this->Suffixes.end(),
                           suffix) - this->Suffixes.begin();
    }
  bool HasValidSuffix(std::string const& name);
  void SetName(std::string const& name);
  bool CheckDirectory(std::string const& path);
};

//----------------------------------------------------------------------------
cmFindLibraryHelper::cmFindLibraryHelper(cmMakefile* mf):
  Makefile(mf)
{
  this->GG = this->Makefile->GetLocalGenerator()->GetGlobalGenerator();

  // Collect the list of library name prefixes/suffixes to try.
  const char* prefixes_list =
    this->Makefile->GetRequiredDefinition("CMAKE_FIND_LIBRARY_PREFIXES");
  const char* suffixes_list =
    this->Makefile->GetRequiredDefinition("CMAKE_FIND_LIBRARY_SUFFIXES");
  cmSystemTools::ExpandListArgument(prefixes_list, this->Prefixes, true);
  cmSystemTools::ExpandListArgument(suffixes_list, this->Suffixes, true);
  this->RegexFromList(this->PrefixRegexStr, this->Prefixes);
  this->RegexFromList(this->SuffixRegexStr, this->Suffixes);

  // Check whether to use OpenBSD-style library version comparisons.
  this->OpenBSD =
    this->Makefile->GetCMakeInstance()
    ->GetPropertyAsBool("FIND_LIBRARY_USE_OPENBSD_VERSIONING");

  this->TryRawName = false;

  // No library file has yet been found.
  this->BestPrefix = this->Prefixes.size();
  this->BestSuffix = this->Suffixes.size();
  this->BestMajor = 0;
  this->BestMinor = 0;
}

//----------------------------------------------------------------------------
void cmFindLibraryHelper::RegexFromLiteral(std::string& out,
                                           std::string const& in)
{
  for(std::string::const_iterator ci = in.begin(); ci != in.end(); ++ci)
    {
    char ch = *ci;
    if(ch == '[' || ch == ']' || ch == '(' || ch == ')' || ch == '\\' ||
       ch == '.' || ch == '*' || ch == '+' || ch == '?' || ch == '-' ||
       ch == '^' || ch == '$')
      {
      out += "\\";
      }
#if defined(_WIN32) || defined(__APPLE__)
    out += tolower(ch);
#else
    out += ch;
#endif
    }
}

//----------------------------------------------------------------------------
void cmFindLibraryHelper::RegexFromList(std::string& out,
                                        std::vector<std::string> const& in)
{
  // Surround the list in parens so the '|' does not apply to anything
  // else and the result can be checked after matching.
  out += "(";
  const char* sep = "";
  for(std::vector<std::string>::const_iterator si = in.begin();
      si != in.end(); ++si)
    {
    // Separate from previous item.
    out += sep;
    sep = "|";

    // Append this item.
    this->RegexFromLiteral(out, *si);
    }
  out += ")";
}

//----------------------------------------------------------------------------
bool cmFindLibraryHelper::HasValidSuffix(std::string const& name)
{
  // Check if the given name ends in a valid library suffix.
  for(std::vector<std::string>::const_iterator si = this->Suffixes.begin();
      si != this->Suffixes.end(); ++si)
    {
    std::string const& suffix = *si;
    if(name.length() > suffix.length() &&
       name.substr(name.size()-suffix.length()) == suffix)
      {
      return true;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
void cmFindLibraryHelper::SetName(std::string const& name)
{
  // Consider checking the raw name too.
  this->TryRawName = this->HasValidSuffix(name);
  this->RawName = name;

  // Build a regular expression to match library names.
  std::string regex = "^";
  regex += this->PrefixRegexStr;
  this->RegexFromLiteral(regex, name);
  regex += this->SuffixRegexStr;
  if(this->OpenBSD)
    {
    regex += "(\\.[0-9]+\\.[0-9]+)?";
    }
  regex += "$";
  this->NameRegex.compile(regex.c_str());
}

//----------------------------------------------------------------------------
bool cmFindLibraryHelper::CheckDirectory(std::string const& path)
{
  // If the original library name provided by the user matches one of
  // the suffixes, try it first.  This allows users to search
  // specifically for a static library on some platforms (on MS tools
  // one cannot tell just from the library name whether it is a static
  // library or an import library).
  if(this->TryRawName)
    {
    this->TestPath = path;
    this->TestPath += this->RawName;
    if(cmSystemTools::FileExists(this->TestPath.c_str(), true))
      {
      this->BestPath =
        cmSystemTools::CollapseFullPath(this->TestPath.c_str());
      cmSystemTools::ConvertToUnixSlashes(this->BestPath);
      return true;
      }
    }

  // Search for a file matching the library name regex.
  std::string dir = path;
  cmSystemTools::ConvertToUnixSlashes(dir);
  std::set<cmStdString> const& files = this->GG->GetDirectoryContent(dir);
  for(std::set<cmStdString>::const_iterator fi = files.begin();
      fi != files.end(); ++fi)
    {
    std::string const& origName = *fi;
#if defined(_WIN32) || defined(__APPLE__)
    std::string testName = cmSystemTools::LowerCase(origName);
#else
    std::string const& testName = origName;
#endif
    if(this->NameRegex.find(testName))
      {
      this->TestPath = path;
      this->TestPath += origName;
      if(!cmSystemTools::FileIsDirectory(this->TestPath.c_str()))
        {
        // This is a matching file.  Check if it is better than the
        // best name found so far.  Earlier prefixes are preferred,
        // followed by earlier suffixes.  For OpenBSD, shared library
        // version extensions are compared.
        size_type prefix = this->GetPrefixIndex(this->NameRegex.match(1));
        size_type suffix = this->GetSuffixIndex(this->NameRegex.match(2));
        unsigned int major = 0;
        unsigned int minor = 0;
        if(this->OpenBSD)
          {
          sscanf(this->NameRegex.match(3).c_str(), ".%u.%u", &major, &minor);
          }
        if(this->BestPath.empty() || prefix < this->BestPrefix ||
           (prefix == this->BestPrefix && suffix < this->BestSuffix) ||
           (prefix == this->BestPrefix && suffix == this->BestSuffix &&
            (major > this->BestMajor ||
             (major == this->BestMajor && minor > this->BestMinor))))
          {
          this->BestPath = this->TestPath;
          this->BestPrefix = prefix;
          this->BestSuffix = suffix;
          this->BestMajor = major;
          this->BestMinor = minor;
          }
        }
      }
    }

  // Use the best candidate found in this directory, if any.
  return !this->BestPath.empty();
}

//----------------------------------------------------------------------------
std::string cmFindLibraryCommand::FindNormalLibrary()
{
  // Search the entire path for each name.
  cmFindLibraryHelper helper(this->Makefile);
  for(std::vector<std::string>::const_iterator ni = this->Names.begin();
      ni != this->Names.end() ; ++ni)
    {
    // Switch to searching for this name.
    std::string const& name = *ni;
    helper.SetName(name);

    // Search every directory.
    for(std::vector<std::string>::const_iterator
          p = this->SearchPaths.begin();
        p != this->SearchPaths.end(); ++p)
      {
      if(helper.CheckDirectory(*p))
        {
        return helper.BestPath;
        }
      }
    }
  // Couldn't find the library.
  return "";
}

//----------------------------------------------------------------------------
std::string cmFindLibraryCommand::FindFrameworkLibrary()
{
  // Search for a framework of each name in the entire search path.
  for(std::vector<std::string>::const_iterator ni = this->Names.begin();
      ni != this->Names.end() ; ++ni)
    {
    // Search the paths for a framework with this name.
    std::string fwName = *ni;
    fwName += ".framework";
    std::string fwPath = cmSystemTools::FindDirectory(fwName.c_str(),
                                                      this->SearchPaths,
                                                      true);
    if(!fwPath.empty())
      {
      return fwPath;
      }
    }

  // No framework found.
  return "";
}
