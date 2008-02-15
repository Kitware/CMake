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
#include <stdlib.h>

//----------------------------------------------------------------------------
cmExtraEclipseCDT4Generator
::cmExtraEclipseCDT4Generator() : cmExternalMakefileProjectGenerator()
{
// TODO: Verify if __CYGWIN__ should be checked.
//#if defined(_WIN32) && !defined(__CYGWIN__)
#if defined(_WIN32)
  this->SupportedGlobalGenerators.push_back("NMake Makefiles");
  this->SupportedGlobalGenerators.push_back("MinGW Makefiles");
//  this->SupportedGlobalGenerators.push_back("MSYS Makefiles");
#endif
  this->SupportedGlobalGenerators.push_back("Unix Makefiles");
}

//----------------------------------------------------------------------------
void cmExtraEclipseCDT4Generator
::GetDocumentation(cmDocumentationEntry& entry, const char*) const
{
  entry.Name = this->GetName();
  entry.Brief = "Generates Eclipse CDT 4.0 project files.";
  entry.Full =
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
  mf->SetToolSupportsColor(true);
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

  const std::string homeDirectory(mf->GetHomeDirectory());
  const std::string homeOutputDirectory(mf->GetHomeOutputDirectory());
  const std::string filename = homeOutputDirectory + "/.project";

  cmGeneratedFileStream fout(filename.c_str());
  if (!fout)
    {
    return;
    }

  fout << 
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<projectDescription>\n"
    "\t<name>" << this->GetPathBasename(homeOutputDirectory) << "</name>\n"
    "\t<comment></comment>\n"
    "\t<projects>\n"
    "\t</projects>\n"
    "\t<buildSpec>\n"
    "\t\t<buildCommand>\n"
    "\t\t\t<name>org.eclipse.cdt.make.core.makeBuilder</name>\n"
    "\t\t\t<triggers>clean,full,incremental,</triggers>\n"
    "\t\t\t<arguments>\n"
    ;

  // use clean target
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

  // set the make command
  std::string make = mf->GetRequiredDefinition("CMAKE_MAKE_PROGRAM");
  fout << 
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.enabledIncrementalBuild</key>\n"
    "\t\t\t\t\t<value>true</value>\n"
    "\t\t\t\t</dictionary>\n"
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.build.command</key>\n"
    "\t\t\t\t\t<value>" + this->GetEclipsePath(make) + "</value>\n"
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
     << this->GetEclipsePath(homeOutputDirectory) << "</value>\n"
    "\t\t\t\t</dictionary>\n"
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.useDefaultBuildCmd</key>\n"
    "\t\t\t\t\t<value>false</value>\n"
    "\t\t\t\t</dictionary>\n"
    ;

  // set project specific environment
  fout <<
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.environment</key>\n"
    "\t\t\t\t\t<value>"
    ;
  // set vsvars32.bat environment available at CMake time,
  //   but not necessarily when eclipse is open
  if (make.find("nmake") != std::string::npos)
    {
    if (getenv("PATH"))
      {
      fout << "PATH=" << getenv("PATH") << "|";
      }
    if (getenv("INCLUDE"))
      {
      fout << "INCLUDE=" << getenv("INCLUDE") << "|";
      }
    if (getenv("LIB"))
      {
      fout << "LIB=" << getenv("LIB") << "|";
      }
    if (getenv("LIBPATH"))
      {
      fout << "LIBPATH=" << getenv("LIBPATH") << "|";
      }
    }
  fout <<
    "</value>\n"
    "\t\t\t\t</dictionary>\n"
    ;

  fout <<
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
    << this->GetEclipsePath(homeOutputDirectory) << "</value>\n"
    "\t\t\t\t</dictionary>\n"
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.make.core.autoBuildTarget</key>\n"
    "\t\t\t\t\t<value>all</value>\n"
    "\t\t\t\t</dictionary>\n"
    ;

  // set error parsers
  fout <<
    "\t\t\t\t<dictionary>\n"
    "\t\t\t\t\t<key>org.eclipse.cdt.core.errorOutputParser</key>\n"
    "\t\t\t\t\t<value>"
    ;
  if (this->GetToolChainType(*mf) == EclipseToolchainOther)
    {
    fout << "org.eclipse.cdt.core.VCErrorParser;";
    }
  fout <<
    "org.eclipse.cdt.core.MakeErrorParser;"
    "org.eclipse.cdt.core.GCCErrorParser;"
    "org.eclipse.cdt.core.GASErrorParser;"
    "org.eclipse.cdt.core.GLDErrorParser;"
    "</value>\n"
    "\t\t\t\t</dictionary>\n"
    ;

  fout <<
    "\t\t\t</arguments>\n"
    "\t\t</buildCommand>\n"
    "\t\t<buildCommand>\n"
    "\t\t\t<name>org.eclipse.cdt.make.core.ScannerConfigBuilder</name>\n"
    "\t\t\t<arguments>\n"
    "\t\t\t</arguments>\n"
    "\t\t</buildCommand>\n"
    "\t</buildSpec>\n"
    ;

  // set natures for c/c++ projects
  fout <<
    "\t<natures>\n"
    // TODO: ccnature only if it is c++ ???
    "\t\t<nature>org.eclipse.cdt.core.ccnature</nature>\n"
    "\t\t<nature>org.eclipse.cdt.make.core.makeNature</nature>\n"
    "\t\t<nature>org.eclipse.cdt.make.core.ScannerConfigNature</nature>\n"
    "\t\t<nature>org.eclipse.cdt.core.cnature</nature>\n"
    "\t</natures>\n"
    ;

  // TODO: refactor this
  // create linked resources
  if (homeDirectory != homeOutputDirectory)
    {
    fout << "\t<linkedResources>\n";
    // for each sub project create a linked resource to the source dir
    // - only if it is an out-of-source build
    for (std::map<cmStdString, std::vector<cmLocalGenerator*> >::const_iterator
          it = this->GlobalGenerator->GetProjectMap().begin();
         it != this->GlobalGenerator->GetProjectMap().end();
         ++it)
      {
      std::string linkSourceDirectory =this->GetEclipsePath(
                            it->second[0]->GetMakefile()->GetStartDirectory());
      if (!cmSystemTools::IsSubDirectory(homeOutputDirectory.c_str(),
                                         linkSourceDirectory.c_str()))
        {
        fout <<
          "\t\t<link>\n"
          "\t\t\t<name>" << it->first << "</name>\n"
          "\t\t\t<type>2</type>\n"
          "\t\t\t<location>"
          << this->GetEclipsePath(linkSourceDirectory)
          << "</location>\n"
          "\t\t</link>\n"
          ;
        }
      }
    // for EXECUTABLE_OUTPUT_PATH when not in binary dir
    std::string output_path = mf->GetDefinition("EXECUTABLE_OUTPUT_PATH");
    if (!cmSystemTools::IsSubDirectory(output_path.c_str(),
                                       homeOutputDirectory.c_str()))
      {
      std::string name = this->GetPathBasename(output_path);
      while (this->GlobalGenerator->GetProjectMap().find(name)
             != this->GlobalGenerator->GetProjectMap().end())
        {
        name += "_";
        }
      fout <<
        "\t\t<link>\n"
        "\t\t\t<name>" << name << "</name>\n"
        "\t\t\t<type>2</type>\n"
        "\t\t\t<location>"
        << this->GetEclipsePath(output_path)
        << "</location>\n"
        "\t\t</link>\n"
        ;
      }
    // for LIBRARY_OUTPUT_PATH when not in binary dir
    if (output_path != mf->GetDefinition("LIBRARY_OUTPUT_PATH"))
      {
      output_path = mf->GetDefinition("LIBRARY_OUTPUT_PATH");
      if (!cmSystemTools::IsSubDirectory(output_path.c_str(),
                                         homeOutputDirectory.c_str()))
        {
        std::string name = this->GetPathBasename(output_path);
        while (this->GlobalGenerator->GetProjectMap().find(name)
               != this->GlobalGenerator->GetProjectMap().end())
          {
          name += "_";
          }
        fout <<
          "\t\t<link>\n"
          "\t\t\t<name>" << name << "</name>\n"
          "\t\t\t<type>2</type>\n"
          "\t\t\t<location>"
          << this->GetEclipsePath(output_path)
          << "</location>\n"
          "\t\t</link>\n"
          ;
        }
      }
    fout << "\t</linkedResources>\n";
    }

  fout << "</projectDescription>\n";
}

//----------------------------------------------------------------------------
void cmExtraEclipseCDT4Generator::CreateCProjectFile() const
{
  std::set<std::string> emmited;
  
  const cmMakefile* mf
    = this->GlobalGenerator->GetLocalGenerators()[0]->GetMakefile();

  const std::string homeOutputDirectory(mf->GetHomeOutputDirectory());
  const std::string filename = homeOutputDirectory + "/.cproject";

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

  // Configuration settings...
  fout << 
    "<storageModule"
    " buildSystemId=\"org.eclipse.cdt.core.defaultConfigDataProvider\""
    " id=\"org.eclipse.cdt.core.default.config.1\""
    " moduleId=\"org.eclipse.cdt.core.settings\" name=\"Configuration\">\n"
    "<externalSettings/>\n"
    "<extensions>\n"
    ;
  // TODO: refactor this out...
  switch (this->GetToolChainType(*mf))
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
    fout << "<pathentry kind=\"src\" path=\"" << it->first << "\"/>\n";

    // exlude source directory from output search path
    // - only if not named the same as an output directory
    if (!cmSystemTools::FileIsDirectory(
           std::string(homeOutputDirectory + "/" + it->first).c_str()))
      {
      exclude_from_out += it->first + "/|";
      }
    }
  exclude_from_out += "**/CMakeFiles/";
  fout << "<pathentry excluding=\"" << exclude_from_out
       << "\" kind=\"out\" path=\"\"/>\n";
  // add output entry for EXECUTABLE_OUTPUT_PATH and LIBRARY_OUTPUT_PATH
  // - if it is a subdir of homeOutputDirectory, there is no need to add it
  // - if it is not then create a linked resource and add the linked name
  //   but check it doesn't conflict with other linked resources names
  std::string output_path = mf->GetDefinition("EXECUTABLE_OUTPUT_PATH");
  if (!cmSystemTools::IsSubDirectory(output_path.c_str(),
                                     homeOutputDirectory.c_str()))
    {
    std::string name = this->GetPathBasename(output_path);
    while (this->GlobalGenerator->GetProjectMap().find(name)
           != this->GlobalGenerator->GetProjectMap().end())
      {
      name += "_";
      }
      fout << "<pathentry kind=\"out\" path=\"" << name << "\"/>\n";
    }
  // for LIBRARY_OUTPUT_PATH when not in binary dir
  if (output_path != mf->GetDefinition("LIBRARY_OUTPUT_PATH"))
    {
    output_path = mf->GetDefinition("LIBRARY_OUTPUT_PATH");
    if (!cmSystemTools::IsSubDirectory(output_path.c_str(),
                                       homeOutputDirectory.c_str()))
      {
      std::string name = this->GetPathBasename(output_path);
      while (this->GlobalGenerator->GetProjectMap().find(name)
             != this->GlobalGenerator->GetProjectMap().end())
        {
        name += "_";
        }
      fout << "<pathentry kind=\"out\" path=\"" << name << "\"/>\n";
      }
    }

  // add pre-processor definitions to allow eclipse to gray out sections
  emmited.clear();
  for (std::vector<cmLocalGenerator*>::const_iterator
        it = this->GlobalGenerator->GetLocalGenerators().begin();
       it != this->GlobalGenerator->GetLocalGenerators().end();
       ++it)
    {

    if(const char* cdefs = (*it)->GetMakefile()->GetProperty(
                                                        "COMPILE_DEFINITIONS"))
      {
      // Expand the list.
      std::vector<std::string> defs;
      cmSystemTools::ExpandListArgument(cdefs, defs);

      for(std::vector<std::string>::const_iterator di = defs.begin();
          di != defs.end(); ++di)
        {
        std::string::size_type equals = di->find('=', 0);
        std::string::size_type enddef = di->length();
      
        std::string def;
        std::string val;
        if (equals != std::string::npos && equals < enddef)
          {
          // we have -DFOO=BAR
          def = di->substr(0, equals);
          val = di->substr(equals + 1, enddef - equals + 1);
          }
        else
          {
          // we have -DFOO
          def = *di;
          }
          
        // insert the definition if not already added.
        if(emmited.find(def) == emmited.end())
          {
          emmited.insert(def);
          fout << "<pathentry kind=\"mac\" name=\"" << def
               << "\" path=\"\" value=\"" << val << "\"/>\n";
          }
        }
      }
    }

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
      std::string dir = cmSystemTools::CollapseFullPath(inc->c_str());
      if(emmited.find(dir) == emmited.end())
        {
        emmited.insert(dir);
        fout << "<pathentry include=\"" << this->GetEclipsePath(dir)
             << "\" kind=\"inc\" path=\"\" system=\"true\"/>\n";
        }
      }
    }
  fout << "</storageModule>\n";

  // add build targets
  fout << 
    "<storageModule moduleId=\"org.eclipse.cdt.make.core.buildtargets\">\n"
    "<buildTargets>\n"
    ;
  emmited.clear();
  const std::string make = mf->GetRequiredDefinition("CMAKE_MAKE_PROGRAM");
  cmGlobalGenerator* generator
    = const_cast<cmGlobalGenerator*>(this->GlobalGenerator);
  if (generator->GetAllTargetName())
    {
    emmited.insert(generator->GetAllTargetName());
    cmExtraEclipseCDT4Generator::AppendTarget(fout,
                                              generator->GetAllTargetName(),
                                              make);
    }
  if (generator->GetPreinstallTargetName())
    {
    emmited.insert(generator->GetPreinstallTargetName());
    cmExtraEclipseCDT4Generator
    ::AppendTarget(fout, generator->GetPreinstallTargetName(), make);
    }

  if (generator->GetCleanTargetName())
    {
    emmited.insert(generator->GetCleanTargetName());
    cmExtraEclipseCDT4Generator
    ::AppendTarget(fout, generator->GetCleanTargetName(), make);
    }

  // add all executable and library targets and some of the GLOBAL 
  // and UTILITY targets
  for (std::vector<cmLocalGenerator*>::const_iterator
        it = this->GlobalGenerator->GetLocalGenerators().begin();
       it != this->GlobalGenerator->GetLocalGenerators().end();
       ++it)
    {
    const cmTargets& targets = (*it)->GetMakefile()->GetTargets();
    for(cmTargets::const_iterator t = targets.begin(); t != targets.end(); ++t)
      {
      switch(t->second.GetType())
        {
        case cmTarget::UTILITY:
        case cmTarget::GLOBAL_TARGET:
          {
          // only add these global targets
          if (!( (t->first=="install")
              || (t->first=="install/strip")
              || (t->first=="test")
              || (t->first=="Experimental")
              || (t->first=="Nightly")
              || (t->first=="package")
              || (t->first=="package_source")
              || (t->first=="rebuild_cache") ))
            {
            break;
            }
          }
        case cmTarget::EXECUTABLE:
        case cmTarget::STATIC_LIBRARY:
        case cmTarget::SHARED_LIBRARY:
        case cmTarget::MODULE_LIBRARY:
          {
          if(emmited.find(t->first) == emmited.end())
            {
            emmited.insert(t->first);
            cmExtraEclipseCDT4Generator::AppendTarget(fout, t->first, make);
            }
           break;
          }
        // ignore these:
        case cmTarget::INSTALL_FILES:
        case cmTarget::INSTALL_PROGRAMS:
        case cmTarget::INSTALL_DIRECTORY:
        default:
          break;
        }
      }
    }
  fout << "</buildTargets>\n"
          "</storageModule>\n"
          ;

  this->AppendStorageScanners(fout);

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
cmExtraEclipseCDT4Generator::GetToolChainType(const cmMakefile& makefile)
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
cmExtraEclipseCDT4Generator::GetEclipsePath(const std::string& path)
{
#if defined(__CYGWIN__)
  std::string cmd = "cygpath -m " + path;
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

std::string
cmExtraEclipseCDT4Generator::GetPathBasename(const std::string& path)
{
  std::string outputBasename = path;
  while (outputBasename.size() > 0 &&
         (outputBasename[outputBasename.size() - 1] == '/' ||
          outputBasename[outputBasename.size() - 1] == '\\'))
    {
    outputBasename.resize(outputBasename.size() - 1);
    }
  std::string::size_type loc = outputBasename.find_last_of("/\\");
  if (loc != std::string::npos)
    {
    outputBasename = outputBasename.substr(loc + 1);
    }

  return outputBasename;
}

//----------------------------------------------------------------------------
// Helper functions
//----------------------------------------------------------------------------
void cmExtraEclipseCDT4Generator
::AppendStorageScanners(cmGeneratedFileStream& fout)
{
  fout << 
    "<storageModule moduleId=\"scannerConfiguration\">\n"
    "<autodiscovery enabled=\"true\" problemReportingEnabled=\"true\""
    " selectedProfileId="
    "\"org.eclipse.cdt.make.core.GCCStandardMakePerProjectProfile\"/>\n"
    ;
  cmExtraEclipseCDT4Generator::AppendScannerProfile(fout,
    "org.eclipse.cdt.make.core.GCCStandardMakePerProjectProfile",
    true, "", true, "specsFile",
    "-E -P -v -dD ${plugin_state_location}/${specs_file}",
    "gcc", true, true);
  cmExtraEclipseCDT4Generator::AppendScannerProfile(fout,
    "org.eclipse.cdt.make.core.GCCStandardMakePerFileProfile",
    true, "", true, "makefileGenerator",
    "-f ${project_name}_scd.mk",
    "make", true, true);

  fout << "</storageModule>\n";
}

void cmExtraEclipseCDT4Generator::AppendTarget(cmGeneratedFileStream& fout,
                                               const std::string&     target,
                                               const std::string&     make)
{
  fout << 
    "<target name=\"" << target << "\""
    " path=\"\""
    " targetID=\"org.eclipse.cdt.make.MakeTargetBuilder\">\n"
    "<buildCommand>"
    << cmExtraEclipseCDT4Generator::GetEclipsePath(make)
    << "</buildCommand>\n"
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
                       bool                   sipParserEnabled)
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
