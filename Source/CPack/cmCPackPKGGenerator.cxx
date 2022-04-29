/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCPackPKGGenerator.h"

#include <vector>

#include "cmCPackComponentGroup.h"
#include "cmCPackGenerator.h"
#include "cmCPackLog.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"
#include "cmXMLWriter.h"

cmCPackPKGGenerator::cmCPackPKGGenerator()
{
  this->componentPackageMethod = ONE_PACKAGE;
}

cmCPackPKGGenerator::~cmCPackPKGGenerator() = default;

bool cmCPackPKGGenerator::SupportsComponentInstallation() const
{
  return true;
}

int cmCPackPKGGenerator::InitializeInternal()
{
  cmCPackLogger(cmCPackLog::LOG_DEBUG,
                "cmCPackPKGGenerator::Initialize()" << std::endl);

  return this->Superclass::InitializeInternal();
}

std::string cmCPackPKGGenerator::GetPackageName(
  const cmCPackComponent& component)
{
  if (component.ArchiveFile.empty()) {
    std::string packagesDir =
      cmStrCat(this->GetOption("CPACK_TEMPORARY_DIRECTORY"), ".dummy");
    std::ostringstream out;
    out << cmSystemTools::GetFilenameWithoutLastExtension(packagesDir) << "-"
        << component.Name << ".pkg";
    return out.str();
  }

  return component.ArchiveFile + ".pkg";
}

void cmCPackPKGGenerator::CreateBackground(const char* themeName,
                                           const char* metapackageFile,
                                           cm::string_view genName,
                                           cmXMLWriter& xout)
{
  std::string paramSuffix =
    (themeName == nullptr) ? "" : cmSystemTools::UpperCase(themeName);
  std::string opt = (themeName == nullptr)
    ? cmStrCat("CPACK_", genName, "_BACKGROUND")
    : cmStrCat("CPACK_", genName, "_BACKGROUND_", paramSuffix);
  cmValue bgFileName = this->GetOption(opt);
  if (bgFileName == nullptr) {
    return;
  }

  std::string bgFilePath = cmStrCat(metapackageFile, "/Contents/", bgFileName);

  if (!cmSystemTools::FileExists(bgFilePath)) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Background image doesn't exist in the resource directory: "
                    << bgFileName << std::endl);
    return;
  }

  if (themeName == nullptr) {
    xout.StartElement("background");
  } else {
    xout.StartElement(cmStrCat("background-", themeName));
  }

  xout.Attribute("file", bgFileName);

  cmValue param = this->GetOption(cmStrCat(opt, "_ALIGNMENT"));
  if (param != nullptr) {
    xout.Attribute("alignment", param);
  }

  param = this->GetOption(cmStrCat(opt, "_SCALING"));
  if (param != nullptr) {
    xout.Attribute("scaling", param);
  }

  // Apple docs say that you must provide either mime-type or uti
  // attribute for the background, but I've seen examples that
  // doesn't have them, so don't make them mandatory.
  param = this->GetOption(cmStrCat(opt, "_MIME_TYPE"));
  if (param != nullptr) {
    xout.Attribute("mime-type", param);
  }

  param = this->GetOption(cmStrCat(opt, "_UTI"));
  if (param != nullptr) {
    xout.Attribute("uti", param);
  }

  xout.EndElement();
}

void cmCPackPKGGenerator::WriteDistributionFile(const char* metapackageFile,
                                                const char* genName)
{
  std::string distributionTemplate =
    this->FindTemplate("CPack.distribution.dist.in");
  if (distributionTemplate.empty()) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Cannot find input file: " << distributionTemplate
                                             << std::endl);
    return;
  }

  std::string distributionFile =
    cmStrCat(metapackageFile, "/Contents/distribution.dist");

  std::ostringstream xContents;
  cmXMLWriter xout(xContents, 1);

  // Installer-wide options and domains. These need to be separate from the
  // choices and background elements added further below so that we can
  // preserve backward compatibility.
  xout.StartElement("options");
  xout.Attribute("allow-external-scripts", "no");
  xout.Attribute("customize", "allow");
  if (cmIsOff(this->GetOption("CPACK_PRODUCTBUILD_DOMAINS"))) {
    xout.Attribute("rootVolumeOnly", "false");
  }
  xout.EndElement();
  this->CreateDomains(xout);

  // In order to preserve backward compatibility, all elements added below
  // here need to be made available in a variable named
  // CPACK_PACKAGEMAKER_CHOICES. The above elements are new and only appear
  // in the CPACK_APPLE_PKG_INSTALLER_CONTENT variable, which is a superset
  // of what CPACK_PACKAGEMAKER_CHOICES used to provide. The renaming reflects
  // the fact that CMake has deprecated the PackageMaker generator.

  // Create the choice outline, which provides a tree-based view of
  // the components in their groups.
  std::ostringstream choiceOut;
  cmXMLWriter xChoiceOut(choiceOut, 1);
  xChoiceOut.StartElement("choices-outline");

  // Emit the outline for the groups
  for (auto const& group : this->ComponentGroups) {
    if (group.second.ParentGroup == nullptr) {
      CreateChoiceOutline(group.second, xChoiceOut);
    }
  }

  // Emit the outline for the non-grouped components
  for (auto const& comp : this->Components) {
    if (!comp.second.Group) {
      xChoiceOut.StartElement("line");
      xChoiceOut.Attribute("choice", comp.first + "Choice");
      xChoiceOut.Content(""); // Avoid self-closing tag.
      xChoiceOut.EndElement();
    }
  }
  if (!this->PostFlightComponent.Name.empty()) {
    xChoiceOut.StartElement("line");
    xChoiceOut.Attribute("choice", PostFlightComponent.Name + "Choice");
    xChoiceOut.Content(""); // Avoid self-closing tag.
    xChoiceOut.EndElement();
  }
  xChoiceOut.EndElement(); // choices-outline>

  // Create the actual choices
  for (auto const& group : this->ComponentGroups) {
    CreateChoice(group.second, xChoiceOut);
  }
  for (auto const& comp : this->Components) {
    CreateChoice(comp.second, xChoiceOut);
  }

  if (!this->PostFlightComponent.Name.empty()) {
    CreateChoice(PostFlightComponent, xChoiceOut);
  }

  // default background. These are not strictly part of the choices, but they
  // must be included in CPACK_PACKAGEMAKER_CHOICES to preserve backward
  // compatibility.
  this->CreateBackground(nullptr, metapackageFile, genName, xChoiceOut);
  // Dark Aqua
  this->CreateBackground("darkAqua", metapackageFile, genName, xChoiceOut);

  // Provide the content for substitution into the template. Support both the
  // old and new variables.
  this->SetOption("CPACK_PACKAGEMAKER_CHOICES", choiceOut.str());
  this->SetOption("CPACK_APPLE_PKG_INSTALLER_CONTENT",
                  cmStrCat(xContents.str(), "    ", choiceOut.str()));

  // Create the distribution.dist file in the metapackage to turn it
  // into a distribution package.
  this->ConfigureFile(distributionTemplate, distributionFile);
}

void cmCPackPKGGenerator::CreateChoiceOutline(
  const cmCPackComponentGroup& group, cmXMLWriter& xout)
{
  xout.StartElement("line");
  xout.Attribute("choice", group.Name + "Choice");
  for (cmCPackComponentGroup* subgroup : group.Subgroups) {
    CreateChoiceOutline(*subgroup, xout);
  }

  for (cmCPackComponent* comp : group.Components) {
    xout.StartElement("line");
    xout.Attribute("choice", comp->Name + "Choice");
    xout.Content(""); // Avoid self-closing tag.
    xout.EndElement();
  }
  xout.EndElement();
}

void cmCPackPKGGenerator::CreateChoice(const cmCPackComponentGroup& group,
                                       cmXMLWriter& xout)
{
  xout.StartElement("choice");
  xout.Attribute("id", group.Name + "Choice");
  xout.Attribute("title", group.DisplayName);
  xout.Attribute("start_selected", "true");
  xout.Attribute("start_enabled", "true");
  xout.Attribute("start_visible", "true");
  if (!group.Description.empty()) {
    xout.Attribute("description", group.Description);
  }
  xout.EndElement();
}

void cmCPackPKGGenerator::CreateChoice(const cmCPackComponent& component,
                                       cmXMLWriter& xout)
{
  std::string packageId;
  if (cmValue i = this->GetOption("CPACK_PRODUCTBUILD_IDENTIFIER")) {
    packageId = cmStrCat(i, '.', component.Name);
  } else {
    packageId =
      cmStrCat("com.", this->GetOption("CPACK_PACKAGE_VENDOR"), '.',
               this->GetOption("CPACK_PACKAGE_NAME"), '.', component.Name);
  }

  xout.StartElement("choice");
  xout.Attribute("id", component.Name + "Choice");
  xout.Attribute("title", component.DisplayName);
  xout.Attribute(
    "start_selected",
    component.IsDisabledByDefault && !component.IsRequired ? "false" : "true");
  xout.Attribute("start_enabled", component.IsRequired ? "false" : "true");
  xout.Attribute("start_visible", component.IsHidden ? "false" : "true");
  if (!component.Description.empty()) {
    xout.Attribute("description", component.Description);
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
    std::ostringstream selected("my.choice.selected", std::ios_base::ate);
    std::set<const cmCPackComponent*> visited;
    AddDependencyAttributes(component, visited, selected);
    visited.clear();
    AddReverseDependencyAttributes(component, visited, selected);
    xout.Attribute("selected", selected.str());
  }
  xout.StartElement("pkg-ref");
  xout.Attribute("id", packageId);
  xout.EndElement(); // pkg-ref
  xout.EndElement(); // choice

  // Create a description of the package associated with this
  // component.
  std::string relativePackageLocation =
    cmStrCat("Contents/Packages/", this->GetPackageName(component));

  // Determine the installed size of the package.
  std::string dirName =
    cmStrCat(this->GetOption("CPACK_TEMPORARY_DIRECTORY"), '/', component.Name,
             this->GetOption("CPACK_PACKAGING_INSTALL_PREFIX"));
  unsigned long installedSize = component.GetInstalledSizeInKbytes(dirName);

  xout.StartElement("pkg-ref");
  xout.Attribute("id", packageId);
  xout.Attribute("version", this->GetOption("CPACK_PACKAGE_VERSION"));
  xout.Attribute("installKBytes", installedSize);
  // The auth attribute is deprecated in favor of the domains element
  if (cmIsOff(this->GetOption("CPACK_PRODUCTBUILD_DOMAINS"))) {
    xout.Attribute("auth", "Admin");
  }
  xout.Attribute("onConclusion", "None");
  if (component.IsDownloaded) {
    xout.Content(this->GetOption("CPACK_DOWNLOAD_SITE"));
    xout.Content(this->GetPackageName(component));
  } else {
    xout.Content("file:./");
    xout.Content(cmSystemTools::EncodeURL(relativePackageLocation,
                                          /*escapeSlashes=*/false));
  }
  xout.EndElement(); // pkg-ref
}

void cmCPackPKGGenerator::CreateDomains(cmXMLWriter& xout)
{
  if (cmIsOff(this->GetOption("CPACK_PRODUCTBUILD_DOMAINS"))) {
    return;
  }

  xout.StartElement("domains");

  // Product can be installed at the root of any volume by default
  // unless specified
  cmValue param = this->GetOption("CPACK_PRODUCTBUILD_DOMAINS_ANYWHERE");
  xout.Attribute("enable_anywhere",
                 (param && cmIsOff(param)) ? "false" : "true");

  // Product cannot be installed into the current user's home directory
  // by default unless specified
  param = this->GetOption("CPACK_PRODUCTBUILD_DOMAINS_USER");
  xout.Attribute("enable_currentUserHome",
                 (param && cmIsOn(param)) ? "true" : "false");

  // Product can be installed into the root directory by default
  // unless specified
  param = this->GetOption("CPACK_PRODUCTBUILD_DOMAINS_ROOT");
  xout.Attribute("enable_localSystem",
                 (param && cmIsOff(param)) ? "false" : "true");

  xout.EndElement();
}

void cmCPackPKGGenerator::AddDependencyAttributes(
  const cmCPackComponent& component,
  std::set<const cmCPackComponent*>& visited, std::ostringstream& out)
{
  if (visited.find(&component) != visited.end()) {
    return;
  }
  visited.insert(&component);

  for (cmCPackComponent* depend : component.Dependencies) {
    out << " && choices['" << depend->Name << "Choice'].selected";
    AddDependencyAttributes(*depend, visited, out);
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

  for (cmCPackComponent* depend : component.ReverseDependencies) {
    out << " || choices['" << depend->Name << "Choice'].selected";
    AddReverseDependencyAttributes(*depend, visited, out);
  }
}

bool cmCPackPKGGenerator::CopyCreateResourceFile(const std::string& name,
                                                 const std::string& dirName)
{
  std::string uname = cmSystemTools::UpperCase(name);
  std::string cpackVar = "CPACK_RESOURCE_FILE_" + uname;
  cmValue inFileName = this->GetOption(cpackVar);
  if (!inFileName) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "CPack option: " << cpackVar.c_str()
                                   << " not specified. It should point to "
                                   << (!name.empty() ? name : "<empty>")
                                   << ".rtf, " << name << ".html, or " << name
                                   << ".txt file" << std::endl);
    return false;
  }
  if (!cmSystemTools::FileExists(inFileName)) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Cannot find " << (!name.empty() ? name : "<empty>")
                                 << " resource file: " << inFileName
                                 << std::endl);
    return false;
  }
  std::string ext = cmSystemTools::GetFilenameLastExtension(inFileName);
  if (ext != ".rtfd" && ext != ".rtf" && ext != ".html" && ext != ".txt") {
    cmCPackLogger(
      cmCPackLog::LOG_ERROR,
      "Bad file extension specified: "
        << ext
        << ". Currently only .rtfd, .rtf, .html, and .txt files allowed."
        << std::endl);
    return false;
  }

  std::string destFileName = cmStrCat(dirName, '/', name, ext);

  // Set this so that distribution.dist gets the right name (without
  // the path).
  this->SetOption("CPACK_RESOURCE_FILE_" + uname + "_NOPATH", (name + ext));

  cmCPackLogger(cmCPackLog::LOG_VERBOSE,
                "Configure file: " << (inFileName ? *inFileName : "(NULL)")
                                   << " to " << destFileName << std::endl);
  this->ConfigureFile(inFileName, destFileName);
  return true;
}

bool cmCPackPKGGenerator::CopyResourcePlistFile(const std::string& name,
                                                const char* outName)
{
  if (!outName) {
    outName = name.c_str();
  }

  std::string inFName = cmStrCat("CPack.", name, ".in");
  std::string inFileName = this->FindTemplate(inFName.c_str());
  if (inFileName.empty()) {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Cannot find input file: " << inFName << std::endl);
    return false;
  }

  std::string destFileName =
    cmStrCat(this->GetOption("CPACK_TOPLEVEL_DIRECTORY"), '/', outName);

  cmCPackLogger(cmCPackLog::LOG_VERBOSE,
                "Configure file: " << inFileName << " to " << destFileName
                                   << std::endl);
  this->ConfigureFile(inFileName, destFileName);
  return true;
}

int cmCPackPKGGenerator::CopyInstallScript(const std::string& resdir,
                                           const std::string& script,
                                           const std::string& name)
{
  std::string dst = cmStrCat(resdir, '/', name);
  cmSystemTools::CopyFileAlways(script, dst);
  cmSystemTools::SetPermissions(dst.c_str(), 0777);
  cmCPackLogger(cmCPackLog::LOG_VERBOSE,
                "copy script : " << script << "\ninto " << dst << std::endl);

  return 1;
}
