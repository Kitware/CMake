/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCPackPKGGenerator.h"

#include "cmCPackComponentGroup.h"
#include "cmCPackLog.h"
#include "cmGeneratedFileStream.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmSystemTools.h"
#include "cmake.h"

#include <cmsys/Glob.hxx>
#include <cmsys/SystemTools.hxx>

cmCPackPKGGenerator::cmCPackPKGGenerator()
{
  this->componentPackageMethod = ONE_PACKAGE;
}

cmCPackPKGGenerator::~cmCPackPKGGenerator()
{
}

bool cmCPackPKGGenerator::SupportsComponentInstallation() const
{
  return true;
}

int cmCPackPKGGenerator::InitializeInternal()
{
  cmCPackLogger(cmCPackLog::LOG_DEBUG, "cmCPackPKGGenerator::Initialize()"
                  << std::endl);

  return this->Superclass::InitializeInternal();
}

std::string cmCPackPKGGenerator::GetPackageName(
  const cmCPackComponent& component)
{
  if (component.ArchiveFile.empty()) {
    std::string packagesDir = this->GetOption("CPACK_TEMPORARY_DIRECTORY");
    packagesDir += ".dummy";
    std::ostringstream out;
    out << cmSystemTools::GetFilenameWithoutLastExtension(packagesDir) << "-"
        << component.Name << ".pkg";
    return out.str();
  } else {
    return component.ArchiveFile + ".pkg";
  }
}

void cmCPackPKGGenerator::WriteDistributionFile(const char* metapackageFile)
{
  std::string distributionTemplate =
    this->FindTemplate("CPack.distribution.dist.in");
  if (distributionTemplate.empty()) {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Cannot find input file: "
                    << distributionTemplate << std::endl);
    return;
  }

  std::string distributionFile = metapackageFile;
  distributionFile += "/Contents/distribution.dist";

  // Create the choice outline, which provides a tree-based view of
  // the components in their groups.
  std::ostringstream choiceOut;
  choiceOut << "<choices-outline>" << std::endl;

  // Emit the outline for the groups
  std::map<std::string, cmCPackComponentGroup>::iterator groupIt;
  for (groupIt = this->ComponentGroups.begin();
       groupIt != this->ComponentGroups.end(); ++groupIt) {
    if (groupIt->second.ParentGroup == 0) {
      CreateChoiceOutline(groupIt->second, choiceOut);
    }
  }

  // Emit the outline for the non-grouped components
  std::map<std::string, cmCPackComponent>::iterator compIt;
  for (compIt = this->Components.begin(); compIt != this->Components.end();
       ++compIt) {
    if (!compIt->second.Group) {
      choiceOut << "<line choice=\"" << compIt->first << "Choice\"></line>"
                << std::endl;
    }
  }
  if (!this->PostFlightComponent.Name.empty()) {
    choiceOut << "<line choice=\"" << PostFlightComponent.Name
              << "Choice\"></line>" << std::endl;
  }
  choiceOut << "</choices-outline>" << std::endl;

  // Create the actual choices
  for (groupIt = this->ComponentGroups.begin();
       groupIt != this->ComponentGroups.end(); ++groupIt) {
    CreateChoice(groupIt->second, choiceOut);
  }
  for (compIt = this->Components.begin(); compIt != this->Components.end();
       ++compIt) {
    CreateChoice(compIt->second, choiceOut);
  }

  if (!this->PostFlightComponent.Name.empty()) {
    CreateChoice(PostFlightComponent, choiceOut);
  }

  this->SetOption("CPACK_PACKAGEMAKER_CHOICES", choiceOut.str().c_str());

  // Create the distribution.dist file in the metapackage to turn it
  // into a distribution package.
  this->ConfigureFile(distributionTemplate.c_str(), distributionFile.c_str());
}

void cmCPackPKGGenerator::CreateChoiceOutline(
  const cmCPackComponentGroup& group, std::ostringstream& out)
{
  out << "<line choice=\"" << group.Name << "Choice\">" << std::endl;
  std::vector<cmCPackComponentGroup*>::const_iterator groupIt;
  for (groupIt = group.Subgroups.begin(); groupIt != group.Subgroups.end();
       ++groupIt) {
    CreateChoiceOutline(**groupIt, out);
  }

  std::vector<cmCPackComponent*>::const_iterator compIt;
  for (compIt = group.Components.begin(); compIt != group.Components.end();
       ++compIt) {
    out << "  <line choice=\"" << (*compIt)->Name << "Choice\"></line>"
        << std::endl;
  }
  out << "</line>" << std::endl;
}

void cmCPackPKGGenerator::CreateChoice(const cmCPackComponentGroup& group,
                                       std::ostringstream& out)
{
  out << "<choice id=\"" << group.Name << "Choice\" "
      << "title=\"" << group.DisplayName << "\" "
      << "start_selected=\"true\" "
      << "start_enabled=\"true\" "
      << "start_visible=\"true\" ";
  if (!group.Description.empty()) {
    out << "description=\"" << EscapeForXML(group.Description) << "\"";
  }
  out << "></choice>" << std::endl;
}

void cmCPackPKGGenerator::CreateChoice(const cmCPackComponent& component,
                                       std::ostringstream& out)
{
  std::string packageId = "com.";
  packageId += this->GetOption("CPACK_PACKAGE_VENDOR");
  packageId += '.';
  packageId += this->GetOption("CPACK_PACKAGE_NAME");
  packageId += '.';
  packageId += component.Name;

  out << "<choice id=\"" << component.Name << "Choice\" "
      << "title=\"" << component.DisplayName << "\" "
      << "start_selected=\""
      << (component.IsDisabledByDefault && !component.IsRequired ? "false"
                                                                 : "true")
      << "\" "
      << "start_enabled=\"" << (component.IsRequired ? "false" : "true")
      << "\" "
      << "start_visible=\"" << (component.IsHidden ? "false" : "true")
      << "\" ";
  if (!component.Description.empty()) {
    out << "description=\"" << EscapeForXML(component.Description) << "\" ";
  }
  if (!component.Dependencies.empty() ||
      !component.ReverseDependencies.empty()) {
    // The "selected" expression is evaluated each time any choice is
    // selected, for all choices *except* the one that the user
    // selected. A component is marked selected if it has been
    // selected (my.choice.selected in Javascript) and all of the
    // components it depends on have been selected (transitively) or
    // if any of the components that depend on it have been selected
    // (transitively). Assume that we have components A, B, C, D, and
    // E, where each component depends on the previous component (B
    // depends on A, C depends on B, D depends on C, and E depends on
    // D). The expression we build for the component C will be
    //   my.choice.selected && B && A || D || E
    // This way, selecting C will automatically select everything it depends
    // on (B and A), while selecting something that depends on C--either D
    // or E--will automatically cause C to get selected.
    out << "selected=\"my.choice.selected";
    std::set<const cmCPackComponent*> visited;
    AddDependencyAttributes(component, visited, out);
    visited.clear();
    AddReverseDependencyAttributes(component, visited, out);
    out << "\"";
  }
  out << ">" << std::endl;
  out << "  <pkg-ref id=\"" << packageId << "\"></pkg-ref>" << std::endl;
  out << "</choice>" << std::endl;

  // Create a description of the package associated with this
  // component.
  std::string relativePackageLocation = "Contents/Packages/";
  relativePackageLocation += this->GetPackageName(component);

  // Determine the installed size of the package.
  std::string dirName = this->GetOption("CPACK_TEMPORARY_DIRECTORY");
  dirName += '/';
  dirName += component.Name;
  dirName += this->GetOption("CPACK_PACKAGING_INSTALL_PREFIX");
  unsigned long installedSize =
    component.GetInstalledSizeInKbytes(dirName.c_str());

  out << "<pkg-ref id=\"" << packageId << "\" "
      << "version=\"" << this->GetOption("CPACK_PACKAGE_VERSION") << "\" "
      << "installKBytes=\"" << installedSize << "\" "
      << ">";
  if (component.IsDownloaded) {
    out << this->GetOption("CPACK_DOWNLOAD_SITE")
        << this->GetPackageName(component);
  } else {
    out << "file:./" << relativePackageLocation;
  }
  out << "</pkg-ref>" << std::endl;
}

void cmCPackPKGGenerator::AddDependencyAttributes(
  const cmCPackComponent& component,
  std::set<const cmCPackComponent*>& visited, std::ostringstream& out)
{
  if (visited.find(&component) != visited.end()) {
    return;
  }
  visited.insert(&component);

  std::vector<cmCPackComponent*>::const_iterator dependIt;
  for (dependIt = component.Dependencies.begin();
       dependIt != component.Dependencies.end(); ++dependIt) {
    out << " &amp;&amp; choices['" << (*dependIt)->Name << "Choice'].selected";
    AddDependencyAttributes(**dependIt, visited, out);
  }
}

void cmCPackPKGGenerator::AddReverseDependencyAttributes(
  const cmCPackComponent& component,
  std::set<const cmCPackComponent*>& visited, std::ostringstream& out)
{
  if (visited.find(&component) != visited.end()) {
    return;
  }
  visited.insert(&component);

  std::vector<cmCPackComponent*>::const_iterator dependIt;
  for (dependIt = component.ReverseDependencies.begin();
       dependIt != component.ReverseDependencies.end(); ++dependIt) {
    out << " || choices['" << (*dependIt)->Name << "Choice'].selected";
    AddReverseDependencyAttributes(**dependIt, visited, out);
  }
}

std::string cmCPackPKGGenerator::EscapeForXML(std::string str)
{
  cmSystemTools::ReplaceString(str, "&", "&amp;");
  cmSystemTools::ReplaceString(str, "<", "&lt;");
  cmSystemTools::ReplaceString(str, ">", "&gt;");
  cmSystemTools::ReplaceString(str, "\"", "&quot;");
  return str;
}

bool cmCPackPKGGenerator::CopyCreateResourceFile(const std::string& name,
                                                 const std::string& dirName)
{
  std::string uname = cmSystemTools::UpperCase(name);
  std::string cpackVar = "CPACK_RESOURCE_FILE_" + uname;
  const char* inFileName = this->GetOption(cpackVar.c_str());
  if (!inFileName) {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "CPack option: "
                    << cpackVar.c_str()
                    << " not specified. It should point to "
                    << (!name.empty() ? name : "<empty>") << ".rtf, " << name
                    << ".html, or " << name << ".txt file" << std::endl);
    return false;
  }
  if (!cmSystemTools::FileExists(inFileName)) {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Cannot find "
                    << (!name.empty() ? name : "<empty>")
                    << " resource file: " << inFileName << std::endl);
    return false;
  }
  std::string ext = cmSystemTools::GetFilenameLastExtension(inFileName);
  if (ext != ".rtfd" && ext != ".rtf" && ext != ".html" && ext != ".txt") {
    cmCPackLogger(
      cmCPackLog::LOG_ERROR, "Bad file extension specified: "
        << ext
        << ". Currently only .rtfd, .rtf, .html, and .txt files allowed."
        << std::endl);
    return false;
  }

  std::string destFileName = dirName;
  destFileName += '/';
  destFileName += name + ext;

  // Set this so that distribution.dist gets the right name (without
  // the path).
  this->SetOption(("CPACK_RESOURCE_FILE_" + uname + "_NOPATH").c_str(),
                  (name + ext).c_str());

  cmCPackLogger(cmCPackLog::LOG_VERBOSE,
                "Configure file: " << (inFileName ? inFileName : "(NULL)")
                                   << " to " << destFileName << std::endl);
  this->ConfigureFile(inFileName, destFileName.c_str());
  return true;
}

bool cmCPackPKGGenerator::CopyResourcePlistFile(const std::string& name,
                                                const char* outName)
{
  if (!outName) {
    outName = name.c_str();
  }

  std::string inFName = "CPack.";
  inFName += name;
  inFName += ".in";
  std::string inFileName = this->FindTemplate(inFName.c_str());
  if (inFileName.empty()) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Cannot find input file: " << inFName << std::endl);
    return false;
  }

  std::string destFileName = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
  destFileName += "/";
  destFileName += outName;

  cmCPackLogger(cmCPackLog::LOG_VERBOSE, "Configure file: "
                  << inFileName << " to " << destFileName << std::endl);
  this->ConfigureFile(inFileName.c_str(), destFileName.c_str());
  return true;
}

int cmCPackPKGGenerator::CopyInstallScript(const std::string& resdir,
                                           const std::string& script,
                                           const std::string& name)
{
  std::string dst = resdir;
  dst += "/";
  dst += name;
  cmSystemTools::CopyFileAlways(script.c_str(), dst.c_str());
  cmSystemTools::SetPermissions(dst.c_str(), 0777);
  cmCPackLogger(cmCPackLog::LOG_VERBOSE,
                "copy script : " << script << "\ninto " << dst << std::endl);

  return 1;
}
