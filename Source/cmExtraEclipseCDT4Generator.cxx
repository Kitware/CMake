/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  Copyright (c) 2004 Alexander Neundorf neundorf@kde.org, All rights reserved.
  Copyright (c) 2007 Miguel A. Figueroa-Villanueva. All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "cmExtraEclipseCDT4Generator.h"
#include "cmGlobalUnixMakefileGenerator3.h"
#include "cmLocalUnixMakefileGenerator3.h"
#include "cmMakefile.h"
#include "cmGeneratedFileStream.h"
#include "cmTarget.h"

#include "cmSystemTools.h"

//----------------------------------------------------------------------------
cmExtraEclipseCDT4Generator
::cmExtraEclipseCDT4Generator() : cmExternalMakefileProjectGenerator()
{
#if defined(_WIN32)
  this->SupportedGlobalGenerators.push_back("NMake Makefiles");
//  this->SupportedGlobalGenerators.push_back("MSYS Makefiles");
//  this->SupportedGlobalGenerators.push_back("MinGW Makefiles");
#endif
  this->SupportedGlobalGenerators.push_back("Unix Makefiles");
}

//----------------------------------------------------------------------------
void cmExtraEclipseCDT4Generator
::GetDocumentation(cmDocumentationEntry& entry, const char*) const
{
  entry.name = this->GetName();
  entry.brief = "Generates Eclipse CDT 4.0 project files.";
  entry.full =
    "Project files for Eclipse will be created in the top directory "
    "and will have a linked resource to every subdirectory which "
    "features a CMakeLists.txt file containing a PROJECT() call."
    "Additionally a hierarchy of makefiles is generated into the "
    "build tree. The appropriate make program can build the project through "
    "the default make target. A \"make install\" target is also provided.";
}

//----------------------------------------------------------------------------
void cmExtraEclipseCDT4Generator
::SetGlobalGenerator(cmGlobalGenerator* generator)
{
  cmExternalMakefileProjectGenerator::SetGlobalGenerator(generator);
  cmGlobalUnixMakefileGenerator3* mf
    = static_cast<cmGlobalUnixMakefileGenerator3*>(generator);
  mf->SetToolSupportsColor(false);
  mf->SetForceVerboseMakefiles(true);
  mf->EnableInstallTarget();
}

//----------------------------------------------------------------------------
void cmExtraEclipseCDT4Generator::Generate()
{
  // create a .project file
  this->CreateProjectFile();

  // create a .cproject file
  this->CreateCProjectFile();
}

//----------------------------------------------------------------------------
void cmExtraEclipseCDT4Generator::CreateProjectFile() const
{
  const cmMakefile* mf
    = this->GlobalGenerator->GetLocalGenerators()[0]->GetMakefile();

  std::string filename = mf->GetStartOutputDirectory();
  filename = filename + "/" + ".project";

  cmGeneratedFileStream fout(filename.c_str());
  if (!fout)
    {
    return;
    }

  fout << 
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<projectDescription>\n"
    "\t<name>" << mf->GetProjectName() << "</name>\n"
    "\t<comment></comment>\n"
    "\t<projects>\n"
    "\t</projects>\n"
    "\t<buildSpec>\n"
    "\t\t<buildCommand>\n"
    "\t\t\t<name>org.eclipse.cdt.make.core.makeBuilder</name>\n"
    "\t\t\t<triggers>clean,full,incremental,</triggers>\n"
    "\t\t\t<arguments>\n"
    ;

  // use clean target...
  fout << 
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.cleanBuildTarget</key>\n"
    "\t\t\t\t\t<value>clean</value>\n"
    "\t\t\t\t</dictionary>\n"
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.enableCleanBuild</key>\n"
    "\t\t\t\t\t<value>true</value>\n"
    "\t\t\t\t</dictionary>\n"
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.append_environment</key>\n"
    "\t\t\t\t\t<value>true</value>\n"
    "\t\t\t\t</dictionary>\n"
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.stopOnError</key>\n"
    "\t\t\t\t\t<value>true</value>\n"
    "\t\t\t\t</dictionary>\n"
    ;

  // set the make command...
  std::string make = mf->GetRequiredDefinition("CMAKE_MAKE_PROGRAM");
  //fout << "\t\t\t\t<dictionary>\n"
  //        "\t\t\t\t\t<key>org.eclipse.cdt.make.core.buildCommand</key>\n"
  //        "\t\t\t\t\t<value>" + make + "</value>\n"
  //        "\t\t\t\t</dictionary>\n"
  //        ;
  fout << 
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.enabledIncrementalBuild</key>\n"
    "\t\t\t\t\t<value>true</value>\n"
    "\t\t\t\t</dictionary>\n"
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.build.command</key>\n"
    "\t\t\t\t\t<value>" + make + "</value>\n"
    "\t\t\t\t</dictionary>\n"
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.contents</key>\n"
    "\t\t\t\t\t<value>org.eclipse.cdt.make.core.activeConfigSettings</value>\n"
    "\t\t\t\t</dictionary>\n"
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.build.target.inc</key>\n"
    "\t\t\t\t\t<value>all</value>\n"
    "\t\t\t\t</dictionary>\n"
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.build.arguments</key>\n"
    "\t\t\t\t\t<value></value>\n"
    "\t\t\t\t</dictionary>\n"
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.buildLocation</key>\n"
    "\t\t\t\t\t<value>"
     << this->GetEclipsePath(mf->GetStartOutputDirectory()) << "</value>\n"
    "\t\t\t\t</dictionary>\n"
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.useDefaultBuildCmd</key>\n"
    "\t\t\t\t\t<value>false</value>\n"
    "\t\t\t\t</dictionary>\n"
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.environment</key>\n"
    "\t\t\t\t\t<value></value>\n"
    "\t\t\t\t</dictionary>\n"
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.enableFullBuild</key>\n"
    "\t\t\t\t\t<value>true</value>\n"
    "\t\t\t\t</dictionary>\n"
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.build.target.auto</key>\n"
    "\t\t\t\t\t<value>all</value>\n"
    "\t\t\t\t</dictionary>\n"
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.enableAutoBuild</key>\n"
    "\t\t\t\t\t<value>false</value>\n"
    "\t\t\t\t</dictionary>\n"
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.build.target.clean</key>\n"
    "\t\t\t\t\t<value>clean</value>\n"
    "\t\t\t\t</dictionary>\n"
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.fullBuildTarget</key>\n"
    "\t\t\t\t\t<value>all</value>\n"
    "\t\t\t\t</dictionary>\n"
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.buildArguments</key>\n"
    "\t\t\t\t\t<value></value>\n"
    "\t\t\t\t</dictionary>\n"
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.build.location</key>\n"
    "\t\t\t\t\t<value>"
    << this->GetEclipsePath(mf->GetStartOutputDirectory()) << "</value>\n"
    "\t\t\t\t</dictionary>\n"
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.autoBuildTarget</key>\n"
    "\t\t\t\t\t<value>all</value>\n"
    "\t\t\t\t</dictionary>\n"
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.core.errorOutputParser</key>\n"
    "\t\t\t\t\t<value>"
    "org.eclipse.cdt.core.MakeErrorParser;"
    "org.eclipse.cdt.core.GCCErrorParser;"
    "org.eclipse.cdt.core.GASErrorParser;"
    "org.eclipse.cdt.core.GLDErrorParser;"
    // *** "org.eclipse.cdt.core.VCErrorParser;"
    "</value>\n"
    "\t\t\t\t</dictionary>\n"
    "\t\t\t</arguments>\n"
    "\t\t</buildCommand>\n"
    "\t\t<buildCommand>\n"
    "\t\t\t<name>org.eclipse.cdt.make.core.ScannerConfigBuilder</name>\n"
    "\t\t\t<arguments>\n"
    "\t\t\t</arguments>\n"
    "\t\t</buildCommand>\n"
    "\t</buildSpec>\n"
    "\t<natures>\n"
    // *** ccnature only if it is c++ ???
    "\t\t<nature>org.eclipse.cdt.core.ccnature</nature>\n"
    "\t\t<nature>org.eclipse.cdt.make.core.makeNature</nature>\n"
    "\t\t<nature>org.eclipse.cdt.make.core.ScannerConfigNature</nature>\n"
    "\t\t<nature>org.eclipse.cdt.core.cnature</nature>\n"
    "\t</natures>\n"
    "\t<linkedResources>\n"
    ;

  // for each sub project create a linked resource to the source dir
  for (std::map<cmStdString, std::vector<cmLocalGenerator*> >::const_iterator
        it = this->GlobalGenerator->GetProjectMap().begin();
       it != this->GlobalGenerator->GetProjectMap().end();
       ++it)
    {
    fout << "\t\t<link>\n"
            "\t\t\t<name>" << it->first << "</name>\n"
            "\t\t\t<type>2</type>\n"
            "\t\t\t<location>"
            << this->GetEclipsePath(
                 it->second[0]->GetMakefile()->GetStartDirectory())
            << "</location>\n"
            "\t\t</link>\n"
            ;
    }

  fout << "\t</linkedResources>\n"
          "</projectDescription>\n"
          ;
}

//----------------------------------------------------------------------------
void cmExtraEclipseCDT4Generator::CreateCProjectFile() const
{
  std::set<std::string> emmited;
  
  const cmMakefile* mf
    = this->GlobalGenerator->GetLocalGenerators()[0]->GetMakefile();

  std::string filename = mf->GetStartOutputDirectory();
  filename = filename + "/" + ".cproject";

  cmGeneratedFileStream fout(filename.c_str());
  if (!fout)
    {
    return;
    }

  // add header
  fout << 
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
    "<?fileVersion 4.0.0?>\n\n"
    "<cproject>\n"
    "<storageModule moduleId=\"org.eclipse.cdt.core.settings\">\n"
    ;

  fout << "<cconfiguration id=\"org.eclipse.cdt.core.default.config.1\">\n";

  // *** what is this...
  fout << 
    "<storageModule"
    " buildSystemId=\"org.eclipse.cdt.core.defaultConfigDataProvider\""
    " id=\"org.eclipse.cdt.core.default.config.1\""
    " moduleId=\"org.eclipse.cdt.core.settings\" name=\"Configuration\">\n"
    "<externalSettings/>\n"
    "<extensions>\n"
    ;
  // *** refactor this out...
  switch (GetToolChainType(*mf))
  {
    case EclipseToolchainLinux   :
      fout << "<extension id=\"org.eclipse.cdt.core.ELF\""
              " point=\"org.eclipse.cdt.core.BinaryParser\"/>\n"
              ;
      fout << "<extension id=\"org.eclipse.cdt.core.GNU_ELF\""
              " point=\"org.eclipse.cdt.core.BinaryParser\">\n"
              "<attribute key=\"addr2line\" value=\"addr2line\"/>\n"
              "<attribute key=\"c++filt\" value=\"c++filt\"/>\n"
              "</extension>\n"
              ;
      break;
    case EclipseToolchainCygwin  :
      fout << "<extension id=\"org.eclipse.cdt.core.Cygwin_PE\""
              " point=\"org.eclipse.cdt.core.BinaryParser\">\n"
              "<attribute key=\"addr2line\" value=\"addr2line\"/>\n"
              "<attribute key=\"c++filt\" value=\"c++filt\"/>\n"
              "<attribute key=\"cygpath\" value=\"cygpath\"/>\n"
              "<attribute key=\"nm\" value=\"nm\"/>\n"
              "</extension>\n"
              ;
      break;
    case EclipseToolchainMinGW   :
      fout << "<extension id=\"org.eclipse.cdt.core.PE\""
              " point=\"org.eclipse.cdt.core.BinaryParser\"/>\n"
              ;
      break;
    case EclipseToolchainSolaris :
      fout << "<extension id=\"org.eclipse.cdt.core.ELF\""
              " point=\"org.eclipse.cdt.core.BinaryParser\"/>\n"
              ;
      break;
    case EclipseToolchainMacOSX  :
      fout << "<extension id=\"org.eclipse.cdt.core.MachO\""
              " point=\"org.eclipse.cdt.core.BinaryParser\">\n"
              "<attribute key=\"c++filt\" value=\"c++filt\"/>\n"
              "</extension>\n"
              ;
      break;
    case EclipseToolchainOther   :
      fout << "<extension id=\"org.eclipse.cdt.core.PE\""
              " point=\"org.eclipse.cdt.core.BinaryParser\"/>\n"
              ;
      fout << "<extension id=\"org.eclipse.cdt.core.ELF\""
              " point=\"org.eclipse.cdt.core.BinaryParser\"/>\n"
              ;
      break;
    default      :
      // *** Should never get here ***
      fout << "<error_toolchain_type/>\n";
  }
  fout << "</extensions>\n"
          "</storageModule>\n"
          ;
  
  // ???
  fout << 
    "<storageModule moduleId=\"org.eclipse.cdt.core.language.mapping\">\n"
    "<project-mappings/>\n"
    "</storageModule>\n"
    ;

  // ???
  fout<<"<storageModule moduleId=\"org.eclipse.cdt.core.externalSettings\"/>\n"
          ;

  // set the path entries (includes, libs, source dirs, etc.)
  fout << "<storageModule moduleId=\"org.eclipse.cdt.core.pathentry\">\n"
          ;
  // for each sub project with a linked resource to the source dir:
  // - make it type 'src'
  // - and exclude it from type 'out'
  std::string exclude_from_out;
  for (std::map<cmStdString, std::vector<cmLocalGenerator*> >::const_iterator
        it = this->GlobalGenerator->GetProjectMap().begin();
       it != this->GlobalGenerator->GetProjectMap().end();
       ++it)
    {
    fout << "<pathentry kind=\"src\" path=\"" << it->first << "\"/>\n"
            ;
    exclude_from_out += it->first + "/,";
    }
  exclude_from_out.resize(exclude_from_out.size()-1);
  fout << "<pathentry excluding=\"" << exclude_from_out
       << "\" kind=\"out\" path=\"\"/>\n"
       ;
  // include dirs
  emmited.clear();
  for (std::vector<cmLocalGenerator*>::const_iterator
        it = this->GlobalGenerator->GetLocalGenerators().begin();
       it != this->GlobalGenerator->GetLocalGenerators().end();
       ++it)
    {
    const std::vector<std::string>& includeDirs
      = (*it)->GetMakefile()->GetIncludeDirectories();
    for(std::vector<std::string>::const_iterator inc = includeDirs.begin();
        inc != includeDirs.end();
        ++inc)
      {
      if(emmited.find(*inc) == emmited.end())
        {
        emmited.insert(*inc);
        fout << "<pathentry include=\"" << this->GetEclipsePath(*inc)
             << "\" kind=\"inc\" path=\"\" system=\"true\"/>\n";
        }
      }
    }
  fout << "</storageModule>\n"
          ;

  // add build targets
  fout << 
    "<storageModule moduleId=\"org.eclipse.cdt.make.core.buildtargets\">\n"
    "<buildTargets>\n"
    ;
  emmited.clear();
  for (std::vector<cmLocalGenerator*>::const_iterator
        it = this->GlobalGenerator->GetLocalGenerators().begin();
       it != this->GlobalGenerator->GetLocalGenerators().end();
       ++it)
    {
    const cmTargets& targets = (*it)->GetMakefile()->GetTargets();
    for(cmTargets::const_iterator t = targets.begin(); t != targets.end(); ++t)
      {
      if(emmited.find(t->first) == emmited.end())
        {
        emmited.insert(t->first);
        this->AppendTarget(fout, t->first);
        }
      }
    }
  fout << "</buildTargets>\n"
          "</storageModule>\n"
          ;

  this->AppendStorageScanners(fout, *mf);

  fout << "</cconfiguration>\n"
          "</storageModule>\n"
          "<storageModule moduleId=\"cdtBuildSystem\" version=\"4.0.0\">\n"
          "<project id=\"" << mf->GetProjectName() << ".null.1\""
          " name=\"" << mf->GetProjectName() << "\"/>\n"
          "</storageModule>\n"
          "</cproject>\n"
          ;
}

//----------------------------------------------------------------------------
cmExtraEclipseCDT4Generator::EclipseToolchainType
cmExtraEclipseCDT4Generator::GetToolChainType(const cmMakefile& makefile) const
{
  if (makefile.IsSet("UNIX"))
    {
    if (makefile.IsSet("CYGWIN"))
      {
      return EclipseToolchainCygwin;
      }
    if (makefile.IsSet("APPLE" ))
      {
      return EclipseToolchainMacOSX;
      }
    // *** how do I determine if it is Solaris ???
    return EclipseToolchainLinux;
    }
  else if (makefile.IsSet("WIN32"))
    {
    if (makefile.IsSet("MINGW"))
      {
      return EclipseToolchainMinGW;
      }
    if (makefile.IsSet("MSYS" ))
      {
      return EclipseToolchainMinGW;
      }
    return EclipseToolchainOther;
    }
  else
    {
    return EclipseToolchainOther;
    }
}

std::string
cmExtraEclipseCDT4Generator::GetEclipsePath(const std::string& path) const
{
#if defined(__CYGWIN__)
  std::string cmd = "cygpath -w " + path;
  std::string out;
  if (!cmSystemTools::RunCommand(cmd.c_str(), out, 0, false))
    {
    return path;
    }
  else
    {
    out.erase(out.find_last_of('\n'));
    return out;
    }
#else
  return path;
#endif
}

//----------------------------------------------------------------------------
// Helper functions
//----------------------------------------------------------------------------
void cmExtraEclipseCDT4Generator
::AppendStorageScanners(cmGeneratedFileStream& fout,
                        const cmMakefile&      makefile) const
{
  fout << 
    "<storageModule moduleId=\"scannerConfiguration\">\n"
    "<autodiscovery enabled=\"true\" problemReportingEnabled=\"true\""
    " selectedProfileId=\"org.eclipse.cdt.make.core.GCCStandardMakePerProjectProfile\"/>\n"
    ;
  this->AppendScannerProfile(fout,
    "org.eclipse.cdt.make.core.GCCStandardMakePerProjectProfile",
    true, "", true, "specsFile",
    "-E -P -v -dD ${plugin_state_location}/${specs_file}",
    "gcc", true, true);
  this->AppendScannerProfile(fout,
    "org.eclipse.cdt.make.core.GCCStandardMakePerFileProfile",
    true, "", true, "makefileGenerator",
    "-f ${project_name}_scd.mk",
    "make", true, true);
  this->AppendScannerProfile(fout,
    "org.eclipse.cdt.managedbuilder.core.GCCManagedMakePerProjectProfile",
    false, "", false, "specsFile",
    "-E -P -v -dD ${plugin_state_location}/${specs_file}",
    "gcc", true, true);
  this->AppendScannerProfile(fout,
    "org.eclipse.cdt.managedbuilder.core.GCCManagedMakePerProjectProfileCPP",
    false, "", false, "specsFile",
    "-E -P -v -dD ${plugin_state_location}/specs.cpp",
    "g++", true, true);
  this->AppendScannerProfile(fout,
    "org.eclipse.cdt.managedbuilder.core.GCCManagedMakePerProjectProfileC",
    false, "", false, "specsFile",
    "-E -P -v -dD ${plugin_state_location}/specs.c",
    "gcc", true, true);
  this->AppendScannerProfile(fout,
    "org.eclipse.cdt.managedbuilder.core.GCCWinManagedMakePerProjectProfile",
    false, "", false, "specsFile",
    "-E -P -v -dD ${plugin_state_location}/${specs_file}",
    "gcc", true, true);
  this->AppendScannerProfile(fout,
    "org.eclipse.cdt.managedbuilder.core.GCCWinManagedMakePerProjectProfileCPP",
    false, "", false, "specsFile",
    "-E -P -v -dD ${plugin_state_location}/specs.cpp",
    "g++", true, true);
  this->AppendScannerProfile(fout,
    "org.eclipse.cdt.managedbuilder.core.GCCWinManagedMakePerProjectProfileC",
    false, "", false, "specsFile",
    "-E -P -v -dD ${plugin_state_location}/specs.c",
    "gcc", true, true);

/*
  // *** this needs to be conditional on platform ???
  fout << "<scannerConfigBuildInfo instanceId=\""
          "cdt.managedbuild.toolchain.gnu.cygwin.base.1;"
          "cdt.managedbuild.toolchain.gnu.cygwin.base.1.1;"
          "cdt.managedbuild.tool.gnu.cpp.compiler.cygwin.base.1;"
          "cdt.managedbuild.tool.gnu.cpp.compiler.input.cygwin.1\">\n"
          "<autodiscovery enabled=\"true\" problemReportingEnabled=\"true\""
          " selectedProfileId=\""
          "org.eclipse.cdt.managedbuilder.core.GCCWinManagedMakePerProjectProfileCPP\"/>\n"
          ;

  AppendScannerProfile(fout,
    "org.eclipse.cdt.make.core.GCCStandardMakePerProjectProfile",
    true, "", true, "specsFile",
    "-E -P -v -dD ${plugin_state_location}/${specs_file}",
    "gcc", true, true);
  AppendScannerProfile(fout,
    "org.eclipse.cdt.make.core.GCCStandardMakePerFileProfile",
    true, "", true, "makefileGenerator",
    "-f ${project_name}_scd.mk",
    "make", true, true);
  AppendScannerProfile(fout,
    "org.eclipse.cdt.managedbuilder.core.GCCManagedMakePerProjectProfile",
    true, "", false, "specsFile",
    "-E -P -v -dD ${plugin_state_location}/${specs_file}",
    "gcc", true, true);
  AppendScannerProfile(fout,
    "org.eclipse.cdt.managedbuilder.core.GCCManagedMakePerProjectProfileCPP",
    true, "", false, "specsFile",
    "-E -P -v -dD ${plugin_state_location}/specs.cpp",
    "g++", true, true);
  AppendScannerProfile(fout,
    "org.eclipse.cdt.managedbuilder.core.GCCManagedMakePerProjectProfileC",
    true, "", false, "specsFile",
    "-E -P -v -dD ${plugin_state_location}/specs.c",
    "gcc", true, true);
  AppendScannerProfile(fout,
    "org.eclipse.cdt.managedbuilder.core.GCCWinManagedMakePerProjectProfile",
    true, "", false, "specsFile",
    "-E -P -v -dD ${plugin_state_location}/${specs_file}",
    "gcc", true, true);
  AppendScannerProfile(fout,
    "org.eclipse.cdt.managedbuilder.core.GCCWinManagedMakePerProjectProfileCPP",
    true, "", false, "specsFile",
    "-E -P -v -dD ${plugin_state_location}/specs.cpp",
    "g++", true, true);
  AppendScannerProfile(fout,
    "org.eclipse.cdt.managedbuilder.core.GCCWinManagedMakePerProjectProfileC",
    true, "", false, "specsFile",
    "-E -P -v -dD ${plugin_state_location}/specs.c",
    "gcc", true, true);

  // *** this needs to be conditional on platform ???
  fout << "</scannerConfigBuildInfo>\n"
          "<scannerConfigBuildInfo instanceId=\""
          "cdt.managedbuild.toolchain.gnu.cygwin.base.1;"
          "cdt.managedbuild.toolchain.gnu.cygwin.base.1.1;"
          "cdt.managedbuild.tool.gnu.c.compiler.cygwin.base.1;"
          "cdt.managedbuild.tool.gnu.c.compiler.input.cygwin.1\">\n"
          "<autodiscovery enabled=\"true\" problemReportingEnabled=\"true\""
          " selectedProfileId=\""
          "org.eclipse.cdt.managedbuilder.core.GCCWinManagedMakePerProjectProfileC\"/>\n"
          ;

  AppendScannerProfile(fout,
    "org.eclipse.cdt.make.core.GCCStandardMakePerProjectProfile",
    true, "", true, "specsFile",
    "-E -P -v -dD ${plugin_state_location}/${specs_file}",
    "gcc", true, true);
  AppendScannerProfile(fout,
    "org.eclipse.cdt.make.core.GCCStandardMakePerFileProfile",
    true, "", true, "makefileGenerator",
    "-f ${project_name}_scd.mk",
    "make", true, true);
  AppendScannerProfile(fout,
    "org.eclipse.cdt.managedbuilder.core.GCCManagedMakePerProjectProfile",
    false, "", false, "specsFile",
    "-E -P -v -dD ${plugin_state_location}/${specs_file}",
    "gcc", true, true);
  AppendScannerProfile(fout,
    "org.eclipse.cdt.managedbuilder.core.GCCManagedMakePerProjectProfileCPP",
    false, "", false, "specsFile",
    "-E -P -v -dD ${plugin_state_location}/specs.cpp",
    "g++", true, true);
  AppendScannerProfile(fout,
    "org.eclipse.cdt.managedbuilder.core.GCCManagedMakePerProjectProfileC",
    false, "", false, "specsFile",
    "-E -P -v -dD ${plugin_state_location}/specs.c",
    "gcc", true, true);
  AppendScannerProfile(fout,
    "org.eclipse.cdt.managedbuilder.core.GCCWinManagedMakePerProjectProfile",
    false, "", false, "specsFile",
    "-E -P -v -dD ${plugin_state_location}/${specs_file}",
    "gcc", true, true);
  AppendScannerProfile(fout,
    "org.eclipse.cdt.managedbuilder.core.GCCWinManagedMakePerProjectProfileCPP",
    false, "", false, "specsFile",
    "-E -P -v -dD ${plugin_state_location}/specs.cpp",
    "g++", true, true);
  AppendScannerProfile(fout,
    "org.eclipse.cdt.managedbuilder.core.GCCWinManagedMakePerProjectProfileC",
    false, "", false, "specsFile",
    "-E -P -v -dD ${plugin_state_location}/specs.c",
    "gcc", true, true);

  fout << "</scannerConfigBuildInfo>\n"
          ;
*/

  fout << "</storageModule>\n"
          ;
}

void cmExtraEclipseCDT4Generator
::AppendTarget(cmGeneratedFileStream& fout,
               const std::string&     target) const
{
  const cmMakefile& mf
    = *(this->GlobalGenerator->GetLocalGenerators()[0]->GetMakefile());
  std::string make = mf.GetRequiredDefinition("CMAKE_MAKE_PROGRAM");

  fout << 
    "<target name=\"" << target << "\""
    " path=\"\""
    " targetID=\"org.eclipse.cdt.make.MakeTargetBuilder\">\n"
    "<buildCommand>" << make << "</buildCommand>\n"
    "<buildArguments/>\n"
    "<buildTarget>" << target << "</buildTarget>\n"
    "<stopOnError>true</stopOnError>\n"
    "<useDefaultCommand>false</useDefaultCommand>\n"
    "</target>\n"
    ;
}

void cmExtraEclipseCDT4Generator
::AppendScannerProfile(cmGeneratedFileStream& fout,
                       const std::string&     profileID,
                       bool                   openActionEnabled,
                       const std::string&     openActionFilePath,
                       bool                   pParserEnabled,
                       const std::string&     scannerInfoProviderID,
                       const std::string&     runActionArguments,
                       const std::string&     runActionCommand,
                       bool                   runActionUseDefault,
                       bool                   sipParserEnabled) const
{
  fout << 
    "<profile id=\"" << profileID << "\">\n"
    "<buildOutputProvider>\n"
    "<openAction enabled=\"" << (openActionEnabled ? "true" : "false")
    << "\" filePath=\"" << openActionFilePath << "\"/>\n"
    "<parser enabled=\"" << (pParserEnabled ? "true" : "false") << "\"/>\n"
    "</buildOutputProvider>\n"
    "<scannerInfoProvider id=\"" << scannerInfoProviderID << "\">\n"
    "<runAction arguments=\"" << runActionArguments << "\""
    " command=\"" << runActionCommand
    << "\" useDefault=\"" << (runActionUseDefault ? "true":"false") << "\"/>\n"
    "<parser enabled=\"" << (sipParserEnabled ? "true" : "false") << "\"/>\n"
    "</scannerInfoProvider>\n"
    "</profile>\n"
    ;
}
