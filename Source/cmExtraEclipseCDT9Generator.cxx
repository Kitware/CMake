/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmExtraEclipseCDT9Generator.h"

#include "cmsys/RegularExpression.hxx"
#include <algorithm>
#include <assert.h>
#include <sstream>
#include <stdio.h>
#include <utility>

#include "cmGeneratedFileStream.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmSourceGroup.h"
#include "cmState.h"
#include "cmStateTypes.h"
#include "cmSystemTools.h"
#include "cmXMLWriter.h"
#include "cmake.h"

static void AppendAttribute(cmXMLWriter& xml, const char* keyval)
{
  xml.StartElement("attribute");
  xml.Attribute("key", keyval);
  xml.Attribute("value", keyval);
  xml.EndElement();
}

template <typename T>
void AppendDictionary(cmXMLWriter& xml, const char* key, T const& value)
{
  xml.StartElement("dictionary");
  xml.Element("key", key);
  xml.Element("value", value);
  xml.EndElement();
}

cmExtraEclipseCDT9Generator::cmExtraEclipseCDT9Generator()
  : cmExternalMakefileProjectGenerator()
{
  this->SupportsVirtualFolders = true;
  this->GenerateLinkedResources = true;
  this->SupportsGmakeErrorParser = true;
  this->SupportsMachO64Parser = true;
  this->CEnabled = false;
  this->CXXEnabled = false;
}

cmExternalMakefileProjectGeneratorFactory*
cmExtraEclipseCDT9Generator::GetFactory()
{
  static cmExternalMakefileProjectGeneratorSimpleFactory<
    cmExtraEclipseCDT9Generator>
    factory("Eclipse CDT9", "Generates Eclipse CDT 9.0 project files.");

  if (factory.GetSupportedGlobalGenerators().empty()) {
// TODO: Verify if __CYGWIN__ should be checked.
//#if defined(_WIN32) && !defined(__CYGWIN__)
#if defined(_WIN32)
    factory.AddSupportedGlobalGenerator("NMake Makefiles");
    factory.AddSupportedGlobalGenerator("MinGW Makefiles");
// factory.AddSupportedGlobalGenerator("MSYS Makefiles");
#endif
    factory.AddSupportedGlobalGenerator("Ninja");
    factory.AddSupportedGlobalGenerator("Unix Makefiles");
  }

  return &factory;
}

void cmExtraEclipseCDT9Generator::EnableLanguage(
  std::vector<std::string> const& languages, cmMakefile* /*unused*/,
  bool /*optional*/)
{
  for (std::string const& l : languages) {
    if (l == "CXX") {
      this->Natures.insert("org.eclipse.cdt.core.ccnature");
      this->Natures.insert("org.eclipse.cdt.core.cnature");
      this->CXXEnabled = true;
    } else if (l == "C") {
      this->Natures.insert("org.eclipse.cdt.core.cnature");
      this->CEnabled = true;
    } else if (l == "Java") {
      this->Natures.insert("org.eclipse.jdt.core.javanature");
    }
  }
}

void cmExtraEclipseCDT9Generator::Generate()
{
  cmLocalGenerator* lg = this->GlobalGenerator->GetLocalGenerators()[0];
  const cmMakefile* mf = lg->GetMakefile();

  std::string eclipseVersion = mf->GetSafeDefinition("CMAKE_ECLIPSE_VERSION");
  cmsys::RegularExpression regex(".*([0-9]+\\.[0-9]+).*");
  if (regex.find(eclipseVersion)) {
    unsigned int majorVersion = 0;
    unsigned int minorVersion = 0;
    int res =
      sscanf(regex.match(1).c_str(), "%u.%u", &majorVersion, &minorVersion);
    if (res == 2) {
      int version = majorVersion * 1000 + minorVersion;
      if (version < 3006) // 3.6 is Helios
      {
        this->SupportsVirtualFolders = false;
        this->SupportsMachO64Parser = false;
      }
      if (version < 3007) // 3.7 is Indigo
      {
        this->SupportsGmakeErrorParser = false;
      }
    }
  }

  // TODO: Decide if these are local or member variables
  this->HomeDirectory = lg->GetSourceDirectory();
  this->HomeOutputDirectory = lg->GetBinaryDirectory();

  this->GenerateLinkedResources =
    mf->IsOn("CMAKE_ECLIPSE_GENERATE_LINKED_RESOURCES");

  this->IsOutOfSourceBuild =
    (this->HomeDirectory != this->HomeOutputDirectory);

  this->GenerateSourceProject =
    (this->IsOutOfSourceBuild &&
     mf->IsOn("CMAKE_ECLIPSE_GENERATE_SOURCE_PROJECT"));

  if (!this->GenerateSourceProject &&
      (mf->IsOn("ECLIPSE_CDT4_GENERATE_SOURCE_PROJECT"))) {
    mf->IssueMessage(
      cmake::WARNING,
      "ECLIPSE_CDT4_GENERATE_SOURCE_PROJECT is set to TRUE, "
      "but this variable is not supported anymore since CMake 2.8.7.\n"
      "Enable CMAKE_ECLIPSE_GENERATE_SOURCE_PROJECT instead.");
  }

  if (cmSystemTools::IsSubDirectory(this->HomeOutputDirectory,
                                    this->HomeDirectory)) {
    mf->IssueMessage(cmake::WARNING,
                     "The build directory is a subdirectory "
                     "of the source directory.\n"
                     "This is not supported well by Eclipse. It is strongly "
                     "recommended to use a build directory which is a "
                     "sibling of the source directory.");
  }

  // NOTE: This is not good, since it pollutes the source tree. However,
  //       Eclipse doesn't allow CVS/SVN to work when the .project is not in
  //       the cvs/svn root directory. Hence, this is provided as an option.
  if (this->GenerateSourceProject) {
    // create .project file in the source tree
    this->CreateSourceProjectFile();
  }

  // create a .project file
  this->CreateProjectFile();

  // create a .cproject file
  this->CreateCProjectFile();
}

void cmExtraEclipseCDT9Generator::CreateSourceProjectFile()
{
  assert(this->HomeDirectory != this->HomeOutputDirectory);

  // set up the project name: <project>-Source@<baseSourcePathName>
  cmLocalGenerator* lg = this->GlobalGenerator->GetLocalGenerators()[0];
  std::string name =
    this->GenerateProjectName(lg->GetProjectName(), "Source",
                              this->GetPathBasename(this->HomeDirectory));

  const std::string filename = this->HomeDirectory + "/.project";
  cmGeneratedFileStream fout(filename);
  if (!fout) {
    return;
  }

  cmXMLWriter xml(fout);
  xml.StartDocument("UTF-8");
  xml.StartElement("projectDescription");
  xml.Element("name", name);
  xml.Element("comment", "");
  xml.Element("projects", "");
  xml.Element("buildSpec", "");
  xml.Element("natures", "");
  xml.StartElement("linkedResources");

  if (this->SupportsVirtualFolders) {
    this->CreateLinksToSubprojects(xml, this->HomeDirectory);
    this->SrcLinkedResources.clear();
  }

  xml.EndElement(); // linkedResources
  xml.EndElement(); // projectDescription
  xml.EndDocument();
}

void cmExtraEclipseCDT9Generator::AddEnvVar(std::ostream& out,
                                            const char* envVar,
                                            cmLocalGenerator* lg)
{
  cmMakefile* mf = lg->GetMakefile();

  // get the variables from the environment and from the cache and then
  // figure out which one to use:

  std::string envVarValue;
  const bool envVarSet = cmSystemTools::GetEnv(envVar, envVarValue);

  std::string cacheEntryName = "CMAKE_ECLIPSE_ENVVAR_";
  cacheEntryName += envVar;
  const std::string* cacheValue =
    lg->GetState()->GetInitializedCacheValue(cacheEntryName);

  // now we have both, decide which one to use
  std::string valueToUse;
  if (!envVarSet && cacheValue == nullptr) {
    // nothing known, do nothing
    valueToUse.clear();
  } else if (envVarSet && cacheValue == nullptr) {
    // The variable is in the env, but not in the cache. Use it and put it
    // in the cache
    valueToUse = envVarValue;
    mf->AddCacheDefinition(cacheEntryName, valueToUse.c_str(),
                           cacheEntryName.c_str(), cmStateEnums::STRING, true);
    mf->GetCMakeInstance()->SaveCache(lg->GetBinaryDirectory());
  } else if (!envVarSet && cacheValue != nullptr) {
    // It is already in the cache, but not in the env, so use it from the cache
    valueToUse = *cacheValue;
  } else {
    // It is both in the cache and in the env.
    // Use the version from the env. except if the value from the env is
    // completely contained in the value from the cache (for the case that we
    // now have a PATH without MSVC dirs in the env. but had the full PATH with
    // all MSVC dirs during the cmake run which stored the var in the cache:
    valueToUse = *cacheValue;
    if (valueToUse.find(envVarValue) == std::string::npos) {
      valueToUse = envVarValue;
      mf->AddCacheDefinition(cacheEntryName, valueToUse.c_str(),
                             cacheEntryName.c_str(), cmStateEnums::STRING,
                             true);
      mf->GetCMakeInstance()->SaveCache(lg->GetBinaryDirectory());
    }
  }

  if (!valueToUse.empty()) {
    out << envVar << "=" << valueToUse << "|";
  }
}

void cmExtraEclipseCDT9Generator::CreateProjectFile()
{
  cmLocalGenerator* lg = this->GlobalGenerator->GetLocalGenerators()[0];
  cmMakefile* mf = lg->GetMakefile();

  const std::string filename = this->HomeOutputDirectory + "/.project";

  cmGeneratedFileStream fout(filename);
  if (!fout) {
    return;
  }

  std::string compilerId = mf->GetSafeDefinition("CMAKE_C_COMPILER_ID");
  if (compilerId.empty()) // no C compiler, try the C++ compiler:
  {
    compilerId = mf->GetSafeDefinition("CMAKE_CXX_COMPILER_ID");
  }

  cmXMLWriter xml(fout);

  xml.StartDocument("UTF-8");
  xml.StartElement("projectDescription");

  xml.Element("name",
              this->GenerateProjectName(
                lg->GetProjectName(),
                mf->GetSafeDefinition("CMAKE_BUILD_TYPE"),
                this->GetPathBasename(this->HomeOutputDirectory)));

  xml.Element("comment", "");
  xml.Element("projects", "");

  xml.StartElement("buildSpec");
  xml.StartElement("buildCommand");
  xml.Element("name", "org.eclipse.cdt.managedbuilder.core.genmakebuilder");
  xml.Element("triggers", "clean,full,incremental,");
  xml.StartElement("arguments");
  xml.EndElement(); // arguments
  xml.EndElement(); // buildCommand
  xml.StartElement("buildCommand");
  xml.Element("name", "org.eclipse.cdt.make.core.ScannerConfigBuilder");
  xml.StartElement("arguments");
  xml.EndElement(); // arguments
  xml.EndElement(); // buildCommand
  xml.EndElement(); // buildSpec

  // set natures for c/c++ projects
  xml.StartElement("natures");
  xml.Element("nature",
          "org.eclipse.cdt.managedbuilder.core.managedBuildNature");
  xml.Element("nature",
          "org.eclipse.cdt.managedbuilder.core.ScannerConfigNature");

  for (std::string const& n : this->Natures) {
    xml.Element("nature", n);
  }

  if (const char* extraNaturesProp =
          mf->GetState()->GetGlobalProperty("ECLIPSE_EXTRA_NATURES")) {
    std::vector<std::string> extraNatures;
    cmSystemTools::ExpandListArgument(extraNaturesProp, extraNatures);
    for (std::string const& n : extraNatures) {
      xml.Element("nature", n);
    }
  }

  xml.EndElement(); // natures

  xml.StartElement("linkedResources");
  // create linked resources
  if (this->IsOutOfSourceBuild) {
    // create a linked resource to CMAKE_SOURCE_DIR
    // (this is not done anymore for each project because of
    // https://gitlab.kitware.com/cmake/cmake/issues/9978 and because I found
    // it actually quite confusing in bigger projects with many directories and
    // projects, Alex

    std::string sourceLinkedResourceName = "[Source directory]";
    std::string linkSourceDirectory =
      this->GetEclipsePath(lg->GetCurrentSourceDirectory());
    // .project dir can't be subdir of a linked resource dir
    if (!cmSystemTools::IsSubDirectory(this->HomeOutputDirectory,
                                       linkSourceDirectory)) {
      this->AppendLinkedResource(xml, sourceLinkedResourceName,
                                 this->GetEclipsePath(linkSourceDirectory),
                                 LinkToFolder);
      this->SrcLinkedResources.push_back(std::move(sourceLinkedResourceName));
    }
  }

  if (this->SupportsVirtualFolders) {
    this->CreateLinksToSubprojects(xml, this->HomeOutputDirectory);

    this->CreateLinksForTargets(xml);
  }

  xml.EndElement(); // linkedResources
  xml.EndElement(); // projectDescription
}

void cmExtraEclipseCDT9Generator::WriteGroups(
  std::vector<cmSourceGroup> const& sourceGroups, std::string& linkName,
  cmXMLWriter& xml)
{
  for (cmSourceGroup const& sg : sourceGroups) {
    std::string linkName3 = linkName;
    linkName3 += "/";
    linkName3 += sg.GetFullName();

    std::replace(linkName3.begin(), linkName3.end(), '\\', '/');

    this->AppendLinkedResource(xml, linkName3, "virtual:/virtual",
                               VirtualFolder);
    std::vector<cmSourceGroup> const& children = sg.GetGroupChildren();
    if (!children.empty()) {
      this->WriteGroups(children, linkName, xml);
    }
    std::vector<const cmSourceFile*> sFiles = sg.GetSourceFiles();
    for (cmSourceFile const* file : sFiles) {
      std::string const& fullPath = file->GetFullPath();

      if (!cmSystemTools::FileIsDirectory(fullPath)) {
        std::string linkName4 = linkName3;
        linkName4 += "/";
        linkName4 += cmSystemTools::GetFilenameName(fullPath);
        this->AppendLinkedResource(xml, linkName4,
                                   this->GetEclipsePath(fullPath), LinkToFile);
      }
    }
  }
}

void cmExtraEclipseCDT9Generator::CreateLinksForTargets(cmXMLWriter& xml)
{
  std::string linkName = "[Targets]";
  this->AppendLinkedResource(xml, linkName, "virtual:/virtual", VirtualFolder);

  for (cmLocalGenerator* lg : this->GlobalGenerator->GetLocalGenerators()) {
    cmMakefile* makefile = lg->GetMakefile();
    const std::vector<cmGeneratorTarget*>& targets = lg->GetGeneratorTargets();

    for (cmGeneratorTarget* target : targets) {
      std::string linkName2 = linkName;
      linkName2 += "/";
      switch (target->GetType()) {
        case cmStateEnums::EXECUTABLE:
        case cmStateEnums::STATIC_LIBRARY:
        case cmStateEnums::SHARED_LIBRARY:
        case cmStateEnums::MODULE_LIBRARY:
        case cmStateEnums::OBJECT_LIBRARY: {
          const char* prefix =
            (target->GetType() == cmStateEnums::EXECUTABLE ? "[exe] "
                                                           : "[lib] ");
          linkName2 += prefix;
          linkName2 += target->GetName();
          this->AppendLinkedResource(xml, linkName2, "virtual:/virtual",
                                     VirtualFolder);
          if (!this->GenerateLinkedResources) {
            break; // skip generating the linked resources to the source files
          }
          std::vector<cmSourceGroup> sourceGroups =
            makefile->GetSourceGroups();
          // get the files from the source lists then add them to the groups
          cmGeneratorTarget* gt = const_cast<cmGeneratorTarget*>(target);
          std::vector<cmSourceFile*> files;
          gt->GetSourceFiles(files,
                             makefile->GetSafeDefinition("CMAKE_BUILD_TYPE"));
          for (cmSourceFile* sf : files) {
            // Add the file to the list of sources.
            std::string const& source = sf->GetFullPath();
            cmSourceGroup* sourceGroup =
              makefile->FindSourceGroup(source, sourceGroups);
            sourceGroup->AssignSource(sf);
          }

          this->WriteGroups(sourceGroups, linkName2, xml);
        } break;
        // ignore all others:
        default:
          break;
      }
    }
  }
}

void cmExtraEclipseCDT9Generator::CreateLinksToSubprojects(
  cmXMLWriter& xml, const std::string& baseDir)
{
  if (!this->GenerateLinkedResources) {
    return;
  }

  // for each sub project create a linked resource to the source dir
  // - only if it is an out-of-source build
  this->AppendLinkedResource(xml, "[Subprojects]", "virtual:/virtual",
                             VirtualFolder);

  for (auto const& it : this->GlobalGenerator->GetProjectMap()) {
    std::string linkSourceDirectory =
      this->GetEclipsePath(it.second[0]->GetCurrentSourceDirectory());
    // a linked resource must not point to a parent directory of .project or
    // .project itself
    if ((baseDir != linkSourceDirectory) &&
        !cmSystemTools::IsSubDirectory(baseDir, linkSourceDirectory)) {
      std::string linkName = "[Subprojects]/";
      linkName += it.first;
      this->AppendLinkedResource(xml, linkName,
                                 this->GetEclipsePath(linkSourceDirectory),
                                 LinkToFolder);
      // Don't add it to the srcLinkedResources, because listing multiple
      // directories confuses the Eclipse indexer (#13596).
    }
  }
}

void cmExtraEclipseCDT9Generator::AppendIncludeDirectories(
  cmXMLWriter& xml, const std::vector<std::string>& includeDirs,
  std::set<std::string>& emittedDirs)
{
  for (std::string const& inc : includeDirs) {
    if (!inc.empty()) {
      std::string dir = cmSystemTools::CollapseFullPath(inc);

      // handle framework include dirs on OSX, the remainder after the
      // Frameworks/ part has to be stripped
      //   /System/Library/Frameworks/GLUT.framework/Headers
      cmsys::RegularExpression frameworkRx("(.+/Frameworks)/.+\\.framework/");
      if (frameworkRx.find(dir.c_str())) {
        dir = frameworkRx.match(1);
      }

      if (emittedDirs.find(dir) == emittedDirs.end()) {
        emittedDirs.insert(dir);
        xml.StartElement("pathentry");
        xml.Attribute("include",
                      cmExtraEclipseCDT9Generator::GetEclipsePath(dir));
        xml.Attribute("kind", "inc");
        xml.Attribute("path", "");
        xml.Attribute("system", "true");
        xml.EndElement();
      }
    }
  }
}

void cmExtraEclipseCDT9Generator::AppendIncludeDirectories_CDT9(
  cmXMLWriter& xml, const std::vector<std::string>& includeDirs,
  std::set<std::string>& emittedDirs)
{
  for (std::string const& inc : includeDirs) {
    if (!inc.empty()) {
      std::string dir = cmSystemTools::CollapseFullPath(inc);

      // handle framework include dirs on OSX, the remainder after the
      // Frameworks/ part has to be stripped
      //   /System/Library/Frameworks/GLUT.framework/Headers
      cmsys::RegularExpression frameworkRx("(.+/Frameworks)/.+\\.framework/");
      if (frameworkRx.find(dir.c_str())) {
        dir = frameworkRx.match(1);
      }

      if (emittedDirs.find(dir) == emittedDirs.end()) {
        emittedDirs.insert(dir);
        xml.StartElement("listOptionValue");
        xml.Attribute("builtIn", "false");
        xml.Attribute("value",
                cmExtraEclipseCDT9Generator::GetEclipsePath(dir));
        xml.EndElement();
      }
    }
  }
}
void cmExtraEclipseCDT9Generator::CreateCProjectFile() const
{
  std::set<std::string> emmited;

  cmLocalGenerator* lg = this->GlobalGenerator->GetLocalGenerators()[0];
  const cmMakefile* mf = lg->GetMakefile();

  const std::string filename = this->HomeOutputDirectory + "/.cproject";

  cmGeneratedFileStream fout(filename);
  if (!fout) {
    return;
  }

  cmXMLWriter xml(fout);

  // add header
  xml.StartDocument("UTF-8");
  xml.ProcessingInstruction("fileVersion", "4.0.0");
  xml.StartElement("cproject");
  xml.Attribute("storage_type_id",
                "org.eclipse.cdt.core.XmlProjectDescriptionStorage");
  xml.StartElement("storageModule");
  xml.Attribute("moduleId", "org.eclipse.cdt.core.settings");

  xml.StartElement("cconfiguration");
  xml.Attribute("id", "cdt.managedbuild.toolchain.gnu.base.1802459151");

  // Configuration settings...
  xml.StartElement("storageModule");
  xml.Attribute("buildSystemId",
            "org.eclipse.cdt.managedbuilder.core.configurationDataProvider");
  xml.Attribute("id", "cdt.managedbuild.toolchain.gnu.base.1802459151");
  xml.Attribute("moduleId", "org.eclipse.cdt.core.settings");
  xml.Attribute("name", "Configuration");
  xml.Element("externalSettings");
  xml.StartElement("extensions");

  // TODO: refactor this out...
  std::string executableFormat =
    mf->GetSafeDefinition("CMAKE_EXECUTABLE_FORMAT");
  if (executableFormat == "ELF") {
    xml.StartElement("extension");
    xml.Attribute("id", "org.eclipse.cdt.core.GNU_ELF");
    xml.Attribute("point", "org.eclipse.cdt.core.BinaryParser");
    xml.EndElement(); // extension

    xml.StartElement("extension");
    xml.Attribute("id", "org.eclipse.cdt.core.GASErrorParser");
    xml.Attribute("point", "org.eclipse.cdt.core.ErrorParser");
    xml.EndElement(); // extension

    xml.StartElement("extension");
    xml.Attribute("id", "org.eclipse.cdt.core.GmakeErrorParser");
    xml.Attribute("point", "org.eclipse.cdt.core.ErrorParser");
    xml.EndElement(); // extension

    xml.StartElement("extension");
    xml.Attribute("id", "org.eclipse.cdt.core.GLDErrorParser");
    xml.Attribute("point", "org.eclipse.cdt.core.ErrorParser");
    xml.EndElement(); // extension

    xml.StartElement("extension");
    xml.Attribute("id", "org.eclipse.cdt.core.CWDLocator");
    xml.Attribute("point", "org.eclipse.cdt.core.ErrorParser");
    xml.EndElement(); // extension

    xml.StartElement("extension");
    xml.Attribute("id", "org.eclipse.cdt.core.GCCErrorParser");
    xml.Attribute("point", "org.eclipse.cdt.core.ErrorParser");
    xml.EndElement(); // extension
  } else {
    std::string systemName = mf->GetSafeDefinition("CMAKE_SYSTEM_NAME");
    if (systemName == "CYGWIN") {
      xml.StartElement("extension");
      xml.Attribute("id", "org.eclipse.cdt.core.Cygwin_PE");
      xml.Attribute("point", "org.eclipse.cdt.core.BinaryParser");
      AppendAttribute(xml, "addr2line");
      AppendAttribute(xml, "c++filt");
      AppendAttribute(xml, "cygpath");
      AppendAttribute(xml, "nm");
      xml.EndElement(); // extension
    } else if (systemName == "Windows") {
      xml.StartElement("extension");
      xml.Attribute("id", "org.eclipse.cdt.core.PE");
      xml.Attribute("point", "org.eclipse.cdt.core.BinaryParser");
      xml.EndElement(); // extension
    } else if (systemName == "Darwin") {
      xml.StartElement("extension");
      xml.Attribute("id",
                    this->SupportsMachO64Parser
                      ? "org.eclipse.cdt.core.MachO64"
                      : "org.eclipse.cdt.core.MachO");
      xml.Attribute("point", "org.eclipse.cdt.core.BinaryParser");
      AppendAttribute(xml, "c++filt");
      xml.EndElement(); // extension
    } else {
      // *** Should never get here ***
      xml.Element("error_toolchain_type");
    }
  }

  xml.EndElement(); // extensions
  xml.EndElement(); // storageModule

  // ???
//   xml.StartElement("storageModule");
//   xml.Attribute("moduleId", "org.eclipse.cdt.core.language.mapping");
//   xml.Element("project-mappings");
//   xml.EndElement(); // storageModule

  // ???
  xml.StartElement("storageModule");
  xml.Attribute("moduleId", "cdtBuildSystem");
  xml.Attribute("version", "4.0.0");

    xml.StartElement("configuration");
    xml.Attribute("artifactName", "${ProjName}");
    xml.Attribute("buildProperties", "");
    xml.Attribute("description", "");
    xml.Attribute("id", "cdt.managedbuild.toolchain.gnu.base.1802459151");
    xml.Attribute("name", "Default");
    xml.Attribute("optionalBuildProperties", "");
    xml.Attribute("parent", "org.eclipse.cdt.build.core.emptycfg");

      xml.StartElement("folderInfo");
      xml.Attribute("id",
              "cdt.managedbuild.toolchain.gnu.base.1802459151.1444882859");
      xml.Attribute("name", "/");
      xml.Attribute("resourcePath", "");

        xml.StartElement("toolChain");
        xml.Attribute("id", "cdt.managedbuild.toolchain.gnu.base.238819452");
        xml.Attribute("name", "Linux GCC");
        xml.Attribute("superClass", "cdt.managedbuild.toolchain.gnu.base");

          xml.StartElement("targetPlatform");
          xml.Attribute("archList", "all");
          xml.Attribute("binaryParser", "org.eclipse.cdt.core.GNU_ELF");
          xml.Attribute("id",
                  "cdt.managedbuild.target.gnu.platform.base.1869015533");
          xml.Attribute("name", "Debug Platform");
          xml.Attribute("osList", "linux,hpux,aix,qnx");
          xml.Attribute("superClass",
                  "cdt.managedbuild.target.gnu.platform.base");
          xml.EndElement(); // targetPlatform

          xml.StartElement("builder");

          const std::string& make =
                  mf->GetSafeDefinition("CMAKE_ECLIPSE_MAKE_ARGUMENTS");
          const std::string& makeArgs =
                  mf->GetRequiredDefinition("CMAKE_MAKE_PROGRAM");
          xml.Attribute("arguments", make);
          xml.Attribute("command", makeArgs);
          xml.Attribute("id",
                  "cdt.managedbuild.target.gnu.builder.base.1983775073");
          xml.Attribute("keepEnvironmentInBuildfile", "false");
          xml.Attribute("managedBuildOn", "false");
          xml.Attribute("name", "Gnu Make Builder");
          xml.Attribute("superClass",
                  "cdt.managedbuild.target.gnu.builder.base");
          xml.EndElement(); // builder

          xml.StartElement("tool");
          xml.Attribute("id",
                  "cdt.managedbuild.tool.gnu.archiver.base.264314146");
          xml.Attribute("name", "GCC Archiver");
          xml.Attribute("superClass",
                  "cdt.managedbuild.tool.gnu.archiver.base");
          xml.EndElement(); // tool

          xml.StartElement("tool");
          xml.Attribute("id",
                  "cdt.managedbuild.tool.gnu.cpp.compiler.base.987241452");
          xml.Attribute("name",
                  "GCC C++ Compiler");
          xml.Attribute("superClass",
                  "cdt.managedbuild.tool.gnu.cpp.compiler.base");

            // Include path
            xml.StartElement("option");
            xml.Attribute("IS_BUILTIN_EMPTY", "false");
            xml.Attribute("IS_VALUE_EMPTY", "false");
            xml.Attribute("id",
                    "gnu.cpp.compiler.option.include.paths.1525486168");
            xml.Attribute("name",
                    "Include paths (-I)");
            xml.Attribute("superClass",
                    "gnu.cpp.compiler.option.include.paths");
            xml.Attribute("valueType", "includePath");

            for (cmLocalGenerator* lgen :
                    this->GlobalGenerator->GetLocalGenerators()) {
              const std::vector<cmGeneratorTarget*>& targets =
                lgen->GetGeneratorTargets();
              for (cmGeneratorTarget* target : targets) {
                std::vector<std::string> includeDirs;
                std::string config = mf->GetSafeDefinition("CMAKE_BUILD_TYPE");
                lgen->GetIncludeDirectories(includeDirs, target, "C", config);
                this->AppendIncludeDirectories_CDT9(xml, includeDirs, emmited);
              }
            }

            xml.EndElement(); // option

            // Define
            xml.StartElement("option");
            xml.Attribute("IS_BUILTIN_EMPTY", "false");
            xml.Attribute("IS_VALUE_EMPTY", "false");
            xml.Attribute("id",
                    "gnu.cpp.compiler.option.preprocessor.def.1541727279");
            xml.Attribute("name", "Defined symbols (-D)");
            xml.Attribute("superClass",
                    "gnu.cpp.compiler.option.preprocessor.def");
            xml.Attribute("valueType", "definedSymbols");

              emmited.clear();
              for (cmLocalGenerator* lgen :
                      this->GlobalGenerator->GetLocalGenerators()) {

                if (const char* cdefs =
                    lgen->GetMakefile()->GetProperty("COMPILE_DEFINITIONS")) {
                  // Expand the list.
                  std::vector<std::string> defs;
                  cmGeneratorExpression::Split(cdefs, defs);

                  for (std::string const& d : defs) {
                    if (cmGeneratorExpression::Find(d) != std::string::npos) {
                      continue;
                    }

                    std::string::size_type equals = d.find('=', 0);
                    std::string::size_type enddef = d.length();

                    std::string def;
                    std::string val;
                    if (equals != std::string::npos && equals < enddef) {
                      // we have -DFOO=BAR
                      def = d.substr(0, equals);
                      val = d.substr(equals + 1, enddef - equals + 1);
                    } else {
                      // we have -DFOO
                      def = d;
                    }

                    // insert the definition if not already added.
                    if (emmited.find(def) == emmited.end()) {
                      emmited.insert(def);
                      xml.StartElement("listOptionValue");
                      xml.Attribute("builtIn", "false");
                      if (val.empty()) {
                          xml.Attribute("value", def);
                      } else {
                          xml.Attribute("value", def + "=" + val);
                      }
                      xml.EndElement();
                    }
                  }
                }
              }
              // add system defined c macros
              const char* cDefs = mf->GetDefinition
                      ("CMAKE_EXTRA_GENERATOR_C_SYSTEM_DEFINED_MACROS");
              if (this->CEnabled && cDefs) {
                // Expand the list.
                std::vector<std::string> defs;
                cmSystemTools::ExpandListArgument(cDefs, defs, true);

                // the list must contain only definition-value pairs:
                if ((defs.size() % 2) == 0) {
                  std::vector<std::string>::const_iterator di = defs.begin();
                  while (di != defs.end()) {
                    std::string def = *di;
                    ++di;
                    std::string val;
                    if (di != defs.end()) {
                      val = *di;
                      ++di;
                    }

                    // insert the definition if not already added.
                    if (emmited.find(def) == emmited.end()) {
                      emmited.insert(def);
                      xml.StartElement("listOptionValue");
                      xml.Attribute("builtIn", "false");
                      if (val.empty()) {
                          xml.Attribute("value", def);
                      } else {
                          xml.Attribute("value", def + "=" + val);
                      }
                      xml.EndElement();
                    }
                  }
                }
              }
              // add system defined c++ macros
              const char* cxxDefs =
                mf->GetDefinition
                      ("CMAKE_EXTRA_GENERATOR_CXX_SYSTEM_DEFINED_MACROS");
              if (this->CXXEnabled && cxxDefs) {
                // Expand the list.
                std::vector<std::string> defs;
                cmSystemTools::ExpandListArgument(cxxDefs, defs, true);

                // the list must contain only definition-value pairs:
                if ((defs.size() % 2) == 0) {
                  std::vector<std::string>::const_iterator di = defs.begin();
                  while (di != defs.end()) {
                    std::string def = *di;
                    ++di;
                    std::string val;
                    if (di != defs.end()) {
                      val = *di;
                      ++di;
                    }

                    // insert the definition if not already added.
                    if (emmited.find(def) == emmited.end()) {
                      emmited.insert(def);
                      xml.StartElement("listOptionValue");
                      xml.Attribute("builtIn", "false");
                      if (val.empty()) {
                          xml.Attribute("value", def);
                      } else {
                          xml.Attribute("value", def + "=" + val);
                      }
                      xml.EndElement();
                    }
                  }
                }
              }
            xml.EndElement(); // option

            xml.StartElement("inputType");
            xml.Attribute("id",
                    "cdt.managedbuild.tool.gnu.cpp.compiler.input.86908675");
            xml.Attribute("superClass",
                    "cdt.managedbuild.tool.gnu.cpp.compiler.input");
            xml.EndElement(); // inputType

          xml.EndElement(); // tool

        xml.EndElement(); // toolChain

      xml.EndElement(); // folderInfo


      xml.StartElement("sourceEntries");

        std::string excludeFromOut;
        excludeFromOut += "**/CMakeFiles/";

        xml.StartElement("entry");
        xml.Attribute("excluding", excludeFromOut);
        xml.Attribute("flags", "VALUE_WORKSPACE_PATH");
        xml.Attribute("kind", "sourcePath");
        xml.Attribute("name", "");
        xml.EndElement();

      xml.EndElement(); // sourceEntries

    xml.EndElement(); // configuration

  xml.EndElement(); // storageModule

  // ???
  xml.StartElement("storageModule");
  xml.Attribute("moduleId", "org.eclipse.cdt.core.externalSettings");
  xml.EndElement(); // storageModule

  xml.EndElement(); // cconfiguration
  xml.EndElement(); // storageModule

  xml.StartElement("storageModule");
  xml.Attribute("moduleId", "cdtBuildSystem");
  xml.Attribute("version", "4.0.0");

  xml.StartElement("project");
  xml.Attribute("id", std::string(lg->GetProjectName()) + ".null.429358151");
  xml.Attribute("name", lg->GetProjectName());
  xml.EndElement(); // project

  xml.EndElement(); // storageModule

  xml.StartElement("storageModule");
  xml.Attribute("moduleId", "org.eclipse.cdt.core.LanguageSettingsProviders");
  xml.EndElement(); // storageModule

  xml.StartElement("storageModule");
  xml.Attribute("moduleId", "org.eclipse.cdt.make.core.buildtargets");
  xml.EndElement(); // storageModule

  xml.StartElement("storageModule");
  xml.Attribute("moduleId", "refreshScope");
  xml.Attribute("versionNumber", "2");

    xml.StartElement("configuration");
    xml.Attribute("configurationName", "default");

      xml.StartElement("resource");
      xml.Attribute("resourceType", "PROJECT");
      xml.Attribute("workspacePath", "/" + lg->GetProjectName());
      xml.EndElement(); // resource

    xml.EndElement(); // configuration

  xml.EndElement(); // storageModule

  xml.StartElement("storageModule");
  xml.Attribute("moduleId",
          "org.eclipse.cdt.internal.ui.text.commentOwnerProjectMappings");
  xml.EndElement(); // storageModule

  xml.StartElement("storageModule");
  xml.Attribute("moduleId", "scannerConfiguration");
  xml.EndElement(); // storageModule

  // Append additional cproject contents without applying any XML formatting
  if (const char* extraCProjectContents =
        mf->GetState()->GetGlobalProperty("ECLIPSE_EXTRA_CPROJECT_CONTENTS")) {
    fout << extraCProjectContents;
  }

  xml.EndElement(); // cproject
}

std::string cmExtraEclipseCDT9Generator::GetEclipsePath(
  const std::string& path)
{
#if defined(__CYGWIN__)
  std::string cmd = "cygpath -m " + path;
  std::string out;
  if (!cmSystemTools::RunSingleCommand(cmd.c_str(), &out, &out)) {
    return path;
  } else {
    out.erase(out.find_last_of('\n'));
    return out;
  }
#else
  return path;
#endif
}

std::string cmExtraEclipseCDT9Generator::GetPathBasename(
  const std::string& path)
{
  std::string outputBasename = path;
  while (!outputBasename.empty() &&
         (outputBasename[outputBasename.size() - 1] == '/' ||
          outputBasename[outputBasename.size() - 1] == '\\')) {
    outputBasename.resize(outputBasename.size() - 1);
  }
  std::string::size_type loc = outputBasename.find_last_of("/\\");
  if (loc != std::string::npos) {
    outputBasename = outputBasename.substr(loc + 1);
  }

  return outputBasename;
}

std::string cmExtraEclipseCDT9Generator::GenerateProjectName(
  const std::string& name, const std::string& type, const std::string& path)
{
  return name + (type.empty() ? "" : "-") + type + "@" + path;
}

// Helper functions
void cmExtraEclipseCDT9Generator::AppendStorageScanners(
  cmXMLWriter& xml, const cmMakefile& makefile)
{
  // we need the "make" and the C (or C++) compiler which are used, Alex
  std::string make = makefile.GetRequiredDefinition("CMAKE_MAKE_PROGRAM");
  std::string compiler = makefile.GetSafeDefinition("CMAKE_C_COMPILER");
  std::string arg1 = makefile.GetSafeDefinition("CMAKE_C_COMPILER_ARG1");
  if (compiler.empty()) {
    compiler = makefile.GetSafeDefinition("CMAKE_CXX_COMPILER");
    arg1 = makefile.GetSafeDefinition("CMAKE_CXX_COMPILER_ARG1");
  }
  if (compiler.empty()) // Hmm, what to do now ?
  {
    compiler = "gcc";
  }

  // the following right now hardcodes gcc behaviour :-/
  std::string compilerArgs =
    "-E -P -v -dD ${plugin_state_location}/${specs_file}";
  if (!arg1.empty()) {
    arg1 += " ";
    compilerArgs = arg1 + compilerArgs;
  }

  xml.StartElement("storageModule");
  xml.Attribute("moduleId", "scannerConfiguration");

  xml.StartElement("autodiscovery");
  xml.Attribute("enabled", "true");
  xml.Attribute("problemReportingEnabled", "true");
  xml.Attribute("selectedProfileId",
                "org.eclipse.cdt.make.core.GCCStandardMakePerProjectProfile");
  xml.EndElement(); // autodiscovery

  cmExtraEclipseCDT9Generator::AppendScannerProfile(
    xml, "org.eclipse.cdt.make.core.GCCStandardMakePerProjectProfile", true,
    "", true, "specsFile", compilerArgs, compiler, true, true);
  cmExtraEclipseCDT9Generator::AppendScannerProfile(
    xml, "org.eclipse.cdt.make.core.GCCStandardMakePerFileProfile", true, "",
    true, "makefileGenerator", "-f ${project_name}_scd.mk", make, true, true);

  xml.EndElement(); // storageModule
}

// The prefix is prepended before the actual name of the target. The purpose
// of that is to sort the targets in the view of Eclipse, so that at first
// the global/utility/all/clean targets appear ": ", then the executable
// targets "[exe] ", then the libraries "[lib]", then the rules for the
// object files "[obj]", then for preprocessing only "[pre] " and
// finally the assembly files "[to asm] ". Note the "to" in "to asm",
// without it, "asm" would be the first targets in the list, with the "to"
// they are the last targets, which makes more sense.
void cmExtraEclipseCDT9Generator::AppendTarget(
  cmXMLWriter& xml, const std::string& target, const std::string& make,
  const std::string& makeArgs, const std::string& path, const char* prefix,
  const char* makeTarget)
{
  xml.StartElement("target");
  xml.Attribute("name", prefix + target);
  xml.Attribute("path", path);
  xml.Attribute("targetID", "org.eclipse.cdt.make.MakeTargetBuilder");
  xml.Element("buildCommand",
              cmExtraEclipseCDT9Generator::GetEclipsePath(make));
  xml.Element("buildArguments", makeArgs);
  xml.Element("buildTarget", makeTarget ? makeTarget : target.c_str());
  xml.Element("stopOnError", "true");
  xml.Element("useDefaultCommand", "false");
  xml.EndElement();
}

void cmExtraEclipseCDT9Generator::AppendScannerProfile(
  cmXMLWriter& xml, const std::string& profileID, bool openActionEnabled,
  const std::string& openActionFilePath, bool pParserEnabled,
  const std::string& scannerInfoProviderID,
  const std::string& runActionArguments, const std::string& runActionCommand,
  bool runActionUseDefault, bool sipParserEnabled)
{
  xml.StartElement("profile");
  xml.Attribute("id", profileID);

  xml.StartElement("buildOutputProvider");
  xml.StartElement("openAction");
  xml.Attribute("enabled", openActionEnabled ? "true" : "false");
  xml.Attribute("filePath", openActionFilePath);
  xml.EndElement(); // openAction
  xml.StartElement("parser");
  xml.Attribute("enabled", pParserEnabled ? "true" : "false");
  xml.EndElement(); // parser
  xml.EndElement(); // buildOutputProvider

  xml.StartElement("scannerInfoProvider");
  xml.Attribute("id", scannerInfoProviderID);
  xml.StartElement("runAction");
  xml.Attribute("arguments", runActionArguments);
  xml.Attribute("command", runActionCommand);
  xml.Attribute("useDefault", runActionUseDefault ? "true" : "false");
  xml.EndElement(); // runAction
  xml.StartElement("parser");
  xml.Attribute("enabled", sipParserEnabled ? "true" : "false");
  xml.EndElement(); // parser
  xml.EndElement(); // scannerInfoProvider

  xml.EndElement(); // profile
}

void cmExtraEclipseCDT9Generator::AppendLinkedResource(cmXMLWriter& xml,
                                                       const std::string& name,
                                                       const std::string& path,
                                                       LinkType linkType)
{
  const char* locationTag = "location";
  int typeTag = 2;
  if (linkType == VirtualFolder) // ... and not a linked folder
  {
    locationTag = "locationURI";
  }
  if (linkType == LinkToFile) {
    typeTag = 1;
  }

  xml.StartElement("link");
  xml.Element("name", name);
  xml.Element("type", typeTag);
  xml.Element(locationTag, path);
  xml.EndElement();
}
