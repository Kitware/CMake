/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmSetPropertyCommand.h"
#include "cmSetTargetPropertiesCommand.h"
#include "cmSetTestsPropertiesCommand.h"
#include "cmSetSourceFilesPropertiesCommand.h"

#include "cmCacheManager.h"

//----------------------------------------------------------------------------
cmSetPropertyCommand::cmSetPropertyCommand()
{
  this->AppendMode = false;
  this->AppendAsString = false;
  this->Remove = true;
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
  else if(*arg == "CACHE")
    {
    scope = cmProperty::CACHE;
    }
  else
    {
    cmOStringStream e;
    e << "given invalid scope " << *arg << ".  "
      << "Valid scopes are GLOBAL, DIRECTORY, TARGET, SOURCE, TEST, CACHE.";
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
      this->AppendAsString = false;
      }
    else if(*arg == "APPEND_STRING")
      {
      doing = DoingNone;
      this->AppendMode = true;
      this->AppendAsString = true;
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
      this->Remove = false;
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
    case cmProperty::CACHE:       return this->HandleCacheMode();

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
  const char *value = this->PropertyValue.c_str();
  if (this->Remove)
    {
    value = 0;
    }
  if(this->AppendMode)
    {
    cm->AppendProperty(name, value, this->AppendAsString);
    }
  else
    {
    cm->SetProperty(name, value);
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
  const char *value = this->PropertyValue.c_str();
  if (this->Remove)
    {
    value = 0;
    }
  if(this->AppendMode)
    {
    mf->AppendProperty(name, value, this->AppendAsString);
    }
  else
    {
    mf->SetProperty(name, value);
    }

  return true;
}

//----------------------------------------------------------------------------
bool cmSetPropertyCommand::HandleTargetMode()
{
  for(std::set<cmStdString>::const_iterator ni = this->Names.begin();
      ni != this->Names.end(); ++ni)
    {
    if(cmTarget* target = this->Makefile->FindTargetToUse(ni->c_str()))
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
  const char *value = this->PropertyValue.c_str();
  if (this->Remove)
    {
    value = 0;
    }
  if(this->AppendMode)
    {
    target->AppendProperty(name, value, this->AppendAsString);
    }
  else
    {
    target->SetProperty(name, value);
    }

  // Check the resulting value.
  target->CheckProperty(name, this->Makefile);

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
  const char *value = this->PropertyValue.c_str();
  if (this->Remove)
    {
    value = 0;
    }

  if(this->AppendMode)
    {
    sf->AppendProperty(name, value, this->AppendAsString);
    }
  else
    {
    sf->SetProperty(name, value);
    }
  return true;
}

//----------------------------------------------------------------------------
bool cmSetPropertyCommand::HandleTestMode()
{
  // Look for tests with all names given.
  std::set<cmStdString>::iterator next;
  for(std::set<cmStdString>::iterator ni = this->Names.begin();
      ni != this->Names.end(); ni = next)
    {
    next = ni;
    ++next;
    if(cmTest* test = this->Makefile->GetTest(ni->c_str()))
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
  const char *value = this->PropertyValue.c_str();
  if (this->Remove)
    {
    value = 0;
    }
  if(this->AppendMode)
    {
    test->AppendProperty(name, value, this->AppendAsString);
    }
  else
    {
    test->SetProperty(name, value);
    }

  return true;
}

//----------------------------------------------------------------------------
bool cmSetPropertyCommand::HandleCacheMode()
{
  if(this->PropertyName == "ADVANCED")
    {
    if(!this->Remove &&
       !cmSystemTools::IsOn(this->PropertyValue.c_str()) &&
       !cmSystemTools::IsOff(this->PropertyValue.c_str()))
      {
      cmOStringStream e;
      e << "given non-boolean value \"" << this->PropertyValue
        << "\" for CACHE property \"ADVANCED\".  ";
      this->SetError(e.str().c_str());
      return false;
      }
    }
  else if(this->PropertyName == "TYPE")
    {
    if(!cmCacheManager::IsType(this->PropertyValue.c_str()))
      {
      cmOStringStream e;
      e << "given invalid CACHE entry TYPE \"" << this->PropertyValue << "\"";
      this->SetError(e.str().c_str());
      return false;
      }
    }
  else if(this->PropertyName != "HELPSTRING" &&
          this->PropertyName != "STRINGS" &&
          this->PropertyName != "VALUE")
    {
    cmOStringStream e;
    e << "given invalid CACHE property " << this->PropertyName << ".  "
      << "Settable CACHE properties are: "
      << "ADVANCED, HELPSTRING, STRINGS, TYPE, and VALUE.";
    this->SetError(e.str().c_str());
    return false;
    }

  for(std::set<cmStdString>::const_iterator ni = this->Names.begin();
      ni != this->Names.end(); ++ni)
    {
    // Get the source file.
    cmMakefile* mf = this->GetMakefile();
    cmake* cm = mf->GetCMakeInstance();
    cmCacheManager::CacheIterator it =
      cm->GetCacheManager()->GetCacheIterator(ni->c_str());
    if(!it.IsAtEnd())
      {
      if(!this->HandleCacheEntry(it))
        {
        return false;
        }
      }
    else
      {
      cmOStringStream e;
      e << "could not find CACHE variable " << *ni
        << ".  Perhaps it has not yet been created.";
      this->SetError(e.str().c_str());
      return false;
      }
    }
  return true;
}

//----------------------------------------------------------------------------
bool cmSetPropertyCommand::HandleCacheEntry(cmCacheManager::CacheIterator& it)
{
  // Set or append the property.
  const char* name = this->PropertyName.c_str();
  const char* value = this->PropertyValue.c_str();
  if (this->Remove)
    {
    value = 0;
    }
  if(this->AppendMode)
    {
    it.AppendProperty(name, value, this->AppendAsString);
    }
  else
    {
    it.SetProperty(name, value);
    }

  return true;
}
