/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmXCodeScheme.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorTarget.h"
#include "cmXMLSafe.h"

cmXCodeScheme::cmXCodeScheme(cmXCodeObject* xcObj, unsigned int xcVersion)
  : targetName(xcObj->GetTarget()->GetName())
  , targetId(xcObj->GetId())
  , XcodeVersion(xcVersion)
{
}

void cmXCodeScheme::WriteXCodeSharedScheme(const std::string& xcProjDir,
                                           const std::string sourceRoot)
{
  // Create shared scheme sub-directory tree
  //
  std::string xcodeSchemeDir = xcProjDir;
  xcodeSchemeDir += "/xcshareddata/xcschemes";
  cmSystemTools::MakeDirectory(xcodeSchemeDir.c_str());

  std::string xcodeSchemeFile = xcodeSchemeDir;
  xcodeSchemeFile += "/";
  xcodeSchemeFile += targetName;
  xcodeSchemeFile += ".xcscheme";

  cmGeneratedFileStream fout(xcodeSchemeFile.c_str());
  fout.SetCopyIfDifferent(true);
  if (!fout) {
    return;
  }

  std::string xcProjRelDir = xcProjDir.substr(sourceRoot.size() + 1);
  WriteXCodeXCScheme(fout, xcProjRelDir);
}

void cmXCodeScheme::WriteXCodeXCScheme(std::ostream& fout,
                                       const std::string& xcProjDir)
{
  cmXMLWriter xout(fout);
  xout.StartDocument();

  xout.StartElement("Scheme");
  xout.BreakAttributes();
  xout.Attribute("LastUpgradeVersion", WriteVersionString());
  xout.Attribute("version", "1.3");

  WriteBuildAction(xout, xcProjDir);
  WriteTestAction(xout, "Debug");
  WriteLaunchAction(xout, "Release", xcProjDir);
  WriteProfileAction(xout, "Release");
  WriteAnalyzeAction(xout, "Debug");
  WriteArchiveAction(xout, "Release");

  xout.EndElement();
}

void cmXCodeScheme::WriteBuildAction(cmXMLWriter& xout,
                                     const std::string& xcProjDir)
{
  xout.StartElement("BuildAction");
  xout.BreakAttributes();
  xout.Attribute("parallelizeBuildables", "YES");
  xout.Attribute("buildImplicitDependencies", "YES");

  xout.StartElement("BuildActionEntries");
  xout.StartElement("BuildActionEntry");
  xout.BreakAttributes();
  xout.Attribute("buildForTesting", "YES");
  xout.Attribute("buildForRunning", "YES");
  xout.Attribute("buildForProfiling", "YES");
  xout.Attribute("buildForArchiving", "YES");
  xout.Attribute("buildForAnalyzing", "YES");

  xout.StartElement("BuildableReference");
  xout.BreakAttributes();
  xout.Attribute("BuildableIdentifier", "primary");
  xout.Attribute("BlueprintIdentifier", targetId);
  xout.Attribute("BuildableName", targetName);
  xout.Attribute("BlueprintName", targetName);
  xout.Attribute("ReferencedContainer", "container:" + xcProjDir);
  xout.EndElement();

  xout.EndElement(); // BuildActionEntry
  xout.EndElement(); // BuildActionEntries
  xout.EndElement(); // BuildAction
}

void cmXCodeScheme::WriteTestAction(cmXMLWriter& xout,
                                    std::string configuration)
{
  xout.StartElement("TestAction");
  xout.BreakAttributes();
  xout.Attribute("buildConfiguration", configuration);
  xout.Attribute("selectedDebuggerIdentifier",
                 "Xcode.DebuggerFoundation.Debugger.LLDB");
  xout.Attribute("selectedLauncherIdentifier",
                 "Xcode.DebuggerFoundation.Launcher.LLDB");
  xout.Attribute("shouldUseLaunchSchemeArgsEnv", "YES");

  xout.StartElement("Testables");
  xout.EndElement();

  xout.StartElement("AdditionalOptions");
  xout.EndElement();

  xout.EndElement(); // TestAction
}

void cmXCodeScheme::WriteLaunchAction(cmXMLWriter& xout,
                                      std::string configuration,
                                      const std::string& xcProjDir)
{
  xout.StartElement("LaunchAction");
  xout.BreakAttributes();
  xout.Attribute("buildConfiguration", configuration);
  xout.Attribute("selectedDebuggerIdentifier",
                 "Xcode.DebuggerFoundation.Debugger.LLDB");
  xout.Attribute("selectedLauncherIdentifier",
                 "Xcode.DebuggerFoundation.Launcher.LLDB");
  xout.Attribute("launchStyle", "0");
  xout.Attribute("useCustomWorkingDirectory", "NO");
  xout.Attribute("ignoresPersistentStateOnLaunch", "NO");
  xout.Attribute("debugDocumentVersioning", "YES");
  xout.Attribute("debugServiceExtension", "internal");
  xout.Attribute("allowLocationSimulation", "YES");

  xout.StartElement("MacroExpansion");

  xout.StartElement("BuildableReference");
  xout.BreakAttributes();
  xout.Attribute("BuildableIdentifier", "primary");
  xout.Attribute("BlueprintIdentifier", targetId);
  xout.Attribute("BuildableName", targetName);
  xout.Attribute("BlueprintName", targetName);
  xout.Attribute("ReferencedContainer", "container:" + xcProjDir);
  xout.EndElement();

  xout.EndElement(); // MacroExpansion

  xout.StartElement("AdditionalOptions");
  xout.EndElement();

  xout.EndElement(); // LaunchAction
}

void cmXCodeScheme::WriteProfileAction(cmXMLWriter& xout,
                                       std::string configuration)
{
  xout.StartElement("ProfileAction");
  xout.BreakAttributes();
  xout.Attribute("buildConfiguration", configuration);
  xout.Attribute("shouldUseLaunchSchemeArgsEnv", "YES");
  xout.Attribute("savedToolIdentifier", "");
  xout.Attribute("useCustomWorkingDirectory", "NO");
  xout.Attribute("debugDocumentVersioning", "YES");
  xout.EndElement();
}

void cmXCodeScheme::WriteAnalyzeAction(cmXMLWriter& xout,
                                       std::string configuration)
{
  xout.StartElement("AnalyzeAction");
  xout.BreakAttributes();
  xout.Attribute("buildConfiguration", configuration);
  xout.EndElement();
}

void cmXCodeScheme::WriteArchiveAction(cmXMLWriter& xout,
                                       std::string configuration)
{
  xout.StartElement("ArchiveAction");
  xout.BreakAttributes();
  xout.Attribute("buildConfiguration", configuration);
  xout.Attribute("revealArchiveInOrganizer", "YES");
  xout.EndElement();
}

std::string cmXCodeScheme::WriteVersionString()
{
  std::string ver = "0";
  ver += std::to_string(this->XcodeVersion);
  ver += "0";
  return ver;
}
