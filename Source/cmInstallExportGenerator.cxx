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

#include <stdio.h>

#include "cmInstallTargetGenerator.h"
#include "cmGeneratedFileStream.h"
#include "cmTarget.h"

#include "cmInstallExportGenerator.h"

cmInstallExportGenerator::cmInstallExportGenerator(
  const char* destination,
  const char* file_permissions,
  std::vector<std::string> const& configurations,
  const char* component,
  const char* filename, const char* prefix, const char* tempOutputDir)
  :cmInstallGenerator(destination, configurations, component)
  ,FilePermissions(file_permissions)
  ,Filename(filename)
  ,Prefix(prefix)
  ,TempOutputDir(tempOutputDir)
{
}

/* Helper function which adds the install locations from the generator
to the properties for this target.
*/
bool cmInstallExportGenerator::AddInstallLocations(cmTargetWithProperties* twp,
                                           cmInstallTargetGenerator* generator,
                                           const char* prefix)
{
  if (generator == 0) // nothing to do
    {
    return true;
    }

  if (prefix == 0)
    {
    prefix = "";
    }

  const std::vector<std::string>& configs = generator->GetConfigurations();
  if (configs.empty())
    {
    std::string propertyName = prefix;
    propertyName += "LOCATION";
    // check that this property doesn't exist yet and add it then
    if (twp->Properties.find(propertyName.c_str())== twp->Properties.end())
      {
      std::string destinationFilename = generator->GetDestination();
      destinationFilename += "/";
      destinationFilename += generator->GetInstallFilename(0);
      twp->Properties[propertyName.c_str()] = destinationFilename;
      }
    else
      {
      return false;
      }
    }
  else
    {
    for(std::vector<std::string>::const_iterator configIt = configs.begin();
        configIt != configs.end();
        ++configIt)
      {
      std::string propertyName = configIt->c_str();
      propertyName +=  "_";
      propertyName += prefix;
      propertyName += "LOCATION";
      // check that this property doesn't exist yet and add it then
      if (twp->Properties.find(propertyName.c_str())== twp->Properties.end())
        {
        std::string destinationFilename = generator->GetDestination();
        destinationFilename += "/";
        destinationFilename +=generator->GetInstallFilename(configIt->c_str());
        twp->Properties[propertyName.c_str()] = destinationFilename;
        }
      else
        {
        return false;
        }
      }
    }
  return true;
}


bool cmInstallExportGenerator::SetExportSet(const char* name,
                                       const std::vector<cmTargetExport*>* set)
{
  if ((name == 0) || (*name == 0) || (set==0))
    {
    return false;
    }

  this->Name = name;

  /* iterate over all targets in the set.
  If a cmTargetWithProperties with the same name already exists in this 
  generator, add the new properties to it. If the property already exists, 
  fail with an error.
  If no cmTargetWithProperties exists, create a new one.
  */
  for(std::vector<cmTargetExport*>::const_iterator it=set->begin();
      it != set->end();
      ++it)
    {
    std::string targetName = (*it)->Target->GetName();

    cmTargetWithProperties* targetWithProps = 0;
    for(unsigned int i=0; i<this->Targets.size(); i++)
      {
      if (targetName == this->Targets[i]->Target->GetName())
        {
        targetWithProps = this->Targets[i];
        }
      }

    if (targetWithProps == 0)
      {
      targetWithProps = new cmTargetWithProperties((*it)->Target);
      this->Targets.push_back(targetWithProps);
      }

    if (this->AddInstallLocations(targetWithProps, (*it)->ArchiveGenerator, 
                                  "ARCHIVE_") == false)
      {
      return false;
      }
    if (this->AddInstallLocations(targetWithProps, (*it)->RuntimeGenerator, 
                                  "") == false)
      {
      return false;
      }
    if (this->AddInstallLocations(targetWithProps, (*it)->LibraryGenerator, 
                                  "LIBRARY_") == false)
      {
      return false;
      }
    }

  return true;
}

void cmInstallExportGenerator::GenerateScript(std::ostream& os)
{
  // for the case that somebody exports the same set with the same file name 
  // to different locations make the temp filename unique
  char buf[64];
  sprintf(buf, "%p", this);
  this->ExportFilename = this->TempOutputDir;
  this->ExportFilename += "/";
  this->ExportFilename += this->Filename;
  this->ExportFilename += ".";
  this->ExportFilename += buf;

  cmGeneratedFileStream exportFileStream(this->ExportFilename.c_str());
  if(!exportFileStream)
    {
    return;
    }

  /* for every target add the IMPORT statements and set the properties
    of the target.  */
  for(std::vector<cmTargetWithProperties*>::const_iterator 
      targetIt = this->Targets.begin();
      targetIt != this->Targets.end();
      ++targetIt)
    {
      switch ((*targetIt)->Target->GetType())
        {
        case cmTarget::EXECUTABLE:
          exportFileStream << "ADD_EXECUTABLE(" << this->Prefix.c_str() 
                           << (*targetIt)->Target->GetName() 
                           << " IMPORT )\n";
          break;
        case cmTarget::STATIC_LIBRARY:
          exportFileStream << "ADD_LIBRARY(" << this->Prefix.c_str() 
                           << (*targetIt)->Target->GetName() 
                           << " STATIC IMPORT )\n";
          break;
        case cmTarget::SHARED_LIBRARY:
          exportFileStream << "ADD_LIBRARY(" << this->Prefix.c_str() 
                           << (*targetIt)->Target->GetName()
                           << " SHARED IMPORT )\n";
          break;
        case cmTarget::MODULE_LIBRARY:
          exportFileStream << "ADD_LIBRARY(" << this->Prefix.c_str() 
                           << (*targetIt)->Target->GetName()
                           << " MODULE IMPORT )\n";
          break;
        default:  // should never happen
          break;
        }

      exportFileStream << "SET_TARGET_PROPERTIES ( " << this->Prefix.c_str() 
                       << (*targetIt)->Target->GetName() << " PROPERTIES \n";

      for (std::map<std::string, std::string>::const_iterator
           propIt = (*targetIt)->Properties.begin();
           propIt != (*targetIt)->Properties.end();
           ++propIt)
        {
        exportFileStream << "                     " << propIt->first 
                         << " \"" << propIt->second << "\"\n";
        }
      exportFileStream << "                      )\n\n";
    }

  // Perform the main install script generation.
  this->cmInstallGenerator::GenerateScript(os);
}

//----------------------------------------------------------------------------
void cmInstallExportGenerator::GenerateScriptActions(std::ostream& os,
                                                     Indent const& indent)
{
  // install rule for the file created above
  std::vector<std::string> exportFile;
  exportFile.push_back(this->ExportFilename);
  this->AddInstallRule(os, this->Destination.c_str(), cmTarget::INSTALL_FILES,
                       exportFile, false, 0,
                       this->FilePermissions.c_str(),
                       0, this->Filename.c_str(), 0, indent);
}
