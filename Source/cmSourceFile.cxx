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
#include "cmSystemTools.h"

#include "cmake.h"

// Set the name of the class and the full path to the file.
// The class must be found in dir and end in name.cxx, name.txx, 
// name.c or it will be considered a header file only class
// and not included in the build process
bool cmSourceFile::SetName(const char* name, const char* dir,
                           const std::vector<std::string>& sourceExts,
                           const std::vector<std::string>& headerExts,
                           const char* target)
{

  this->SetProperty("HEADER_FILE_ONLY","1");
  this->SourceNameWithoutLastExtension = "";

  // Save the original name given.
  this->SourceName = name;

  // Convert the name to a full path in case the given name is a
  // relative path.
  std::string pathname = cmSystemTools::CollapseFullPath(name, dir);

  // First try and see whether the listed file can be found
  // as is without extensions added on.
  std::string hname = pathname;
  if(cmSystemTools::FileExists(hname.c_str()))
    {
    this->SourceName = cmSystemTools::GetFilenamePath(name);
    if ( this->SourceName.size() > 0 )
      {
      this->SourceName += "/";
      }
    this->SourceName += cmSystemTools::GetFilenameWithoutLastExtension(name);
    std::string::size_type pos = hname.rfind('.');
    if(pos != std::string::npos)
      {
      this->SourceExtension = hname.substr(pos+1, hname.size()-pos);
      if ( cmSystemTools::FileIsFullPath(name) )
        {
        std::string::size_type pos2 = hname.rfind('/');
        if(pos2 != std::string::npos)
          {
          this->SourceName = hname.substr(pos2+1, pos - pos2-1);
          }
        }
      }

    // See if the file is a header file
    if(std::find( headerExts.begin(), headerExts.end(), 
                  this->SourceExtension ) == headerExts.end())
      {
      this->SetProperty("HEADER_FILE_ONLY","0");
      }
    this->FullPath = hname;

    // Mark this as an external object file if it has the proper
    // extension.  THIS CODE IS DUPLICATED IN THE OTHER SetName METHOD.
    // THESE METHODS SHOULD BE MERGED.
    if ( this->SourceExtension == "obj" || this->SourceExtension == "o" ||
      this->SourceExtension == "lo" )
      {
      this->SetProperty("EXTERNAL_OBJECT", "1");
      }
    return true;
    }
  
  // Next, try the various source extensions
  for( std::vector<std::string>::const_iterator ext = sourceExts.begin();
       ext != sourceExts.end(); ++ext )
    {
    hname = pathname;
    hname += ".";
    hname += *ext;
    if(cmSystemTools::FileExists(hname.c_str()))
      {
      this->SourceExtension = *ext;
      this->SetProperty("HEADER_FILE_ONLY","0");
      this->FullPath = hname;
      return true;
      }
    }

  // Finally, try the various header extensions
  for( std::vector<std::string>::const_iterator ext = headerExts.begin();
       ext != headerExts.end(); ++ext )
    {
    hname = pathname;
    hname += ".";
    hname += *ext;
    if(cmSystemTools::FileExists(hname.c_str()))
      {
      this->SourceExtension = *ext;
      this->FullPath = hname;
      return true;
      }
    }

  cmOStringStream e;
  e << "Cannot find source file \"" << pathname << "\"";
  if(target)
    {
    e << " for target \"" << target << "\"";
    }
  e << "\n\nTried extensions";
  for( std::vector<std::string>::const_iterator ext = sourceExts.begin();
       ext != sourceExts.end(); ++ext )
    {
    e << " ." << *ext;
    }
  for( std::vector<std::string>::const_iterator ext = headerExts.begin();
       ext != headerExts.end(); ++ext )
    {
    e << " ." << *ext;
    }
  cmSystemTools::Error(e.str().c_str());
  return false;
}

void cmSourceFile::SetName(const char* name, const char* dir, const char *ext,
                           bool hfo)
{
  this->SetProperty("HEADER_FILE_ONLY",(hfo ? "1" : "0"));
  this->SourceNameWithoutLastExtension = "";
  this->SourceName = name;
  std::string fname = this->SourceName;
  if(ext && strlen(ext))
    {
    fname += ".";
    fname += ext;
    }
  this->FullPath = cmSystemTools::CollapseFullPath(fname.c_str(), dir);
  cmSystemTools::ConvertToUnixSlashes(this->FullPath);
  this->SourceExtension = ext;

  // Mark this as an external object file if it has the proper
  // extension.  THIS CODE IS DUPLICATED IN THE OTHER SetName METHOD.
  // THESE METHODS SHOULD BE MERGED.
  if ( this->SourceExtension == "obj" || this->SourceExtension == "o" ||
       this->SourceExtension == "lo" )
    {
    this->SetProperty("EXTERNAL_OBJECT", "1");
    }
  return;
}

void cmSourceFile::Print() const
{
  std::cerr << "this->FullPath: " <<  this->FullPath << "\n";
  std::cerr << "this->SourceName: " << this->SourceName << std::endl;
  std::cerr << "this->SourceExtension: " << this->SourceExtension << "\n";
}

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

const char *cmSourceFile::GetProperty(const char* prop) const
{
  // watch for special "computed" properties that are dependent on other
  // properties or variables, always recompute them
  if (!strcmp(prop,"LOCATION"))
    {
    return this->FullPath.c_str();
    }

  bool chain = false;
  return this->Properties.GetPropertyValue(prop,cmProperty::SOURCE_FILE, 
                                           chain);
}

bool cmSourceFile::GetPropertyAsBool(const char* prop) const
{
  return cmSystemTools::IsOn(this->GetProperty(prop));
}

void cmSourceFile::SetCustomCommand(cmCustomCommand* cc)
{
  if(this->CustomCommand)
    {
    delete this->CustomCommand;
    }
  this->CustomCommand = cc;
}

const std::string& cmSourceFile::GetSourceNameWithoutLastExtension()
{
  if ( this->SourceNameWithoutLastExtension.empty() )
    {
    this->SourceNameWithoutLastExtension = 
      cmSystemTools::GetFilenameWithoutLastExtension(this->FullPath);
    }
  return this->SourceNameWithoutLastExtension;
}

cmSourceFile::cmSourceFile()
{
  this->CustomCommand = 0; 
}

// define properties
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

