/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2004-2009 Kitware, Inc.
  Copyright 2004 Alexander Neundorf (neundorf@kde.org)
  Copyright 2007 Miguel A. Figueroa-Villanueva

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmExtraEclipseCDT4Generator.h"
#include "cmGlobalUnixMakefileGenerator3.h"
#include "cmLocalUnixMakefileGenerator3.h"
#include "cmMakefile.h"
#include "cmGeneratedFileStream.h"
#include "cmTarget.h"

#include "cmSystemTools.h"
#include <stdlib.h>
#include <assert.h>

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
    "Project files for Eclipse will be created in the top directory. "
    "In out of source builds, a linked resource to the top level source "
    "directory will be created."
    "Additionally a hierarchy of makefiles is generated into the "
    "build tree. The appropriate make program can build the project through "
    "the default make target. A \"make install\" target is also provided.";
}

//----------------------------------------------------------------------------
void cmExtraEclipseCDT4Generator::Generate()
{
  const cmMakefile* mf
    = this->GlobalGenerator->GetLocalGenerators()[0]->GetMakefile();

  // TODO: Decide if these are local or member variables
  this->HomeDirectory       = mf->GetHomeDirectory();
  this->HomeOutputDirectory = mf->GetHomeOutputDirectory();

  this->IsOutOfSourceBuild = (this->HomeDirectory!=this->HomeOutputDirectory);

  this->GenerateSourceProject = (this->IsOutOfSourceBuild &&
                            mf->IsOn("ECLIPSE_CDT4_GENERATE_SOURCE_PROJECT"));

  // NOTE: This is not good, since it pollutes the source tree. However,
  //       Eclipse doesn't allow CVS/SVN to work when the .project is not in
  //       the cvs/svn root directory. Hence, this is provided as an option.
  if (this->GenerateSourceProject)
    {
    // create .project file in the source tree
    this->CreateSourceProjectFile();
    }

  // create a .project file
  this->CreateProjectFile();

  // create a .cproject file
  this->CreateCProjectFile();
}

void cmExtraEclipseCDT4Generator::CreateSourceProjectFile() const
{
  assert(this->HomeDirectory != this->HomeOutputDirectory);

  // set up the project name: <project>-Source@<baseSourcePathName>
  const cmMakefile* mf
     = this->GlobalGenerator->GetLocalGenerators()[0]->GetMakefile();
  std::string name = this->GenerateProjectName(mf->GetProjectName(), "Source",
                                   this->GetPathBasename(this->HomeDirectory));

  const std::string filename = this->HomeDirectory + "/.project";
  cmGeneratedFileStream fout(filename.c_str());
  if (!fout)
    {
    return;
    }

  fout <<
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<projectDescription>\n"
    "\t<name>" << this->EscapeForXML(name) << "</name>\n"
    "\t<comment></comment>\n"
    "\t<projects>\n"
    "\t</projects>\n"
    "\t<buildSpec>\n"
    "\t</buildSpec>\n"
    "\t<natures>\n"
    "\t</natures>\n"
    "</projectDescription>\n"
    ;
}


//----------------------------------------------------------------------------
void cmExtraEclipseCDT4Generator::AddEnvVar(cmGeneratedFileStream& fout,
                                            const char* envVar, cmMakefile* mf)
{
  // get the variables from the environment and from the cache and then
  // figure out which one to use:

  const char* envVarValue = getenv(envVar);

  std::string cacheEntryName = "CMAKE_ECLIPSE_ENVVAR_";
  cacheEntryName += envVar;
  const char* cacheValue = mf->GetCacheManager()->GetCacheValue(
                                                       cacheEntryName.c_str());

  // now we have both, decide which one to use
  std::string valueToUse;
  if (envVarValue==0 && cacheValue==0)
    {
    // nothing known, do nothing
    valueToUse = "";
    }
  else if (envVarValue!=0 && cacheValue==0)
    {
    // The variable is in the env, but not in the cache. Use it and put it
    // in the cache
    valueToUse = envVarValue;
    mf->AddCacheDefinition(cacheEntryName.c_str(), valueToUse.c_str(),
                           cacheEntryName.c_str(), cmCacheManager::STRING,
                           true);
    mf->GetCacheManager()->SaveCache(mf->GetHomeOutputDirectory());
    }
  else if (envVarValue==0 && cacheValue!=0)
    {
    // It is already in the cache, but not in the env, so use it from the cache
    valueToUse = cacheValue;
    }
  else
    {
    // It is both in the cache and in the env.
    // Use the version from the env. except if the value from the env is
    // completely contained in the value from the cache (for the case that we
    // now have a PATH without MSVC dirs in the env. but had the full PATH with
    // all MSVC dirs during the cmake run which stored the var in the cache:
    valueToUse = cacheValue;
    if (valueToUse.find(envVarValue) == std::string::npos)
      {
      valueToUse = envVarValue;
      mf->AddCacheDefinition(cacheEntryName.c_str(), valueToUse.c_str(),
                             cacheEntryName.c_str(), cmCacheManager::STRING,
                             true);
      mf->GetCacheManager()->SaveCache(mf->GetHomeOutputDirectory());
      }
    }

  if (!valueToUse.empty())
    {
    fout << envVar << "=" << valueToUse << "|";
    }
}


//----------------------------------------------------------------------------
void cmExtraEclipseCDT4Generator::CreateProjectFile()
{
  cmMakefile* mf
    = this->GlobalGenerator->GetLocalGenerators()[0]->GetMakefile();

  const std::string filename = this->HomeOutputDirectory + "/.project";

  cmGeneratedFileStream fout(filename.c_str());
  if (!fout)
    {
    return;
    }

  std::string compilerId = mf->GetSafeDefinition("CMAKE_C_COMPILER_ID");
  if (compilerId.empty())  // no C compiler, try the C++ compiler:
    {
    compilerId = mf->GetSafeDefinition("CMAKE_CXX_COMPILER_ID");
    }

  fout <<
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<projectDescription>\n"
    "\t<name>" <<
    this->GenerateProjectName(mf->GetProjectName(),
                              mf->GetSafeDefinition("CMAKE_BUILD_TYPE"),
                              this->GetPathBasename(this->HomeOutputDirectory))
    << "</name>\n"
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
     << this->GetEclipsePath(this->HomeOutputDirectory) << "</value>\n"
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
    "\t\t\t\t\t<value>VERBOSE=1|CMAKE_NO_VERBOSE=1|"  //verbose Makefile output
    ;
  // set vsvars32.bat environment available at CMake time,
  //   but not necessarily when eclipse is open
  if (compilerId == "MSVC")
    {
    AddEnvVar(fout, "PATH", mf);
    AddEnvVar(fout, "INCLUDE", mf);
    AddEnvVar(fout, "LIB", mf);
    AddEnvVar(fout, "LIBPATH", mf);
    }
  else if (compilerId == "Intel")
    {
    // if the env.var is set, use this one and put it in the cache
    // if the env.var is not set, but the value is in the cache,
    // use it from the cache:
    AddEnvVar(fout, "INTEL_LICENSE_FILE", mf);
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
    << this->GetEclipsePath(this->HomeOutputDirectory) << "</value>\n"
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
  if (compilerId == "MSVC")
    {
    fout << "org.eclipse.cdt.core.VCErrorParser;";
    }
  else if (compilerId == "Intel")
    {
    fout << "org.eclipse.cdt.core.ICCErrorParser;";
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

  fout << "\t<linkedResources>\n";
  // create linked resources
  if (this->IsOutOfSourceBuild)
    {
    // create a linked resource to CMAKE_SOURCE_DIR
    // (this is not done anymore for each project because of
    // http://public.kitware.com/Bug/view.php?id=9978 and because I found it
    // actually quite confusing in bigger projects with many directories and
    // projects, Alex

    std::string sourceLinkedResourceName = "[Source directory]";
    std::string linkSourceDirectory = this->GetEclipsePath(
                                                      mf->GetStartDirectory());
    // .project dir can't be subdir of a linked resource dir
    if (!cmSystemTools::IsSubDirectory(this->HomeOutputDirectory.c_str(),
                                         linkSourceDirectory.c_str()))
      {
      this->AppendLinkedResource(fout, sourceLinkedResourceName,
                                 this->GetEclipsePath(linkSourceDirectory));
      this->SrcLinkedResources.push_back(sourceLinkedResourceName);
      }

    }

  // for each sub project create a linked resource to the source dir
  // - only if it is an out-of-source build
  this->AppendLinkedResource(fout, "[Subprojects]",
                             "virtual:/virtual", true);

  for (std::map<cmStdString, std::vector<cmLocalGenerator*> >::const_iterator
       it = this->GlobalGenerator->GetProjectMap().begin();
       it != this->GlobalGenerator->GetProjectMap().end();
       ++it)
    {
    std::string linkSourceDirectory = this->GetEclipsePath(
                            it->second[0]->GetMakefile()->GetStartDirectory());
    // a linked resource must not point to a parent directory of .project or
    // .project itself
    if ((this->HomeOutputDirectory != linkSourceDirectory) &&
        !cmSystemTools::IsSubDirectory(this->HomeOutputDirectory.c_str(),
                                       linkSourceDirectory.c_str()))
      {
      std::string linkName = "[Subprojects]/";
      linkName += it->first;
      this->AppendLinkedResource(fout, linkName,
                                 this->GetEclipsePath(linkSourceDirectory));
      this->SrcLinkedResources.push_back(it->first);
      }
    }

  // I'm not sure this makes too much sense. There can be different
  // output directories in different subdirs, so we would need more of them.

  // for EXECUTABLE_OUTPUT_PATH when not in binary dir
  this->AppendOutLinkedResource(fout,
    mf->GetSafeDefinition("CMAKE_RUNTIME_OUTPUT_DIRECTORY"),
    mf->GetSafeDefinition("EXECUTABLE_OUTPUT_PATH"));
  // for LIBRARY_OUTPUT_PATH when not in binary dir
  this->AppendOutLinkedResource(fout,
    mf->GetSafeDefinition("CMAKE_LIBRARY_OUTPUT_DIRECTORY"),
    mf->GetSafeDefinition("LIBRARY_OUTPUT_PATH"));

  fout << "\t</linkedResources>\n";

  fout << "</projectDescription>\n";
}

//----------------------------------------------------------------------------
void cmExtraEclipseCDT4Generator::AppendIncludeDirectories(
                            cmGeneratedFileStream& fout,
                            const std::vector<std::string>& includeDirs,
                            std::set<std::string>& emittedDirs)
{
  for(std::vector<std::string>::const_iterator inc = includeDirs.begin();
      inc != includeDirs.end();
      ++inc)
    {
    if (!inc->empty())
      {
      std::string dir = cmSystemTools::CollapseFullPath(inc->c_str());
      if(emittedDirs.find(dir) == emittedDirs.end())
        {
        emittedDirs.insert(dir);
        fout << "<pathentry include=\""
             << cmExtraEclipseCDT4Generator::EscapeForXML(
                              cmExtraEclipseCDT4Generator::GetEclipsePath(dir))
             << "\" kind=\"inc\" path=\"\" system=\"true\"/>\n";
        }
      }
    }
}

//----------------------------------------------------------------------------
void cmExtraEclipseCDT4Generator::CreateCProjectFile() const
{
  std::set<std::string> emmited;

  const cmMakefile* mf
    = this->GlobalGenerator->GetLocalGenerators()[0]->GetMakefile();

  const std::string filename = this->HomeOutputDirectory + "/.cproject";

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
  std::string executableFormat = mf->GetSafeDefinition(
                                                    "CMAKE_EXECUTABLE_FORMAT");
  if (executableFormat == "ELF")
    {
    fout << "<extension id=\"org.eclipse.cdt.core.ELF\""
            " point=\"org.eclipse.cdt.core.BinaryParser\"/>\n"
            ;
    fout << "<extension id=\"org.eclipse.cdt.core.GNU_ELF\""
            " point=\"org.eclipse.cdt.core.BinaryParser\">\n"
            "<attribute key=\"addr2line\" value=\"addr2line\"/>\n"
            "<attribute key=\"c++filt\" value=\"c++filt\"/>\n"
            "</extension>\n"
            ;
    }
  else
    {
    std::string systemName = mf->GetSafeDefinition("CMAKE_SYSTEM_NAME");
    if (systemName == "CYGWIN")
      {
      fout << "<extension id=\"org.eclipse.cdt.core.Cygwin_PE\""
              " point=\"org.eclipse.cdt.core.BinaryParser\">\n"
              "<attribute key=\"addr2line\" value=\"addr2line\"/>\n"
              "<attribute key=\"c++filt\" value=\"c++filt\"/>\n"
              "<attribute key=\"cygpath\" value=\"cygpath\"/>\n"
              "<attribute key=\"nm\" value=\"nm\"/>\n"
              "</extension>\n"
              ;
      }
    else if (systemName == "Windows")
      {
      fout << "<extension id=\"org.eclipse.cdt.core.PE\""
              " point=\"org.eclipse.cdt.core.BinaryParser\"/>\n"
              ;
      }
    else if (systemName == "Darwin")
      {
      fout << "<extension id=\"org.eclipse.cdt.core.MachO\""
              " point=\"org.eclipse.cdt.core.BinaryParser\">\n"
              "<attribute key=\"c++filt\" value=\"c++filt\"/>\n"
              "</extension>\n"
              ;
      }
    else
      {
      // *** Should never get here ***
      fout << "<error_toolchain_type/>\n";
      }
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
  std::string excludeFromOut;
  for (std::vector<std::string>::const_iterator
       it = this->SrcLinkedResources.begin();
       it != this->SrcLinkedResources.end();
       ++it)
    {
    fout << "<pathentry kind=\"src\" path=\"" << this->EscapeForXML(*it)
         << "\"/>\n";

    // exlude source directory from output search path
    // - only if not named the same as an output directory
    if (!cmSystemTools::FileIsDirectory(
           std::string(this->HomeOutputDirectory + "/" + *it).c_str()))
      {
      excludeFromOut += this->EscapeForXML(*it) + "/|";
      }
    }
  excludeFromOut += "**/CMakeFiles/";
  fout << "<pathentry excluding=\"" << excludeFromOut
       << "\" kind=\"out\" path=\"\"/>\n";
  // add output entry for EXECUTABLE_OUTPUT_PATH and LIBRARY_OUTPUT_PATH
  // - if it is a subdir of homeOutputDirectory, there is no need to add it
  // - if it is not then create a linked resource and add the linked name
  //   but check it doesn't conflict with other linked resources names
  for (std::vector<std::string>::const_iterator
       it = this->OutLinkedResources.begin();
       it != this->OutLinkedResources.end();
       ++it)
    {
    fout << "<pathentry kind=\"out\" path=\"" << this->EscapeForXML(*it)
         << "\"/>\n";
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
               << "\" path=\"\" value=\"" << this->EscapeForXML(val)
               << "\"/>\n";
          }
        }
      }
    }
  // add system defined c macros
  const char* cDefs=mf->GetDefinition(
                              "CMAKE_EXTRA_GENERATOR_C_SYSTEM_DEFINED_MACROS");
  if(cDefs)
    {
    // Expand the list.
    std::vector<std::string> defs;
    cmSystemTools::ExpandListArgument(cDefs, defs, true);

    // the list must contain only definition-value pairs:
    if ((defs.size() % 2) == 0)
      {
      std::vector<std::string>::const_iterator di = defs.begin();
      while (di != defs.end())
        {
        std::string def = *di;
        ++di;
        std::string val;
        if (di != defs.end())
          {
          val = *di;
          ++di;
          }

        // insert the definition if not already added.
        if(emmited.find(def) == emmited.end())
          {
          emmited.insert(def);
          fout << "<pathentry kind=\"mac\" name=\"" << def
               << "\" path=\"\" value=\"" << this->EscapeForXML(val)
               << "\"/>\n";
          }
        }
      }
    }
  // add system defined c++ macros
  const char* cxxDefs = mf->GetDefinition(
                            "CMAKE_EXTRA_GENERATOR_CXX_SYSTEM_DEFINED_MACROS");
  if(cxxDefs)
    {
    // Expand the list.
    std::vector<std::string> defs;
    cmSystemTools::ExpandListArgument(cxxDefs, defs, true);

    // the list must contain only definition-value pairs:
    if ((defs.size() % 2) == 0)
      {
      std::vector<std::string>::const_iterator di = defs.begin();
      while (di != defs.end())
        {
        std::string def = *di;
        ++di;
        std::string val;
        if (di != defs.end())
          {
          val = *di;
          ++di;
          }

        // insert the definition if not already added.
        if(emmited.find(def) == emmited.end())
          {
          emmited.insert(def);
          fout << "<pathentry kind=\"mac\" name=\"" << def
               << "\" path=\"\" value=\"" << this->EscapeForXML(val)
               << "\"/>\n";
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
    this->AppendIncludeDirectories(fout, includeDirs, emmited);
    }
  // now also the system include directories, in case we found them in
  // CMakeSystemSpecificInformation.cmake. This makes Eclipse find the
  // standard headers.
  std::string compiler = mf->GetSafeDefinition("CMAKE_C_COMPILER");
  if (!compiler.empty())
    {
    std::string systemIncludeDirs = mf->GetSafeDefinition(
                                "CMAKE_EXTRA_GENERATOR_C_SYSTEM_INCLUDE_DIRS");
    std::vector<std::string> dirs;
    cmSystemTools::ExpandListArgument(systemIncludeDirs.c_str(), dirs);
    this->AppendIncludeDirectories(fout, dirs, emmited);
    }
  compiler = mf->GetSafeDefinition("CMAKE_CXX_COMPILER");
  if (!compiler.empty())
    {
    std::string systemIncludeDirs = mf->GetSafeDefinition(
                              "CMAKE_EXTRA_GENERATOR_CXX_SYSTEM_INCLUDE_DIRS");
    std::vector<std::string> dirs;
    cmSystemTools::ExpandListArgument(systemIncludeDirs.c_str(), dirs);
    this->AppendIncludeDirectories(fout, dirs, emmited);
    }

  fout << "</storageModule>\n";

  // add build targets
  fout <<
    "<storageModule moduleId=\"org.eclipse.cdt.make.core.buildtargets\">\n"
    "<buildTargets>\n"
    ;
  emmited.clear();
  const std::string make = mf->GetRequiredDefinition("CMAKE_MAKE_PROGRAM");
  const std::string makeArgs = mf->GetSafeDefinition(
                                               "CMAKE_ECLIPSE_MAKE_ARGUMENTS");
  cmGlobalGenerator* generator
    = const_cast<cmGlobalGenerator*>(this->GlobalGenerator);

  std::string allTarget;
  std::string cleanTarget;
  if (generator->GetAllTargetName())
    {
    allTarget = generator->GetAllTargetName();
    }
  if (generator->GetCleanTargetName())
    {
    cleanTarget = generator->GetCleanTargetName();
    }

  // add all executable and library targets and some of the GLOBAL
  // and UTILITY targets
  for (std::vector<cmLocalGenerator*>::const_iterator
        it = this->GlobalGenerator->GetLocalGenerators().begin();
       it != this->GlobalGenerator->GetLocalGenerators().end();
       ++it)
    {
    const cmTargets& targets = (*it)->GetMakefile()->GetTargets();
    cmMakefile* makefile=(*it)->GetMakefile();
    std::string subdir = (*it)->Convert(makefile->GetCurrentOutputDirectory(),
                           cmLocalGenerator::HOME_OUTPUT);
    if (subdir == ".")
      {
      subdir = "";
      }

    for(cmTargets::const_iterator ti=targets.begin(); ti!=targets.end(); ++ti)
      {
      switch(ti->second.GetType())
        {
        case cmTarget::GLOBAL_TARGET:
          {
          bool insertTarget = false;
          // Only add the global targets from CMAKE_BINARY_DIR,
          // not from the subdirs
          if (subdir.empty())
           {
           insertTarget = true;
           // only add the "edit_cache" target if it's not ccmake, because
           // this will not work within the IDE
           if (ti->first == "edit_cache")
             {
             const char* editCommand = makefile->GetDefinition
                                                        ("CMAKE_EDIT_COMMAND");
             if (editCommand == 0)
               {
               insertTarget = false;
               }
             else if (strstr(editCommand, "ccmake")!=NULL)
               {
               insertTarget = false;
               }
             }
           }
         if (insertTarget)
           {
           this->AppendTarget(fout, ti->first, make, makeArgs, subdir, ": ");
           }
         }
         break;
       case cmTarget::UTILITY:
         // Add all utility targets, except the Nightly/Continuous/
         // Experimental-"sub"targets as e.g. NightlyStart
         if (((ti->first.find("Nightly")==0)   &&(ti->first!="Nightly"))
          || ((ti->first.find("Continuous")==0)&&(ti->first!="Continuous"))
          || ((ti->first.find("Experimental")==0)
                                            && (ti->first!="Experimental")))
           {
           break;
           }

         this->AppendTarget(fout, ti->first, make, makeArgs, subdir, ": ");
         break;
       case cmTarget::EXECUTABLE:
       case cmTarget::STATIC_LIBRARY:
       case cmTarget::SHARED_LIBRARY:
       case cmTarget::MODULE_LIBRARY:
         {
         const char* prefix = (ti->second.GetType()==cmTarget::EXECUTABLE ?
                                                          "[exe] " : "[lib] ");
         this->AppendTarget(fout, ti->first, make, makeArgs, subdir, prefix);
         std::string fastTarget = ti->first;
         fastTarget += "/fast";
         this->AppendTarget(fout, fastTarget, make, makeArgs, subdir, prefix);
         }
         break;
        // ignore these:
        case cmTarget::INSTALL_FILES:
        case cmTarget::INSTALL_PROGRAMS:
        case cmTarget::INSTALL_DIRECTORY:
        default:
          break;
        }
      }

    // insert the all and clean targets in every subdir
    if (!allTarget.empty())
      {
      this->AppendTarget(fout, allTarget, make, makeArgs, subdir, ": ");
      }
    if (!cleanTarget.empty())
      {
      this->AppendTarget(fout, cleanTarget, make, makeArgs, subdir, ": ");
      }

    //insert rules for compiling, preprocessing and assembling individual files
    cmLocalUnixMakefileGenerator3* lumg=(cmLocalUnixMakefileGenerator3*)*it;
    std::vector<std::string> objectFileTargets;
    lumg->GetIndividualFileTargets(objectFileTargets);
    for(std::vector<std::string>::const_iterator fit=objectFileTargets.begin();
        fit != objectFileTargets.end();
        ++fit)
      {
      const char* prefix = "[obj] ";
      if ((*fit)[fit->length()-1] == 's')
        {
        prefix = "[to asm] ";
        }
      else if ((*fit)[fit->length()-1] == 'i')
        {
        prefix = "[pre] ";
        }
      this->AppendTarget(fout, *fit, make, makeArgs, subdir, prefix);
      }
    }

  fout << "</buildTargets>\n"
          "</storageModule>\n"
          ;

  this->AppendStorageScanners(fout, *mf);

  fout << "</cconfiguration>\n"
          "</storageModule>\n"
          "<storageModule moduleId=\"cdtBuildSystem\" version=\"4.0.0\">\n"
          "<project id=\"" << this->EscapeForXML(mf->GetProjectName())
       << ".null.1\" name=\"" << this->EscapeForXML(mf->GetProjectName())
       << "\"/>\n"
          "</storageModule>\n"
          "</cproject>\n"
          ;
}

//----------------------------------------------------------------------------
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

std::string
cmExtraEclipseCDT4Generator::GenerateProjectName(const std::string& name,
                                                 const std::string& type,
                                                 const std::string& path)
{
  return cmExtraEclipseCDT4Generator::EscapeForXML(name)
                                +(type.empty() ? "" : "-") + type + "@" + path;
}

std::string cmExtraEclipseCDT4Generator::EscapeForXML(const std::string& value)
{
  std::string str = value;
  cmSystemTools::ReplaceString(str, "&", "&amp;");
  cmSystemTools::ReplaceString(str, "<", "&lt;");
  cmSystemTools::ReplaceString(str, ">", "&gt;");
  cmSystemTools::ReplaceString(str, "\"", "&quot;");
  // NOTE: This one is not necessary, since as of Eclipse CDT4 it will
  //       automatically change this to the original value (').
  //cmSystemTools::ReplaceString(str, "'", "&apos;");
  return str;
}

//----------------------------------------------------------------------------
// Helper functions
//----------------------------------------------------------------------------
void cmExtraEclipseCDT4Generator
::AppendStorageScanners(cmGeneratedFileStream& fout,
                        const cmMakefile& makefile)
{
  // we need the "make" and the C (or C++) compiler which are used, Alex
  std::string make = makefile.GetRequiredDefinition("CMAKE_MAKE_PROGRAM");
  std::string compiler = makefile.GetSafeDefinition("CMAKE_C_COMPILER");
  if (compiler.empty())
    {
    compiler = makefile.GetSafeDefinition("CMAKE_CXX_COMPILER");
    }
  if (compiler.empty())  //Hmm, what to do now ?
    {
    compiler = "gcc";
    }


  // the following right now hardcodes gcc behaviour :-/
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
    compiler, true, true);
  cmExtraEclipseCDT4Generator::AppendScannerProfile(fout,
    "org.eclipse.cdt.make.core.GCCStandardMakePerFileProfile",
    true, "", true, "makefileGenerator",
    "-f ${project_name}_scd.mk",
    make, true, true);

  fout << "</storageModule>\n";
}

// The prefix is prepended before the actual name of the target. The purpose
// of that is to sort the targets in the view of Eclipse, so that at first
// the global/utility/all/clean targets appear ": ", then the executable
// targets "[exe] ", then the libraries "[lib]", then the rules for the
// object files "[obj]", then for preprocessing only "[pre] " and
// finally the assembly files "[to asm] ". Note the "to" in "to asm",
// without it, "asm" would be the first targets in the list, with the "to"
// they are the last targets, which makes more sense.
void cmExtraEclipseCDT4Generator::AppendTarget(cmGeneratedFileStream& fout,
                                               const std::string&     target,
                                               const std::string&     make,
                                               const std::string&     makeArgs,
                                               const std::string&     path,
                                               const char* prefix)
{
  std::string targetXml = cmExtraEclipseCDT4Generator::EscapeForXML(target);
  std::string pathXml = cmExtraEclipseCDT4Generator::EscapeForXML(path);
  fout <<
    "<target name=\"" << prefix << targetXml << "\""
    " path=\"" << pathXml.c_str() << "\""
    " targetID=\"org.eclipse.cdt.make.MakeTargetBuilder\">\n"
    "<buildCommand>"
    << cmExtraEclipseCDT4Generator::GetEclipsePath(make)
    << "</buildCommand>\n"
    "<buildArguments>"  << makeArgs << "</buildArguments>\n"
    "<buildTarget>" << targetXml << "</buildTarget>\n"
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

void cmExtraEclipseCDT4Generator
::AppendLinkedResource (cmGeneratedFileStream& fout,
                        const std::string&     name,
                        const std::string&     path,
                        bool isVirtualFolder)
{
  const char* locationTag = "location";
  if (isVirtualFolder) // ... and not a linked folder
    {
    locationTag = "locationURI";
    }

  fout <<
    "\t\t<link>\n"
    "\t\t\t<name>"
    << cmExtraEclipseCDT4Generator::EscapeForXML(name)
    << "</name>\n"
    "\t\t\t<type>2</type>\n"
    "\t\t\t<" << locationTag << ">"
    << cmExtraEclipseCDT4Generator::EscapeForXML(path)
    << "</" << locationTag << ">\n"
    "\t\t</link>\n"
    ;
}

bool cmExtraEclipseCDT4Generator
::AppendOutLinkedResource(cmGeneratedFileStream& fout,
                          const std::string&     defname,
                          const std::string&     altdefname)
{
  if (defname.empty() && altdefname.empty())
    {
    return false;
    }

  std::string outputPath = (defname.empty() ? altdefname : defname);

  if (!cmSystemTools::FileIsFullPath(outputPath.c_str()))
    {
    outputPath = this->HomeOutputDirectory + "/" + outputPath;
    }
  if (cmSystemTools::IsSubDirectory(outputPath.c_str(),
                                    this->HomeOutputDirectory.c_str()))
    {
    return false;
    }

  std::string name = this->GetPathBasename(outputPath);

  // make sure linked resource name is unique
  while (this->GlobalGenerator->GetProjectMap().find(name)
      != this->GlobalGenerator->GetProjectMap().end())
    {
    name += "_";
    }

  if (std::find(this->OutLinkedResources.begin(),
                this->OutLinkedResources.end(),
                name)
      != this->OutLinkedResources.end())
    {
    return false;
    }
  else
    {
    this->AppendLinkedResource(fout, name,
                               this->GetEclipsePath(outputPath));
    this->OutLinkedResources.push_back(name);
    return true;
    }
}

