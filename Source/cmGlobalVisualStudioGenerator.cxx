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
#include "cmGlobalVisualStudioGenerator.h"

#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmTarget.h"

//----------------------------------------------------------------------------
cmGlobalVisualStudioGenerator::cmGlobalVisualStudioGenerator()
{
}

//----------------------------------------------------------------------------
cmGlobalVisualStudioGenerator::~cmGlobalVisualStudioGenerator()
{
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudioGenerator::Generate()
{
  // Add a special target that depends on ALL projects for easy build
  // of one configuration only.
  const char* no_working_dir = 0;
  std::vector<std::string> no_depends;
  cmCustomCommandLines no_commands;
  std::map<cmStdString, std::vector<cmLocalGenerator*> >::iterator it;
  for(it = this->ProjectMap.begin(); it!= this->ProjectMap.end(); ++it)
    {
    std::vector<cmLocalGenerator*>& gen = it->second;
    // add the ALL_BUILD to the first local generator of each project
    if(gen.size())
      {
      // Use no actual command lines so that the target itself is not
      // considered always out of date.
      gen[0]->GetMakefile()->
        AddUtilityCommand("ALL_BUILD", true, no_working_dir,
                          no_depends, no_commands, false,
                          "Build all projects");
      }
    }

  // Fix utility dependencies to avoid linking to libraries.
  this->FixUtilityDepends();

  // Run all the local generators.
  this->cmGlobalGenerator::Generate();
}

//----------------------------------------------------------------------------
void cmGlobalVisualStudioGenerator::FixUtilityDepends()
{
  // For VS versions before 8:
  //
  // When a target that links contains a project-level dependency on a
  // library target that library is automatically linked.  In order to
  // allow utility-style project-level dependencies that do not
  // actually link we need to automatically insert an intermediate
  // custom target.
  //
  // Here we edit the utility dependencies of a target to add the
  // intermediate custom target when necessary.
  for(unsigned i = 0; i < this->LocalGenerators.size(); ++i)
    {
    cmTargets* targets =
      &(this->LocalGenerators[i]->GetMakefile()->GetTargets());
    for(cmTargets::iterator tarIt = targets->begin();
        tarIt != targets->end(); ++tarIt)
      {
      this->FixUtilityDependsForTarget(tarIt->second);
      }
    }
}

//----------------------------------------------------------------------------
void
cmGlobalVisualStudioGenerator::FixUtilityDependsForTarget(cmTarget& target)
{
  // Only targets that link need to be fixed.
  if(target.GetType() != cmTarget::STATIC_LIBRARY &&
     target.GetType() != cmTarget::SHARED_LIBRARY &&
     target.GetType() != cmTarget::MODULE_LIBRARY &&
     target.GetType() != cmTarget::EXECUTABLE)
    {
    return;
    }

  // Look at each utility dependency.
  for(std::set<cmStdString>::const_iterator ui =
        target.GetUtilities().begin();
      ui != target.GetUtilities().end(); ++ui)
    {
    if(cmTarget* depTarget = this->FindTarget(0, ui->c_str(), false))
      {
      if(depTarget->GetType() == cmTarget::STATIC_LIBRARY ||
         depTarget->GetType() == cmTarget::SHARED_LIBRARY ||
         depTarget->GetType() == cmTarget::MODULE_LIBRARY)
        {
        // This utility dependency will cause an attempt to link.  If
        // the depender does not already link the dependee we need an
        // intermediate target.
        if(!this->CheckTargetLinks(target, ui->c_str()))
          {
          this->CreateUtilityDependTarget(*depTarget);
          }
        }
      }
    }
}

//----------------------------------------------------------------------------
void
cmGlobalVisualStudioGenerator::CreateUtilityDependTarget(cmTarget& target)
{
  // This target is a library on which a utility dependency exists.
  // We need to create an intermediate custom target to hook up the
  // dependency without causing a link.
  const char* altName = target.GetProperty("ALTERNATIVE_DEPENDENCY_NAME");
  if(!altName)
    {
    // Create the intermediate utility target.
    std::string altNameStr = target.GetName();
    altNameStr += "_UTILITY";
    const std::vector<std::string> no_depends;
    cmCustomCommandLines no_commands;
    const char* no_working_dir = 0;
    const char* no_comment = 0;
    target.GetMakefile()->AddUtilityCommand(altNameStr.c_str(), true,
                                            no_working_dir, no_depends,
                                            no_commands, false, no_comment);
    target.SetProperty("ALTERNATIVE_DEPENDENCY_NAME", altNameStr.c_str());

    // Most targets have a GUID created in ConfigureFinalPass.  Since
    // that has already been called, create one for this target now.
    this->CreateGUID(altNameStr.c_str());

    // The intermediate target should depend on the original target.
    if(cmTarget* alt = this->FindTarget(0, altNameStr.c_str(), false))
      {
      alt->AddUtility(target.GetName());
      }
    }
}

//----------------------------------------------------------------------------
bool cmGlobalVisualStudioGenerator::CheckTargetLinks(cmTarget& target,
                                                     const char* name)
{
  // Return whether the given target links to a target with the given name.
  if(target.GetType() == cmTarget::STATIC_LIBRARY)
    {
    // Static libraries never link to anything.
    return false;
    }
  cmTarget::LinkLibraryVectorType const& libs = target.GetLinkLibraries();
  for(cmTarget::LinkLibraryVectorType::const_iterator i = libs.begin();
      i != libs.end(); ++i)
    {
    if(i->first == name)
      {
      return true;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
const char*
cmGlobalVisualStudioGenerator::GetUtilityForTarget(cmTarget& target,
                                                   const char* name)
{
  // Handle the external MS project special case.
  if(strncmp(name, "INCLUDE_EXTERNAL_MSPROJECT", 26) == 0)
    {
    // Note from Ken:
    // kind of weird removing the first 27 letters.  my
    // recommendatsions: use cmCustomCommand::GetCommand() to get the
    // project name or get rid of the target name starting with
    // "INCLUDE_EXTERNAL_MSPROJECT_" and use another indicator/flag
    // somewhere.  These external project names shouldn't conflict
    // with cmake target names anyways.
    return name+27;
    }

  // Possibly depend on an intermediate utility target to avoid
  // linking.
  if(target.GetType() == cmTarget::STATIC_LIBRARY ||
     target.GetType() == cmTarget::SHARED_LIBRARY ||
     target.GetType() == cmTarget::MODULE_LIBRARY ||
     target.GetType() == cmTarget::EXECUTABLE)
    {
    // The depender is a target that links.  Lookup the dependee to
    // see if it provides an alternative dependency name.
    if(cmTarget* depTarget = this->FindTarget(0, name, false))
      {
      // Check for an alternative name created by FixUtilityDepends.
      if(const char* altName =
         depTarget->GetProperty("ALTERNATIVE_DEPENDENCY_NAME"))
        {
        // The alternative name is needed only if the depender does
        // not really link to the dependee.
        if(!this->CheckTargetLinks(target, name))
          {
          return altName;
          }
        }
      }
    }

  // No special case.  Just use the original dependency name.
  return name;
}
