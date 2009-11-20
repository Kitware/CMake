/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "windows.h" // this must be first to define GetCurrentDirectory
#include "cmGlobalVisualStudio9Generator.h"
#include "cmLocalVisualStudio7Generator.h"
#include "cmMakefile.h"
#include "cmake.h"



cmGlobalVisualStudio9Generator::cmGlobalVisualStudio9Generator()
{
  this->FindMakeProgramFile = "CMakeVS9FindMake.cmake";
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio9Generator::AddPlatformDefinitions(cmMakefile* mf)
{
  mf->AddDefinition("MSVC_C_ARCHITECTURE_ID", "X86");
  mf->AddDefinition("MSVC_CXX_ARCHITECTURE_ID", "X86");
  mf->AddDefinition("MSVC90", "1");
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio9Generator::WriteSLNHeader(std::ostream& fout)
{
  fout << "Microsoft Visual Studio Solution File, Format Version 10.00\n";
  fout << "# Visual Studio 2008\n";
}

///! Create a local generator appropriate to this Global Generator
cmLocalGenerator *cmGlobalVisualStudio9Generator::CreateLocalGenerator()
{
  cmLocalVisualStudio7Generator *lg = new cmLocalVisualStudio7Generator;
  lg->SetVersion9();
  lg->SetExtraFlagTable(this->GetExtraFlagTableVS8());
  lg->SetGlobalGenerator(this);
  return lg;
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio9Generator
::GetDocumentation(cmDocumentationEntry& entry) const
{
  entry.Name = this->GetName();
  entry.Brief = "Generates Visual Studio 9 2008 project files.";
  entry.Full = "";
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudio9Generator
::EnableLanguage(std::vector<std::string>const &  lang, 
                 cmMakefile *mf, bool optional)
{
  cmGlobalVisualStudio8Generator::EnableLanguage(lang, mf, optional);
}

//----------------------------------------------------------------------------
std::string cmGlobalVisualStudio9Generator::GetUserMacrosDirectory()
{
  std::string base;
  std::string path;

  // base begins with the VisualStudioProjectsLocation reg value...
  if (cmSystemTools::ReadRegistryValue(
    "HKEY_CURRENT_USER\\Software\\Microsoft\\VisualStudio\\9.0;"
    "VisualStudioProjectsLocation",
    base))
    {
    cmSystemTools::ConvertToUnixSlashes(base);

    // 9.0 macros folder:
    path = base + "/VSMacros80";
      // *NOT* a typo; right now in Visual Studio 2008 beta the macros
      // folder is VSMacros80... They may change it to 90 before final
      // release of 2008 or they may not... we'll have to keep our eyes
      // on it
    }

  // path is (correctly) still empty if we did not read the base value from
  // the Registry value
  return path;
}

//----------------------------------------------------------------------------
std::string cmGlobalVisualStudio9Generator::GetUserMacrosRegKeyBase()
{
  return "Software\\Microsoft\\VisualStudio\\9.0\\vsmacros";
}
