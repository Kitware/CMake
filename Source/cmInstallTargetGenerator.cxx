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
#include "cmInstallTargetGenerator.h"

#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmTarget.h"

//----------------------------------------------------------------------------
cmInstallTargetGenerator
::cmInstallTargetGenerator(cmTarget& t, const char* dest, bool implib):
  Target(&t), Destination(dest), ImportLibrary(implib)
{
  this->Target->SetHaveInstallRule(true);
}

//----------------------------------------------------------------------------
cmInstallTargetGenerator
::~cmInstallTargetGenerator()
{
}

//----------------------------------------------------------------------------
void cmInstallTargetGenerator::GenerateScript(std::ostream& os)
{
  // Compute the build tree directory from which to copy the target.
  std::string fromDir;
  if(this->Target->NeedRelinkBeforeInstall())
    {
    fromDir = this->Target->GetMakefile()->GetStartOutputDirectory();
    fromDir += "/CMakeFiles/CMakeRelink.dir/";
    }
  else
    {
    fromDir = this->Target->GetDirectory();
    fromDir += "/";
    }

  // Write variable settings to do per-configuration references.
  this->PrepareInstallReference(os);

  // Create the per-configuration reference.
  std::string fromName = this->GetInstallReference();
  std::string fromFile = fromDir;
  fromFile += fromName;

  // Setup special properties for some target types.
  std::string props;
  const char* properties = 0;
  cmTarget::TargetType type = this->Target->GetType();
  switch(type)
    {
    case cmTarget::SHARED_LIBRARY:
      {
      // Add shared library installation properties if this platform
      // supports them.
      const char* lib_version = this->Target->GetProperty("VERSION");
      const char* lib_soversion = this->Target->GetProperty("SOVERSION");
      if(!this->Target->GetMakefile()
         ->GetDefinition("CMAKE_SHARED_LIBRARY_SONAME_C_FLAG"))
        {
        // Versioning is supported only for shared libraries and modules,
        // and then only when the platform supports an soname flag.
        lib_version = 0;
        lib_soversion = 0;
        }
      if(lib_version)
        {
        props += " VERSION ";
        props += lib_version;
        }
      if(lib_soversion)
        {
        props += " SOVERSION ";
        props += lib_soversion;
        }
      properties = props.c_str();
      }
      break;
    case cmTarget::EXECUTABLE:
      {
      // Add executable installation properties if this platform
      // supports them.
#if defined(_WIN32) && !defined(__CYGWIN__)
      const char* exe_version = 0;
#else
      const char* exe_version = this->Target->GetProperty("VERSION");
#endif
      if(exe_version)
        {
        props += " VERSION ";
        props += exe_version;
        properties = props.c_str();
        }

      // Handle OSX Bundles.
      if(this->Target->GetPropertyAsBool("MACOSX_BUNDLE"))
        {
        // Compute the source locations of the bundle executable and
        // Info.plist file.
        std::string plist = fromFile;
        plist += ".app/Contents/Info.plist";
        fromFile += ".app/Contents/MacOS/";
        fromFile += fromName;

        // Compute the destination locations of the bundle executable
        // and Info.plist file.
        std::string bdest = this->Destination;
        bdest += "/";
        bdest += fromName;
        std::string pdest = bdest;
        pdest += ".app/Contents";
        bdest += ".app/Contents/MacOS";

        // Install the Info.plist file.
        this->AddInstallRule(os, pdest.c_str(), cmTarget::INSTALL_FILES,
                             plist.c_str());
        }
      }
      break;
    case cmTarget::STATIC_LIBRARY:
    case cmTarget::MODULE_LIBRARY:
      // Nothing special for modules or static libraries.
      break;
    default:
      break;
    }

  // An import library looks like a static library.
  if(this->ImportLibrary)
    {
    type = cmTarget::STATIC_LIBRARY;
    }

  // Write code to install the target file.
  this->AddInstallRule(os, this->Destination.c_str(), type, fromFile.c_str(),
                       this->ImportLibrary, properties);
}

//----------------------------------------------------------------------------
void
cmInstallTargetGenerator
::PrepareInstallReference(std::ostream& os)
{
  // If the target name may vary with the configuration type then
  // store all possible names ahead of time in variables.
  std::string fname;
  for(std::vector<std::string>::const_iterator i =
        this->ConfigurationTypes->begin();
      i != this->ConfigurationTypes->end(); ++i)
    {
    // Start with the configuration's subdirectory.
    fname = "";
    this->Target->GetMakefile()->GetLocalGenerator()->GetGlobalGenerator()->
      AppendDirectoryForConfig(i->c_str(), fname);

    // Set a variable with the target name for this configuration.
    fname += this->Target->GetFullName(i->c_str(), this->ImportLibrary);
    os << "SET(" << this->Target->GetName()
       << (this->ImportLibrary? "_IMPNAME_" : "_NAME_") << *i
       << " \"" << fname << "\")\n";
    }
}

//----------------------------------------------------------------------------
std::string cmInstallTargetGenerator::GetInstallReference()
{
  if(this->ConfigurationTypes->empty())
    {
    // Reference the target by its one configuration name.
    return this->Target->GetFullName(this->ConfigurationName,
                                     this->ImportLibrary);
    }
  else
    {
    // Reference the target using the per-configuration variable.
    std::string ref = "${";
    ref += this->Target->GetName();
    if(this->ImportLibrary)
      {
      ref += "_IMPNAME_";
      }
    else
      {
      ref += "_NAME_";
      }
    ref += "${CMAKE_INSTALL_CONFIG_NAME}}";
    return ref;
    }
}
