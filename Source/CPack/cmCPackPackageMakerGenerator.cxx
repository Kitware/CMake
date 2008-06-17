/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmCPackPackageMakerGenerator.h"

#include "cmake.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmSystemTools.h"
#include "cmMakefile.h"
#include "cmGeneratedFileStream.h"
#include "cmCPackComponentGroup.h"
#include "cmCPackLog.h"

#include <cmsys/SystemTools.hxx>
#include <cmsys/Glob.hxx>

//----------------------------------------------------------------------
cmCPackPackageMakerGenerator::cmCPackPackageMakerGenerator()
{
  this->PackageMakerVersion = 0.0;
}

//----------------------------------------------------------------------
cmCPackPackageMakerGenerator::~cmCPackPackageMakerGenerator()
{
}

//----------------------------------------------------------------------
bool cmCPackPackageMakerGenerator::SupportsComponentInstallation() const
{
  return true;
}

//----------------------------------------------------------------------
int cmCPackPackageMakerGenerator::CopyInstallScript(const char* resdir,
                                                    const char* script,
                                                    const char* name)
{
  std::string dst = resdir;
  dst += "/";
  dst += name;
  cmSystemTools::CopyFileAlways(script, dst.c_str());
  cmSystemTools::SetPermissions(dst.c_str(),0777);
  cmCPackLogger(cmCPackLog::LOG_VERBOSE,
                "copy script : " << script << "\ninto " << dst.c_str() << 
                std::endl);

  return 1;
}

//----------------------------------------------------------------------
int cmCPackPackageMakerGenerator::CompressFiles(const char* outFileName,
  const char* toplevel,
  const std::vector<std::string>& files)
{
  (void) files; // TODO: Fix api to not need files.
  (void) toplevel; // TODO: Use toplevel
  // Create directory structure
  std::string resDir = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
  resDir += "/Resources";
  std::string preflightDirName = resDir + "/PreFlight";
  std::string postflightDirName = resDir + "/PostFlight";
  const char* preflight = this->GetOption("CPACK_PREFLIGHT_SCRIPT");
  const char* postflight = this->GetOption("CPACK_POSTFLIGHT_SCRIPT");
  const char* postupgrade = this->GetOption("CPACK_POSTUPGRADE_SCRIPT");
  // if preflight or postflight scripts not there create directories
  // of the same name, I think this makes it work
  if(!preflight)
    {
    if ( !cmsys::SystemTools::MakeDirectory(preflightDirName.c_str()))
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "Problem creating installer directory: "
                    << preflightDirName.c_str() << std::endl);
      return 0;
      }
    }
  if(!postflight)
    {
    if ( !cmsys::SystemTools::MakeDirectory(postflightDirName.c_str()))
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "Problem creating installer directory: "
                    << postflightDirName.c_str() << std::endl);
      return 0;
      }
    }
  // if preflight, postflight, or postupgrade are set 
  // then copy them into the resource directory and make
  // them executable
  if(preflight)
    {
    this->CopyInstallScript(resDir.c_str(),
                            preflight,
                            "preflight");
    }
  if(postflight)
    {
    this->CopyInstallScript(resDir.c_str(),
                            postflight,
                            "postflight");
    }
  if(postupgrade)
    {
    this->CopyInstallScript(resDir.c_str(),
                            postupgrade,
                            "postupgrade");
    }

  if (!this->Components.empty())
    {
    // Create the directory where component packages will be installed.
    std::string basePackageDir = toplevel;
    basePackageDir += "/packages";
    if (!cmsys::SystemTools::MakeDirectory(basePackageDir.c_str()))
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "Problem creating component packages directory: "
                    << basePackageDir.c_str() << std::endl);
      return 0;
      }

    // Create packages for each component
    std::map<std::string, cmCPackComponent>::iterator compIt;
    for (compIt = this->Components.begin(); compIt != this->Components.end();
         ++compIt)
      {
      std::string packageFile = basePackageDir;
      packageFile += '/';
      packageFile += GetPackageName(compIt->second);

      std::string packageDir = toplevel;
      packageDir += '/';
      packageDir += compIt->first;
      if (!this->GenerateComponentPackage(packageFile.c_str(), 
                                          packageDir.c_str(),
                                          compIt->second))
        {
        return 0;
        }
      }
    }
  this->SetOption("CPACK_MODULE_VERSION_SUFFIX", "");

  // Copy or create all of the resource files we need.
  if ( !this->CopyCreateResourceFile("License")
    || !this->CopyCreateResourceFile("ReadMe")
    || !this->CopyCreateResourceFile("Welcome")
    || !this->CopyResourcePlistFile("Info.plist")
    || !this->CopyResourcePlistFile("Description.plist") )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem copying the resource files"
      << std::endl);
    return 0;
    }

  std::string packageDirFileName
    = this->GetOption("CPACK_TEMPORARY_DIRECTORY");
  if (this->Components.empty())
    {
    packageDirFileName += ".pkg";
    }
  else
    {
    packageDirFileName += ".mpkg";
    if (this->PackageMakerVersion == 3.0)
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
         "PackageMaker 3.0 cannot build component-based installations."
         << std::endl << "Please use PackageMaker 2.5 instead." << std::endl);
      }
    }

  cmOStringStream pkgCmd;
  pkgCmd << "\"" << this->GetOption("CPACK_INSTALLER_PROGRAM")
         << "\" -build -p \"" << packageDirFileName << "\"";
  if (this->Components.empty())
    {
    pkgCmd << " -f \"" << this->GetOption("CPACK_TEMPORARY_DIRECTORY");
    }
  else
    {
    pkgCmd << " -mi \"" << this->GetOption("CPACK_TEMPORARY_DIRECTORY")
           << "/packages/";
    }
  pkgCmd << "\" -r \"" << this->GetOption("CPACK_TOPLEVEL_DIRECTORY")
         << "/Resources\" -i \""
         << this->GetOption("CPACK_TOPLEVEL_DIRECTORY") << "/Info.plist\" -d \""
         << this->GetOption("CPACK_TOPLEVEL_DIRECTORY") << "/Description.plist\"";
  if ( this->PackageMakerVersion > 2.0 )
    {
    pkgCmd << " -v";
    }
  if (!RunPackageMaker(pkgCmd.str().c_str(), packageDirFileName.c_str()))
    return 0;

  if (!this->Components.empty())
    {
    WriteDistributionFile(packageDirFileName.c_str());
    }

  std::string tmpFile = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
  tmpFile += "/hdiutilOutput.log";
  cmOStringStream dmgCmd;
  dmgCmd << "\"" << this->GetOption("CPACK_INSTALLER_PROGRAM_DISK_IMAGE")
    << "\" create -ov -format UDZO -srcfolder \"" << packageDirFileName
    << "\" \"" << outFileName << "\"";
  std::string output;
  int retVal = 1;
  bool res = cmSystemTools::RunSingleCommand(dmgCmd.str().c_str(), &output,
    &retVal, 0, this->GeneratorVerbose, 0);
  if ( !res || retVal )
    {
    cmGeneratedFileStream ofs(tmpFile.c_str());
    ofs << "# Run command: " << dmgCmd.str().c_str() << std::endl
      << "# Output:" << std::endl
      << output.c_str() << std::endl;
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem running hdiutil command: "
      << dmgCmd.str().c_str() << std::endl
      << "Please check " << tmpFile.c_str() << " for errors" << std::endl);
    return 0;
    }

  return 1;
}

//----------------------------------------------------------------------
int cmCPackPackageMakerGenerator::InitializeInternal()
{
  cmCPackLogger(cmCPackLog::LOG_DEBUG,
    "cmCPackPackageMakerGenerator::Initialize()" << std::endl);
  this->SetOptionIfNotSet("CPACK_PACKAGING_INSTALL_PREFIX", "/usr");
  std::vector<std::string> path;
  std::string pkgPath
    = "/Developer/Applications/Utilities/PackageMaker.app/Contents";
  std::string versionFile = pkgPath + "/version.plist";
  if ( !cmSystemTools::FileExists(versionFile.c_str()) )
    {
    pkgPath = "/Developer/Applications/PackageMaker.app/Contents";
    std::string newVersionFile = pkgPath + "/version.plist";
    if ( !cmSystemTools::FileExists(newVersionFile.c_str()) )
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
        "Cannot find PackageMaker compiler version file: "
        << versionFile.c_str() << " or " << newVersionFile.c_str()
        << std::endl);
      return 0;
      }
    versionFile = newVersionFile;
    }
  std::ifstream ifs(versionFile.c_str());
  if ( !ifs )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Cannot open PackageMaker compiler version file" << std::endl);
    return 0;
    }
  // Check the PackageMaker version
  cmsys::RegularExpression rexKey("<key>CFBundleShortVersionString</key>");
  cmsys::RegularExpression rexVersion("<string>([0-9]+.[0-9.]+)</string>");
  std::string line;
  bool foundKey = false;
  while ( cmSystemTools::GetLineFromStream(ifs, line) )
    {
    if ( rexKey.find(line) )
      {
      foundKey = true;
      break;
      }
    }
  if ( !foundKey )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Cannot find CFBundleShortVersionString in the PackageMaker compiler "
      "version file" << std::endl);
    return 0;
    }
  if ( !cmSystemTools::GetLineFromStream(ifs, line) ||
    !rexVersion.find(line) )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Problem reading the PackageMaker compiler version file: "
      << versionFile.c_str() << std::endl);
    return 0;
    }
  this->PackageMakerVersion = atof(rexVersion.match(1).c_str());
  if ( this->PackageMakerVersion < 1.0 )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Require PackageMaker 1.0 or higher"
      << std::endl);
    return 0;
    }
  cmCPackLogger(cmCPackLog::LOG_DEBUG, "PackageMaker version is: "
    << this->PackageMakerVersion << std::endl);

  pkgPath += "/MacOS";
  path.push_back(pkgPath);
  pkgPath = cmSystemTools::FindProgram("PackageMaker", path, false);
  if ( pkgPath.empty() )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Cannot find PackageMaker compiler"
      << std::endl);
    return 0;
    }
  this->SetOptionIfNotSet("CPACK_INSTALLER_PROGRAM", pkgPath.c_str());
  pkgPath = cmSystemTools::FindProgram("hdiutil", path, false);
  if ( pkgPath.empty() )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Cannot find hdiutil compiler"
      << std::endl);
    return 0;
    }
  this->SetOptionIfNotSet("CPACK_INSTALLER_PROGRAM_DISK_IMAGE", 
                          pkgPath.c_str());

  return this->Superclass::InitializeInternal();
}

//----------------------------------------------------------------------
bool cmCPackPackageMakerGenerator::CopyCreateResourceFile(const char* name)
{
  std::string uname = cmSystemTools::UpperCase(name);
  std::string cpackVar = "CPACK_RESOURCE_FILE_" + uname;
  const char* inFileName = this->GetOption(cpackVar.c_str());
  if ( !inFileName )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "CPack option: " << cpackVar.c_str()
                  << " not specified. It should point to " 
                  << (name ? name : "(NULL)")
                  << ".rtf, " << name
                  << ".html, or " << name << ".txt file" << std::endl);
    return false;
    }
  if ( !cmSystemTools::FileExists(inFileName) )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Cannot find " 
                  << (name ? name : "(NULL)")
                  << " resource file: " << inFileName << std::endl);
    return false;
    }
  std::string ext = cmSystemTools::GetFilenameLastExtension(inFileName);
  if ( ext != ".rtfd" && ext != ".rtf" && ext != ".html" && ext != ".txt" )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Bad file extension specified: "
      << ext << ". Currently only .rtfd, .rtf, .html, and .txt files allowed."
      << std::endl);
    return false;
    }

  std::string destFileName = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
  destFileName += "/Resources/";
  destFileName += name + ext;


  cmCPackLogger(cmCPackLog::LOG_VERBOSE, "Configure file: " 
                << (inFileName ? inFileName : "(NULL)")
                << " to " << destFileName.c_str() << std::endl);
  this->ConfigureFile(inFileName, destFileName.c_str());
  return true;
}

bool cmCPackPackageMakerGenerator::CopyResourcePlistFile(const char* name,
                                                         const char* outName)
{
  if (!outName)
    {
    outName = name;
    }

  std::string inFName = "CPack.";
  inFName += name;
  inFName += ".in";
  std::string inFileName = this->FindTemplate(inFName.c_str());
  if ( inFileName.empty() )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Cannot find input file: "
      << inFName << std::endl);
    return false;
    }

  std::string destFileName = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
  destFileName += "/";
  destFileName += outName;

  cmCPackLogger(cmCPackLog::LOG_VERBOSE, "Configure file: "
    << inFileName.c_str() << " to " << destFileName.c_str() << std::endl);
  this->ConfigureFile(inFileName.c_str(), destFileName.c_str());
  return true;
}

//----------------------------------------------------------------------
bool cmCPackPackageMakerGenerator::RunPackageMaker(const char *command,
                                                   const char *packageFile)
{
  std::string tmpFile = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
  tmpFile += "/PackageMakerOutput.log";

  cmCPackLogger(cmCPackLog::LOG_VERBOSE, "Execute: " << command << std::endl);
  std::string output;
  int retVal = 1;
  bool res = cmSystemTools::RunSingleCommand(command, &output, &retVal, 0, 
                                             this->GeneratorVerbose, 0);
  cmCPackLogger(cmCPackLog::LOG_VERBOSE, "Done running package maker"
    << std::endl);
  if ( !res || retVal )
    {
    cmGeneratedFileStream ofs(tmpFile.c_str());
    ofs << "# Run command: " << command << std::endl
      << "# Output:" << std::endl
      << output.c_str() << std::endl;
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Problem running PackageMaker command: " << command
      << std::endl << "Please check " << tmpFile.c_str() << " for errors"
      << std::endl);
    return false;
    }
  // sometimes the command finishes but the directory is not yet
  // created, so try 10 times to see if it shows up
  int tries = 10;
  while(tries > 0 && 
        !cmSystemTools::FileExists(packageFile))
    {
    cmSystemTools::Delay(500);
    tries--;
    }
  if(!cmSystemTools::FileExists(packageFile))
    {
    cmCPackLogger(
      cmCPackLog::LOG_ERROR,
      "Problem running PackageMaker command: " << command
      << std::endl << "Package not created: " << packageFile
      << std::endl);
    return false;
    }

  return true;
}

//----------------------------------------------------------------------
std::string 
cmCPackPackageMakerGenerator::GetPackageName(const cmCPackComponent& component)
{
  std::string packagesDir = this->GetOption("CPACK_TEMPORARY_DIRECTORY");
  packagesDir += ".dummy";
  cmOStringStream out;
  out << cmSystemTools::GetFilenameWithoutLastExtension(packagesDir)
      << "-" << component.Name << ".pkg";
  return out.str();
}

//----------------------------------------------------------------------
bool
cmCPackPackageMakerGenerator::
GenerateComponentPackage(const char *packageFile,
                         const char *packageDir,
                         const cmCPackComponent& component)
{
  cmCPackLogger(cmCPackLog::LOG_OUTPUT,
                "-   Building component package: " << packageFile << std::endl);

  // Create the description file for this component.
  std::string descriptionFile = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
  descriptionFile += '/' + component.Name + "-Description.plist";
  std::ofstream out(descriptionFile.c_str());
  out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl
      << "<!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\""
      << "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">" << std::endl
      << "<plist version=\"1.4\">" << std::endl
      << "<dict>" << std::endl
      << "  <key>IFPkgDescriptionTitle</key>" << std::endl
      << "  <string>" << component.DisplayName << "</string>" << std::endl
      << "  <key>IFPkgDescriptionVersion</key>" << std::endl
      << "  <string>" << this->GetOption("CPACK_PACKAGE_VERSION") 
      << "</string>" << std::endl
      << "  <key>IFPkgDescriptionDescription</key>" << std::endl
      << "  <string>" + this->EscapeForXML(component.Description) 
      << "</string>" << std::endl
      << "</dict>" << std::endl
      << "</plist>" << std::endl;
  out.close();

  // Create the Info.plist file for this component
  std::string moduleVersionSuffix = ".";
  moduleVersionSuffix += component.Name;
  this->SetOption("CPACK_MODULE_VERSION_SUFFIX", moduleVersionSuffix.c_str());
  std::string infoFileName = component.Name;
  infoFileName += "-Info.plist";
  if (!this->CopyResourcePlistFile("Info.plist", infoFileName.c_str()))
    {
    return false;
    }

  // Run PackageMaker  
  cmOStringStream pkgCmd;
  pkgCmd << "\"" << this->GetOption("CPACK_INSTALLER_PROGRAM")
         << "\" -build -p \"" << packageFile << "\""
         << " -f \"" << packageDir << "\""
         << "-i \"" << this->GetOption("CPACK_TOPLEVEL_DIRECTORY") 
         << "/" << infoFileName << "\""
         << "-d \"" << descriptionFile << "\""; 
  return RunPackageMaker(pkgCmd.str().c_str(), packageFile);
}

//----------------------------------------------------------------------
void 
cmCPackPackageMakerGenerator::
WriteDistributionFile(const char* metapackageFile)
{
  std::string distributionTemplate 
    = this->FindTemplate("CPack.distribution.dist.in");
  if ( distributionTemplate.empty() )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Cannot find input file: "
      << distributionTemplate << std::endl);
    return;
    }

  std::string distributionFile = metapackageFile;
  distributionFile += "/Contents/distribution.dist";

  // Create the choice outline, which provides a tree-based view of
  // the components in their groups.
  cmOStringStream choiceOut;
  choiceOut << "<choices-outline>" << std::endl;

  // Emit the outline for the groups
  std::map<std::string, cmCPackComponentGroup>::iterator groupIt;
  for (groupIt = this->ComponentGroups.begin(); 
       groupIt != this->ComponentGroups.end(); 
       ++groupIt)
    {
    CreateChoiceOutline(groupIt->second, choiceOut);
    }

  // Emit the outline for the non-grouped components
  std::map<std::string, cmCPackComponent>::iterator compIt;
  for (compIt = this->Components.begin(); compIt != this->Components.end();
       ++compIt)
    {
    if (!compIt->second.Group)
      {
      choiceOut << "<line choice=\"" << compIt->first << "Choice\"></line>"
                << std::endl;
      }
    }
  choiceOut << "</choices-outline>" << std::endl;

  // Create the actual choices
  for (groupIt = this->ComponentGroups.begin(); 
       groupIt != this->ComponentGroups.end(); 
       ++groupIt)
    {
    CreateChoice(groupIt->second, choiceOut);
    }
  for (compIt = this->Components.begin(); compIt != this->Components.end();
       ++compIt)
    {
    CreateChoice(compIt->second, choiceOut);
    }
  this->SetOption("CPACK_PACKAGEMAKER_CHOICES", choiceOut.str().c_str());

  // Create the distribution.dist file in the metapackage to turn it
  // into a distribution package.
  this->ConfigureFile(distributionTemplate.c_str(), 
                      distributionFile.c_str());
}

//----------------------------------------------------------------------
void
cmCPackPackageMakerGenerator::
CreateChoiceOutline(const cmCPackComponentGroup& group, cmOStringStream& out)
{
  out << "<line choice=\"" << group.Name << "Choice\">" << std::endl;
  std::vector<cmCPackComponent*>::const_iterator compIt;
  for (compIt = group.Components.begin(); compIt != group.Components.end();
       ++compIt)
    {
    out << "  <line choice=\"" << (*compIt)->Name << "Choice\"></line>"
        << std::endl;
    }
  out << "</line>" << std::endl;
}

//----------------------------------------------------------------------
void 
cmCPackPackageMakerGenerator::CreateChoice(const cmCPackComponentGroup& group,
                                           cmOStringStream& out)
{
  out << "<choice id=\"" << group.Name << "Choice\" " 
      << "title=\"" << group.DisplayName << "\" "
      << "start_selected=\"true\" " 
      << "start_enabled=\"true\" "
      << "start_visible=\"true\" ";
  if (!group.Description.empty())
    {
    out << "description=\"" << EscapeForXML(group.Description)
        << "\"";
    }
  out << "></choice>" << std::endl;
}

//----------------------------------------------------------------------
void 
cmCPackPackageMakerGenerator::CreateChoice(const cmCPackComponent& component,
                                           cmOStringStream& out)
{
  std::string packageId = "com.";
  packageId += this->GetOption("CPACK_PACKAGE_VENDOR");
  packageId += '.'; 
  packageId += this->GetOption("CPACK_PACKAGE_NAME");
  packageId += '.';
  packageId += this->GetOption("CPACK_PACKAGE_VERSION");
  packageId += '.';
  packageId += component.Name;

  out << "<choice id=\"" << component.Name << "Choice\" " 
      << "title=\"" << component.DisplayName << "\" "
      << "start_selected=\"" 
      << (component.IsDisabledByDefault && !component.IsRequired? "false" : "true")
      << "\" "
      << "start_enabled=\""
      << (component.IsRequired? "false" : "true")
      << "\" "
      << "start_visible=\"" << (component.IsHidden? "false" : "true") << "\" ";
  if (!component.Description.empty())
    {
    out << "description=\"" << EscapeForXML(component.Description)
        << "\" ";
    }
  if (!component.Dependencies.empty() || !component.ReverseDependencies.empty())
    {
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
    AddDependencyAttributes(component, out);
    AddReverseDependencyAttributes(component, out);
    out << "\"";
    }
  out << ">" << std::endl;
  out << "  <pkg-ref id=\"" << packageId << "\"></pkg-ref>" << std::endl;
  out << "</choice>" << std::endl;

  // Create a description of the package associated with this
  // component.
  std::string relativePackageLocation = "Contents/Packages/";
  relativePackageLocation += GetPackageName(component);

  // Determine the installed size of the package. To do so, we dig
  // into the Info.plist file from the generated package to retrieve
  // this size.
  int installedSize = 0;
  std::string infoPlistFile = this->GetOption("CPACK_TEMPORARY_DIRECTORY");
  infoPlistFile += ".mpkg/";
  infoPlistFile += relativePackageLocation;
  infoPlistFile += "/Contents/Info.plist";
  bool foundFlagInstalledSize = false;
  std::string line;
  std::ifstream ifs(infoPlistFile.c_str());
  while ( cmSystemTools::GetLineFromStream(ifs, line) )
    {
      if (foundFlagInstalledSize)
        {
        std::string::size_type pos = line.find("<integer>");
        if (pos == std::string::npos)
          {
          cmCPackLogger(cmCPackLog::LOG_ERROR, "Cannot parse package size in "
                        << infoPlistFile << std::endl
                        << "String is \"" << line << "\"" << std::endl);
          }
        else
          {
          line.erase(0, pos + 9);
          pos = line.find("</integer>");
          if (pos == std::string::npos)
            {
            cmCPackLogger(cmCPackLog::LOG_ERROR, "Cannot parse package size in "
                          << infoPlistFile << std::endl);
            }
          else
            {
            line.erase(pos, std::string::npos);
            installedSize = atoi(line.c_str());
            }
          }
        foundFlagInstalledSize = false;
        }
      else 
        {
        foundFlagInstalledSize 
          = line.find("IFPkgFlagInstalledSize") != std::string::npos;
        }
    }
  

  out << "<pkg-ref id=\"" << packageId << "\" "
      << "version=\"" << this->GetOption("CPACK_PACKAGE_VERSION") << "\" "
      << "installKBytes=\"" << installedSize << "\" "
      << "auth=\"Admin\" onConclusion=\"None\">"
      << "file:./" << relativePackageLocation << "</pkg-ref>" << std::endl;
}

//----------------------------------------------------------------------
void 
cmCPackPackageMakerGenerator::
AddDependencyAttributes(const cmCPackComponent& component, cmOStringStream& out)
{
  std::vector<cmCPackComponent *>::const_iterator dependIt;
  for (dependIt = component.Dependencies.begin();
       dependIt != component.Dependencies.end();
       ++dependIt)
    {
    out << " &amp;&amp; choices['" << (*dependIt)->Name << "Choice'].selected";
    AddDependencyAttributes(**dependIt, out);
    }
}

//----------------------------------------------------------------------
void 
cmCPackPackageMakerGenerator::
AddReverseDependencyAttributes(const cmCPackComponent& component, 
                               cmOStringStream& out)
{
  std::vector<cmCPackComponent *>::const_iterator dependIt;
  for (dependIt = component.ReverseDependencies.begin();
       dependIt != component.ReverseDependencies.end();
       ++dependIt)
    {
    out << " || choices['" << (*dependIt)->Name << "Choice'].selected";
    AddReverseDependencyAttributes(**dependIt, out);
    }
}

//----------------------------------------------------------------------
std::string cmCPackPackageMakerGenerator::EscapeForXML(std::string str)
{
  cmSystemTools::ReplaceString(str, "&", "&amp;");
  cmSystemTools::ReplaceString(str, "<", "&lt;");
  cmSystemTools::ReplaceString(str, ">", "&gt;");
  cmSystemTools::ReplaceString(str, "\"", "&quot;");
  return str;
}
