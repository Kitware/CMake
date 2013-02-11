/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2013 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmTargetPropCommandBase.h"

#include "cmGlobalGenerator.h"

//----------------------------------------------------------------------------
bool cmTargetPropCommandBase
::HandleArguments(std::vector<std::string> const& args, const char *prop,
                 ArgumentFlags flags)
{
  if(args.size() < 3)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // Lookup the target for which libraries are specified.
  this->Target =
    this->Makefile->GetCMakeInstance()
    ->GetGlobalGenerator()->FindTarget(0, args[0].c_str());
  if(!this->Target)
    {
    this->Target = this->Makefile->FindTargetToUse(args[0].c_str());
    }
  if(!this->Target)
    {
    this->HandleMissingTarget(args[0]);
    return false;
    }

  unsigned int argIndex = 1;

  bool prepend = false;
  if ((flags & PROCESS_BEFORE) && args[argIndex] == "BEFORE")
    {
    if (args.size() < 4)
      {
      this->SetError("called with incorrect number of arguments");
      return false;
      }
    prepend = true;
    ++argIndex;
    }

  this->Property = prop;

  while (argIndex < args.size())
    {
    if (!this->ProcessContentArgs(args, argIndex, prepend))
      {
      return false;
      }
    }
  return true;
}

//----------------------------------------------------------------------------
static bool isGeneratorExpression(const std::string &lib)
{
  const std::string::size_type openpos = lib.find("$<");
  return (openpos != std::string::npos)
      && (lib.find(">", openpos) != std::string::npos);
}

//----------------------------------------------------------------------------
bool cmTargetPropCommandBase
::ProcessContentArgs(std::vector<std::string> const& args,
                     unsigned int &argIndex, bool prepend)
{
  const std::string scope = args[argIndex];

  if(scope != "PUBLIC"
      && scope != "PRIVATE"
      && scope != "INTERFACE" )
    {
    this->SetError("called with invalid arguments");
    return false;
    }

  if(this->Target->IsImported())
    {
    this->HandleImportedTarget(args[0]);
    return false;
    }

  ++argIndex;

  std::string content;

  std::string sep;
  for(unsigned int i=argIndex; i < args.size(); ++i, ++argIndex)
    {
    if(args[i] == "PUBLIC"
        || args[i] == "PRIVATE"
        || args[i] == "INTERFACE" )
      {
      this->PopulateTargetProperies(scope, content, prepend);
      return true;
      }
    if (this->Makefile->FindTargetToUse(args[i].c_str()))
      {
      content += sep + "$<TARGET_PROPERTY:" + args[i]
                      + ",INTERFACE_" + this->Property + ">";
      }
    else if(isGeneratorExpression(args[i]))
      {
      content += sep + args[i];
      }
    else if (!this->HandleNonTargetArg(content, sep, args[i], args[0]))
      {
      return false;
      }
    sep = ";";
    }
  this->PopulateTargetProperies(scope, content, prepend);
  return true;
}

//----------------------------------------------------------------------------
void cmTargetPropCommandBase
::PopulateTargetProperies(const std::string &scope,
                          const std::string &content, bool prepend)
{
  if (scope == "PRIVATE" || scope == "PUBLIC")
    {
    this->HandleDirectContent(this->Target, content, prepend);
    }
  if (scope == "INTERFACE" || scope == "PUBLIC")
    {
    if (prepend)
      {
      const std::string propName = std::string("INTERFACE_") + this->Property;
      const char *propValue = this->Target->GetProperty(propName.c_str());
      const std::string totalContent = content + (propValue
                                                ? std::string(";") + propValue
                                                : std::string());
      this->Target->SetProperty(propName.c_str(), totalContent.c_str());
      }
    else
      {
      this->Target->AppendProperty(("INTERFACE_" + this->Property).c_str(),
                            content.c_str());
      }
    }
}
