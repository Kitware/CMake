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
  // Compute the final location of the file if necessary.
  if(this->FullPath.empty())
    {
    this->GetFullPath();
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
std::string const& cmSourceFile::GetFullPath()
{
  if(this->FullPath.empty())
    {
    if(this->FindFullPath())
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
bool cmSourceFile::FindFullPath()
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
  e << "Cannot find source file \"" << this->Location.GetName() << "\"";
  e << "\n\nTried extensions";
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
  cmSystemTools::Error(e.str().c_str());
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

  // Look for header files.
  cmMakefile* mf = this->Location.GetMakefile();
  const std::vector<std::string>& hdrExts = mf->GetHeaderExtensions();
  if(std::find(hdrExts.begin(), hdrExts.end(), this->Extension) ==
     hdrExts.end())
    {
    // This is not a known header file extension.  Mark it as not a
    // header unless the user has already explicitly set the property.
    if(!this->GetProperty("HEADER_FILE_ONLY"))
      {
      this->SetProperty("HEADER_FILE_ONLY", "0");
      }
    }
  else
    {
    // This is a known header file extension.  The source cannot be compiled.
    this->SetProperty("HEADER_FILE_ONLY", "1");
    }

  // Try to identify the source file language from the extension.
  cmGlobalGenerator* gg = mf->GetLocalGenerator()->GetGlobalGenerator();
  if(const char* l = gg->GetLanguageFromExtension(this->Extension.c_str()))
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
  if (!value)
    {
    value = "NOTFOUND";
    }

  this->Properties.SetProperty(prop, value, cmProperty::SOURCE_FILE);
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

//----------------------------------------------------------------------------
void cmSourceFile::DefineProperties(cmake *cm)
{
  // define properties
  cm->DefineProperty
    ("ABSTRACT", cmProperty::SOURCE_FILE, 
     "Is this source file an abstract class.",
     "A property ona source file that indicates if the source file "
     "represents a class that is abstract. This only makes sense for "
     "languages that have a notion of an abstract class and it is "
     "only used by somw tools that wrap classes into other languages.");

  cm->DefineProperty
    ("COMPILE_FLAGS", cmProperty::SOURCE_FILE, 
     "Additional flags to be added when compiling this source file.",
     "These flags will be added to the list of compile flags when "
     "this source file.");

  cm->DefineProperty
    ("EXTERNAL_OBJECT", cmProperty::SOURCE_FILE, 
     "If set to true then this is an object file.",
     "If this property is set to true then the source file "
     "is really an object file and should not be compiled.  "
     "It will still be linked into the target though.");

  cm->DefineProperty
    ("EXTRA_CONTENT", cmProperty::SOURCE_FILE, 
     "Is this file part of a target's extra content.",
     "If this property is set, the source file will be added to the "
     "target's list of extra content. This is used by makefile "
     "generators for some sort of Mac budle framework support.");

  cm->DefineProperty
    ("GENERATED", cmProperty::SOURCE_FILE, 
     "Is this source file generated as part of the build process.",
     "If a source file is generated by the build process CMake will "
     "handle it differently in temrs of dependency checking etc. "
     "Otherwise having a non-existent source file could create problems.");

  cm->DefineProperty
    ("HEADER_FILE_ONLY", cmProperty::SOURCE_FILE, 
     "Is this source file only a header file.",
     "A property ona source file that indicates if the source file "
     "is a header file with no associated implementation. This is "
     "set automatically based on the file extension and is used by "
     "CMake to determine is certain dependency information should be "
     "computed.");

  cm->DefineProperty
    ("KEEP_EXTENSION", cmProperty::SOURCE_FILE, 
     "Make th eoutput file have the same extension as the source file.",
     "If this property is set then the file extension of the output "
     "file will be the same as that of the source file. Normally "
     "the output file extension is computed based on the language "
     "of the source file, for example .cxx will go to a .o extension.");

  cm->DefineProperty
    ("LANGUAGE", cmProperty::SOURCE_FILE, 
     "What programming language is the file.",
     "A property that can be set to indicate what programming language "
     "the source file is. If it is not set the language is determined "
     "based on the file extension. Typical values are CXX C etc.");

  cm->DefineProperty
    ("LOCATION", cmProperty::SOURCE_FILE, 
     "The full path to a source file.",
     "A read only property on a SOURCE FILE that contains the full path "
     "to the source file.");

  cm->DefineProperty
    ("MACOSX_PACKAGE_LOCATION", cmProperty::SOURCE_FILE, 
     "Location for MACOSX bundles and frameworks.",
     "MACOSX_PACKAGE_LOCATION is the property of a file within a mac osx "
     "bundle or framework that specifies where this file should be "
     "copied. This makes sense for things like icons and other "
     "resources.");

  cm->DefineProperty
    ("MACOSX_CONTENT", cmProperty::SOURCE_FILE, 
     "If true then this is part of a MACOSX bundle or framework.",
     "MACOSX_CONTENT is a flag that if true this file will be copied "
     "to the bundle or framework.");

  cm->DefineProperty
    ("OBJECT_DEPENDS", cmProperty::SOURCE_FILE, 
     "Additional dependencies.",
     "Additional dependencies that should be checked as part of "
     "building this source file.");

  cm->DefineProperty
    ("OBJECT_OUTPUTS", cmProperty::SOURCE_FILE, 
     "Additional outputs for a Makefile rule.",
     "Additional outputs created by compilation of this source file. "
     "If any of these outputs is missing the object will be recompiled. "
     "This is supported only on Makefile generators and will be ignored "
     "on other generators.");

  cm->DefineProperty
    ("SYMBOLIC", cmProperty::SOURCE_FILE, 
     "Is this just a name for a rule.",
     "If SYMBOLIC (boolean) is set to true the build system will be "
     "informed that the source file is not actually created on disk but "
     "instead used as a symbolic name for a build rule.");
  
  cm->DefineProperty
    ("WRAP_EXCLUDE", cmProperty::SOURCE_FILE, 
     "Exclude this source file from any code wrapping techniques.",
     "Some packages can wrap source files into alternate languages "
     "to provide additional functionality. For example, C++ code "
     "can be wrapped into Java or Python etc using SWIG etc. "
     "If WRAP_EXCLUDE is set to true (1 etc) that indicates then "
     "this source file should not be wrapped.");
}

