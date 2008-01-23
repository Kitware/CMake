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
#include "cmSetPropertyCommand.h"
#include "cmSetTargetPropertiesCommand.h"
#include "cmSetTestsPropertiesCommand.h"
#include "cmSetSourceFilesPropertiesCommand.h"

//----------------------------------------------------------------------------
cmSetPropertyCommand::cmSetPropertyCommand()
{
  this->AppendMode = false;
}

//----------------------------------------------------------------------------
bool cmSetPropertyCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // Get the scope on which to set the property.
  std::vector<std::string>::const_iterator arg = args.begin();
  cmProperty::ScopeType scope;
  if(*arg == "GLOBAL")
    {
    scope = cmProperty::GLOBAL;
    }
  else if(*arg == "DIRECTORY")
    {
    scope = cmProperty::DIRECTORY;
    }
  else if(*arg == "TARGET")
    {
    scope = cmProperty::TARGET;
    }
  else if(*arg == "SOURCE")
    {
    scope = cmProperty::SOURCE_FILE;
    }
  else if(*arg == "TEST")
    {
    scope = cmProperty::TEST;
    }
  else
    {
    cmOStringStream e;
    e << "given invalid scope " << *arg << ".  "
      << "Valid scopes are GLOBAL, DIRECTORY, TARGET, SOURCE, TEST.";
    this->SetError(e.str().c_str());
    return false;
    }

  // Parse the rest of the arguments up to the values.
  enum Doing { DoingNone, DoingNames, DoingProperty, DoingValues };
  Doing doing = DoingNames;
  const char* sep = "";
  for(++arg; arg != args.end(); ++arg)
    {
    if(*arg == "PROPERTY")
      {
      doing = DoingProperty;
      }
    else if(*arg == "APPEND")
      {
      doing = DoingNone;
      this->AppendMode = true;
      }
    else if(doing == DoingNames)
      {
      this->Names.insert(*arg);
      }
    else if(doing == DoingProperty)
      {
      this->PropertyName = *arg;
      doing = DoingValues;
      }
    else if(doing == DoingValues)
      {
      this->PropertyValue += sep;
      sep = ";";
      this->PropertyValue += *arg;
      }
    else
      {
      cmOStringStream e;
      e << "given invalid argument \"" << *arg << "\".";
      this->SetError(e.str().c_str());
      return false;
      }
    }

  // Make sure a property name was found.
  if(this->PropertyName.empty())
    {
    this->SetError("not given a PROPERTY <name> argument.");
    return false;
    }

  // Dispatch property setting.
  switch(scope)
    {
    case cmProperty::GLOBAL:      return this->HandleGlobalMode();
    case cmProperty::DIRECTORY:   return this->HandleDirectoryMode();
    case cmProperty::TARGET:      return this->HandleTargetMode();
    case cmProperty::SOURCE_FILE: return this->HandleSourceMode();
    case cmProperty::TEST:        return this->HandleTestMode();

    case cmProperty::VARIABLE:
    case cmProperty::CACHED_VARIABLE:
      break; // should never happen
    }
  return true;
}

//----------------------------------------------------------------------------
bool cmSetPropertyCommand::HandleGlobalMode()
{
  if(!this->Names.empty())
    {
    this->SetError("given names for GLOBAL scope.");
    return false;
    }

  // Set or append the property.
  cmake* cm = this->Makefile->GetCMakeInstance();
  const char* name = this->PropertyName.c_str();
  if(this->AppendMode)
    {
    cm->AppendProperty(name, this->PropertyValue.c_str());
    }
  else
    {
    cm->SetProperty(name, this->PropertyValue.c_str());
    }

  return true;
}

//----------------------------------------------------------------------------
bool cmSetPropertyCommand::HandleDirectoryMode()
{
  if(this->Names.size() > 1)
    {
    this->SetError("allows at most one name for DIRECTORY scope.");
    return false;
    }

  // Default to the current directory.
  cmMakefile* mf = this->Makefile;

  // Lookup the directory if given.
  if(!this->Names.empty())
    {
    // Construct the directory name.  Interpret relative paths with
    // respect to the current directory.
    std::string dir = *this->Names.begin();
    if(!cmSystemTools::FileIsFullPath(dir.c_str()))
      {
      dir = this->Makefile->GetCurrentDirectory();
      dir += "/";
      dir += *this->Names.begin();
      }

    // The local generators are associated with collapsed paths.
    dir = cmSystemTools::CollapseFullPath(dir.c_str());

    // Lookup the generator.
    if(cmLocalGenerator* lg =
       (this->Makefile->GetLocalGenerator()
        ->GetGlobalGenerator()->FindLocalGenerator(dir.c_str())))
      {
      // Use the makefile for the directory found.
      mf = lg->GetMakefile();
      }
    else
      {
      // Could not find the directory.
      this->SetError
        ("DIRECTORY scope provided but requested directory was not found. "
         "This could be because the directory argument was invalid or, "
         "it is valid but has not been processed yet.");
      return false;
      }
    }

  // Set or append the property.
  const char* name = this->PropertyName.c_str();
  if(this->AppendMode)
    {
    mf->AppendProperty(name, this->PropertyValue.c_str());
    }
  else
    {
    mf->SetProperty(name, this->PropertyValue.c_str());
    }

  return true;
}

//----------------------------------------------------------------------------
bool cmSetPropertyCommand::HandleTargetMode()
{
  for(std::set<cmStdString>::const_iterator ni = this->Names.begin();
      ni != this->Names.end(); ++ni)
    {
    if(cmTarget* target =
       this->Makefile->GetLocalGenerator()->GetGlobalGenerator()
       ->FindTarget(0, ni->c_str(), true))
      {
      // Handle the current target.
      if(!this->HandleTarget(target))
        {
        return false;
        }
      }
    else
      {
      cmOStringStream e;
      e << "could not find TARGET " << *ni
        << ".  Perhaps it has not yet been created.";
      this->SetError(e.str().c_str());
      return false;
      }
    }
  return true;
}

//----------------------------------------------------------------------------
bool cmSetPropertyCommand::HandleTarget(cmTarget* target)
{
  // Set or append the property.
  const char* name = this->PropertyName.c_str();
  if(this->AppendMode)
    {
    target->AppendProperty(name, this->PropertyValue.c_str());
    }
  else
    {
    target->SetProperty(name, this->PropertyValue.c_str());
    }

  return true;
}

//----------------------------------------------------------------------------
bool cmSetPropertyCommand::HandleSourceMode()
{
  for(std::set<cmStdString>::const_iterator ni = this->Names.begin();
      ni != this->Names.end(); ++ni)
    {
    // Get the source file.
    if(cmSourceFile* sf = this->Makefile->GetOrCreateSource(ni->c_str()))
      {
      if(!this->HandleSource(sf))
        {
        return false;
        }
      }
    else
      {
      cmOStringStream e;
      e << "given SOURCE name that could not be found or created: " << *ni;
      this->SetError(e.str().c_str());
      return false;
      }
    }
  return true;
}

//----------------------------------------------------------------------------
bool cmSetPropertyCommand::HandleSource(cmSourceFile* sf)
{
  // Set or append the property.
  const char* name = this->PropertyName.c_str();
  if(this->AppendMode)
    {
    sf->AppendProperty(name, this->PropertyValue.c_str());
    }
  else
    {
    sf->SetProperty(name, this->PropertyValue.c_str());
    }

  // TODO: MACOSX_PACKAGE_LOCATION special case in
  // cmSetSourceFilesPropertiesCommand
  // The logic should be moved to cmSourceFile.

  return true;
}

//----------------------------------------------------------------------------
bool cmSetPropertyCommand::HandleTestMode()
{
  // Loop over all tests looking for matching names.
  std::vector<cmTest*> const& tests = *this->Makefile->GetTests();
  for(std::vector<cmTest*>::const_iterator ti = tests.begin();
      ti != tests.end(); ++ti)
    {
    cmTest* test = *ti;
    std::set<cmStdString>::iterator ni =
      this->Names.find(test->GetName());
    if(ni != this->Names.end())
      {
      if(this->HandleTest(test))
        {
        this->Names.erase(ni);
        }
      else
        {
        return false;
        }
      }
    }

  // Names that are still left were not found.
  if(!this->Names.empty())
    {
    cmOStringStream e;
    e << "given TEST names that do not exist:\n";
    for(std::set<cmStdString>::const_iterator ni = this->Names.begin();
        ni != this->Names.end(); ++ni)
      {
      e << "  " << *ni << "\n";
      }
    this->SetError(e.str().c_str());
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool cmSetPropertyCommand::HandleTest(cmTest* test)
{
  // Set or append the property.
  const char* name = this->PropertyName.c_str();
  if(this->AppendMode)
    {
    test->AppendProperty(name, this->PropertyValue.c_str());
    }
  else
    {
    test->SetProperty(name, this->PropertyValue.c_str());
    }

  return true;
}
