/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmSourceFile.h"

#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmSystemTools.h"
#include "cmake.h"

//----------------------------------------------------------------------------
cmSourceFile::cmSourceFile(cmMakefile* mf, const char* name):
  Location(mf, name)
{
  this->CustomCommand = 0;
  this->Properties.SetCMakeInstance(mf->GetCMakeInstance());
  this->FindFullPathFailed = false;
}

//----------------------------------------------------------------------------
cmSourceFile::~cmSourceFile()
{
  this->SetCustomCommand(0);
}

//----------------------------------------------------------------------------
std::string const& cmSourceFile::GetExtension() const
{
  return this->Extension;
}

//----------------------------------------------------------------------------
const char* cmSourceFile::GetLanguage()
{
  // If the language was set explicitly by the user then use it.
  if(const char* lang = this->GetProperty("LANGUAGE"))
    {
    return lang;
    }

  // Perform computation needed to get the language if necessary.
  if(this->FullPath.empty() && this->Language.empty())
    {
    // If a known extension is given or a known full path is given
    // then trust that the current extension is sufficient to
    // determine the language.  This will fail only if the user
    // specifies a full path to the source but leaves off the
    // extension, which is kind of weird.
    if(this->Location.ExtensionIsAmbiguous() &&
       this->Location.DirectoryIsAmbiguous())
      {
      // Finalize the file location to get the extension and set the
      // language.
      this->GetFullPath();
      }
    else
      {
      // Use the known extension to get the language if possible.
      std::string ext =
        cmSystemTools::GetFilenameLastExtension(this->Location.GetName());
      this->CheckLanguage(ext);
      }
    }

  // Now try to determine the language.
  return static_cast<cmSourceFile const*>(this)->GetLanguage();
}

//----------------------------------------------------------------------------
const char* cmSourceFile::GetLanguage() const
{
  // If the language was set explicitly by the user then use it.
  if(const char* lang = this->GetProperty("LANGUAGE"))
    {
    return lang;
    }

  // If the language was determined from the source file extension use it.
  if(!this->Language.empty())
    {
    return this->Language.c_str();
    }

  // The language is not known.
  return 0;
}

//----------------------------------------------------------------------------
cmSourceFileLocation const& cmSourceFile::GetLocation() const
{
    return this->Location;
}

//----------------------------------------------------------------------------
std::string const& cmSourceFile::GetFullPath(std::string* error)
{
  if(this->FullPath.empty())
    {
    if(this->FindFullPath(error))
      {
      this->CheckExtension();
      }
    }
  return this->FullPath;
}

//----------------------------------------------------------------------------
std::string const& cmSourceFile::GetFullPath() const
{
  return this->FullPath;
}

//----------------------------------------------------------------------------
bool cmSourceFile::FindFullPath(std::string* error)
{
  // If thie method has already failed once do not try again.
  if(this->FindFullPathFailed)
    {
    return false;
    }

  // If the file is generated compute the location without checking on
  // disk.
  if(this->GetPropertyAsBool("GENERATED"))
    {
    // The file is either already a full path or is relative to the
    // build directory for the target.
    this->Location.DirectoryUseBinary();
    this->FullPath = this->Location.GetDirectory();
    this->FullPath += "/";
    this->FullPath += this->Location.GetName();
    return true;
    }

  // The file is not generated.  It must exist on disk.
  cmMakefile* mf = this->Location.GetMakefile();
  const char* tryDirs[3] = {0, 0, 0};
  if(this->Location.DirectoryIsAmbiguous())
    {
    tryDirs[0] = mf->GetCurrentDirectory();
    tryDirs[1] = mf->GetCurrentOutputDirectory();
    }
  else
    {
    tryDirs[0] = "";
    }
  const std::vector<std::string>& srcExts = mf->GetSourceExtensions();
  const std::vector<std::string>& hdrExts = mf->GetHeaderExtensions();
  for(const char* const* di = tryDirs; *di; ++di)
    {
    std::string tryPath = this->Location.GetDirectory();
    if(!tryPath.empty())
      {
      tryPath += "/";
      }
    tryPath += this->Location.GetName();
    tryPath = cmSystemTools::CollapseFullPath(tryPath.c_str(), *di);
    if(this->TryFullPath(tryPath.c_str(), 0))
      {
      return true;
      }
    for(std::vector<std::string>::const_iterator ei = srcExts.begin();
        ei != srcExts.end(); ++ei)
      {
      if(this->TryFullPath(tryPath.c_str(), ei->c_str()))
        {
        return true;
        }
      }
    for(std::vector<std::string>::const_iterator ei = hdrExts.begin();
        ei != hdrExts.end(); ++ei)
      {
      if(this->TryFullPath(tryPath.c_str(), ei->c_str()))
        {
        return true;
        }
      }
    }

  cmOStringStream e;
  std::string missing = this->Location.GetDirectory();
  if(!missing.empty())
    {
    missing += "/";
    }
  missing += this->Location.GetName();
  e << "Cannot find source file:\n  " << missing << "\nTried extensions";
  for(std::vector<std::string>::const_iterator ext = srcExts.begin();
      ext != srcExts.end(); ++ext)
    {
    e << " ." << *ext;
    }
  for(std::vector<std::string>::const_iterator ext = hdrExts.begin();
      ext != hdrExts.end(); ++ext)
    {
    e << " ." << *ext;
    }
  if(error)
    {
    *error = e.str();
    }
  else
    {
    this->Location.GetMakefile()->IssueMessage(cmake::FATAL_ERROR, e.str());
    }
  this->FindFullPathFailed = true;
  return false;
}

//----------------------------------------------------------------------------
bool cmSourceFile::TryFullPath(const char* tp, const char* ext)
{
  std::string tryPath = tp;
  if(ext && *ext)
    {
    tryPath += ".";
    tryPath += ext;
    }
  if(cmSystemTools::FileExists(tryPath.c_str()))
    {
    this->FullPath = tryPath;
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
void cmSourceFile::CheckExtension()
{
  // Compute the extension.
  std::string realExt =
    cmSystemTools::GetFilenameLastExtension(this->FullPath);
  if(!realExt.empty())
    {
    // Store the extension without the leading '.'.
    this->Extension = realExt.substr(1);
    }

  // Look for object files.
  if(this->Extension == "obj" ||
     this->Extension == "o" ||
     this->Extension == "lo")
    {
    this->SetProperty("EXTERNAL_OBJECT", "1");
    }

  // Try to identify the source file language from the extension.
  if(this->Language.empty())
    {
    this->CheckLanguage(this->Extension);
    }
}

//----------------------------------------------------------------------------
void cmSourceFile::CheckLanguage(std::string const& ext)
{
  // Try to identify the source file language from the extension.
  cmMakefile* mf = this->Location.GetMakefile();
  cmGlobalGenerator* gg = mf->GetLocalGenerator()->GetGlobalGenerator();
  if(const char* l = gg->GetLanguageFromExtension(ext.c_str()))
    {
    this->Language = l;
    }
}

//----------------------------------------------------------------------------
bool cmSourceFile::Matches(cmSourceFileLocation const& loc)
{
  return this->Location.Matches(loc);
}

//----------------------------------------------------------------------------
void cmSourceFile::SetProperty(const char* prop, const char* value)
{
  if (!prop)
    {
    return;
    }

  this->Properties.SetProperty(prop, value, cmProperty::SOURCE_FILE);
}

//----------------------------------------------------------------------------
void cmSourceFile::AppendProperty(const char* prop, const char* value,
                                  bool asString)
{
  if (!prop)
    {
    return;
    }
  this->Properties.AppendProperty(prop, value, cmProperty::SOURCE_FILE,
                                  asString);
}

//----------------------------------------------------------------------------
const char* cmSourceFile::GetPropertyForUser(const char *prop)
{
  // This method is a consequence of design history and backwards
  // compatibility.  GetProperty is (and should be) a const method.
  // Computed properties should not be stored back in the property map
  // but instead reference information already known.  If they need to
  // cache information in a mutable ivar to provide the return string
  // safely then so be it.
  //
  // The LOCATION property is particularly problematic.  The CMake
  // language has very loose restrictions on the names that will match
  // a given source file (for historical reasons).  Implementing
  // lookups correctly with such loose naming requires the
  // cmSourceFileLocation class to commit to a particular full path to
  // the source file as late as possible.  If the users requests the
  // LOCATION property we must commit now.
  if(strcmp(prop, "LOCATION") == 0)
    {
    // Commit to a location.
    this->GetFullPath();
    }

  // Perform the normal property lookup.
  return this->GetProperty(prop);
}

//----------------------------------------------------------------------------
const char* cmSourceFile::GetProperty(const char* prop) const
{
  // Check for computed properties.
  if(strcmp(prop, "LOCATION") == 0)
    {
    if(this->FullPath.empty())
      {
      return 0;
      }
    else
      {
      return this->FullPath.c_str();
      }
    }

  bool chain = false;
  const char *retVal =
    this->Properties.GetPropertyValue(prop, cmProperty::SOURCE_FILE, chain);
  if (chain)
    {
    cmMakefile* mf = this->Location.GetMakefile();
    return mf->GetProperty(prop,cmProperty::SOURCE_FILE);
    }

  return retVal;
}

//----------------------------------------------------------------------------
bool cmSourceFile::GetPropertyAsBool(const char* prop) const
{
  return cmSystemTools::IsOn(this->GetProperty(prop));
}

//----------------------------------------------------------------------------
cmCustomCommand* cmSourceFile::GetCustomCommand()
{
  return this->CustomCommand;
}

//----------------------------------------------------------------------------
cmCustomCommand const* cmSourceFile::GetCustomCommand() const
{
  return this->CustomCommand;
}

//----------------------------------------------------------------------------
void cmSourceFile::SetCustomCommand(cmCustomCommand* cc)
{
  cmCustomCommand* old = this->CustomCommand;
  this->CustomCommand = cc;
  delete old;
}
