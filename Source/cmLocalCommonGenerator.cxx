/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2015 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmLocalCommonGenerator.h"

#include "cmMakefile.h"

cmLocalCommonGenerator::cmLocalCommonGenerator(
  cmGlobalGenerator* gg, cmMakefile* mf, cmOutputConverter::RelativeRoot wd)
  : cmLocalGenerator(gg, mf)
  , WorkingDirectory(wd)
{
}

cmLocalCommonGenerator::~cmLocalCommonGenerator()
{
}

void cmLocalCommonGenerator::SetConfigName()
{
  // Store the configuration name that will be generated.
  if (const char* config = this->Makefile->GetDefinition("CMAKE_BUILD_TYPE")) {
    // Use the build type given by the user.
    this->ConfigName = config;
  } else {
    // No configuration type given.
    this->ConfigName = "";
  }
}

std::string cmLocalCommonGenerator::GetTargetFortranFlags(
  cmGeneratorTarget const* target, std::string const& config)
{
  std::string flags;

  // Enable module output if necessary.
  if (const char* modout_flag =
        this->Makefile->GetDefinition("CMAKE_Fortran_MODOUT_FLAG")) {
    this->AppendFlags(flags, modout_flag);
  }

  // Add a module output directory flag if necessary.
  std::string mod_dir = target->GetFortranModuleDirectory();
  if (!mod_dir.empty()) {
    mod_dir =
      this->Convert(mod_dir, this->WorkingDirectory, cmOutputConverter::SHELL);
  } else {
    mod_dir =
      this->Makefile->GetSafeDefinition("CMAKE_Fortran_MODDIR_DEFAULT");
  }
  if (!mod_dir.empty()) {
    const char* moddir_flag =
      this->Makefile->GetRequiredDefinition("CMAKE_Fortran_MODDIR_FLAG");
    std::string modflag = moddir_flag;
    modflag += mod_dir;
    this->AppendFlags(flags, modflag);
  }

  // If there is a separate module path flag then duplicate the
  // include path with it.  This compiler does not search the include
  // path for modules.
  if (const char* modpath_flag =
        this->Makefile->GetDefinition("CMAKE_Fortran_MODPATH_FLAG")) {
    std::vector<std::string> includes;
    this->GetIncludeDirectories(includes, target, "C", config);
    for (std::vector<std::string>::const_iterator idi = includes.begin();
         idi != includes.end(); ++idi) {
      std::string flg = modpath_flag;
      flg +=
        this->Convert(*idi, cmOutputConverter::NONE, cmOutputConverter::SHELL);
      this->AppendFlags(flags, flg);
    }
  }

  return flags;
}
