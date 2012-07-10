/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmIncludeCommand.h"


// cmIncludeCommand
bool cmIncludeCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if (args.size()< 1 || args.size() > 4)
    {
      this->SetError("called with wrong number of arguments.  "
                     "Include only takes one file.");
      return false;
    }
  bool optional = false;
  bool noPolicyScope = false;
  std::string fname = args[0];
  std::string resultVarName;

  for (unsigned int i=1; i<args.size(); i++)
    {
    if (args[i] == "OPTIONAL")
      {
      if (optional)
        {
        this->SetError("called with invalid arguments: OPTIONAL used twice");
        return false;
        }
      optional = true;
      }
    else if(args[i] == "RESULT_VARIABLE")
      {
      if (resultVarName.size() > 0)
        {
        this->SetError("called with invalid arguments: "
            "only one result variable allowed");
        return false;
        }
      if(++i < args.size())
        {
        resultVarName = args[i];
        }
      else
        {
        this->SetError("called with no value for RESULT_VARIABLE.");
        return false;
        }
      }
    else if(args[i] == "NO_POLICY_SCOPE")
      {
      noPolicyScope = true;
      }
      else if(i > 1)  // compat.: in previous cmake versions the second
                      // parameter was ignore if it wasn't "OPTIONAL"
        {
        std::string errorText = "called with invalid argument: ";
        errorText += args[i];
        this->SetError(errorText.c_str());
        return false;
        }
    }

  if(fname.empty())
    {
    this->Makefile->IssueMessage(cmake::AUTHOR_WARNING,
                                 "include() given empty file name (ignored).");
    return true;
    }

  if(!cmSystemTools::FileIsFullPath(fname.c_str()))
    {
    // Not a path. Maybe module.
    std::string module = fname;
    module += ".cmake";
    std::string mfile = this->Makefile->GetModulesFile(module.c_str());
    if ( mfile.size() )
      {
      fname = mfile.c_str();
      }
    }
  std::string fullFilePath;
  bool readit =
    this->Makefile->ReadListFile( this->Makefile->GetCurrentListFile(),
                                  fname.c_str(), &fullFilePath,
                                  noPolicyScope);

  // add the location of the included file if a result variable was given
  if (resultVarName.size())
    {
      this->Makefile->AddDefinition(resultVarName.c_str(),
                                    readit?fullFilePath.c_str():"NOTFOUND");
    }

  if(!optional && !readit && !cmSystemTools::GetFatalErrorOccured())
    {
    std::string m =
      "could not find load file:\n"
      "  ";
    m += fname;
    this->SetError(m.c_str());
    return false;
    }
  return true;
}


