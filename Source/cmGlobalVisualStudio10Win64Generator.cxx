/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmGlobalVisualStudio10Win64Generator.h"
#include "cmMakefile.h"
#include "cmake.h"

//----------------------------------------------------------------------------
cmGlobalVisualStudio10Win64Generator::cmGlobalVisualStudio10Win64Generator()
{
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio10Win64Generator
::GetDocumentation(cmDocumentationEntry& entry) const
{
  entry.Name = this->GetName();
  entry.Brief = "Generates Visual Studio 10 Win64 project files.";
  entry.Full = "";
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio10Win64Generator
::AddPlatformDefinitions(cmMakefile* mf)
{
  this->cmGlobalVisualStudio10Generator::AddPlatformDefinitions(mf);
  mf->AddDefinition("CMAKE_FORCE_WIN64", "TRUE");
  mf->AddDefinition("MSVC_C_ARCHITECTURE_ID", "x64");
  mf->AddDefinition("MSVC_CXX_ARCHITECTURE_ID", "x64");
}

//----------------------------------------------------------------------------
bool cmGlobalVisualStudio10Win64Generator::Find64BitTools(cmMakefile* mf)
{
  if(!this->PlatformToolset.empty())
    {
    return true;
    }
  // This edition does not come with 64-bit tools.  Look for them.
  //
  // TODO: Detect available tools?  x64\v100 exists but does not work?
  // KHLM\\SOFTWARE\\Microsoft\\MSBuild\\ToolsVersions\\4.0;VCTargetsPath
  // c:/Program Files (x86)/MSBuild/Microsoft.Cpp/v4.0/Platforms/
  //   {Itanium,Win32,x64}/PlatformToolsets/{v100,v90,Windows7.1SDK}
  std::string winSDK_7_1;
  if(cmSystemTools::ReadRegistryValue(
       "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Microsoft SDKs\\"
       "Windows\\v7.1;InstallationFolder", winSDK_7_1))
    {
    cmOStringStream m;
    m << "Found Windows SDK v7.1: " << winSDK_7_1;
    mf->DisplayStatus(m.str().c_str(), -1);
    this->PlatformToolset = "Windows7.1SDK";
    return true;
    }
  else
    {
    cmOStringStream e;
    e << "Cannot enable 64-bit tools with Visual Studio 2010 Express.\n"
      << "Install the Microsoft Windows SDK v7.1 to get 64-bit tools:\n"
      << "  http://msdn.microsoft.com/en-us/windows/bb980924.aspx";
    mf->IssueMessage(cmake::FATAL_ERROR, e.str().c_str());
    cmSystemTools::SetFatalErrorOccured();
    return false;
    }
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio10Win64Generator
::EnableLanguage(std::vector<std::string> const& languages,
                 cmMakefile* mf, bool optional)
{
  if(this->IsExpressEdition() && !this->Find64BitTools(mf))
    {
    return;
    }
  this->cmGlobalVisualStudio10Generator
    ::EnableLanguage(languages, mf, optional);
}
