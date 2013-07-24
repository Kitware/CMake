/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmQTWrapCPPCommand.h"

// cmQTWrapCPPCommand
bool cmQTWrapCPPCommand::InitialPass(std::vector<std::string> const& argsIn,
                                     cmExecutionStatus &)
{
  if(argsIn.size() < 3 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // This command supports source list inputs for compatibility.
  std::vector<std::string> args;
  this->Makefile->ExpandSourceListArguments(argsIn, args, 2);

  // Get the moc executable to run in the custom command.
  const char* moc_exe =
    this->Makefile->GetRequiredDefinition("QT_MOC_EXECUTABLE");

  // Get the variable holding the list of sources.
  std::string const& sourceList = args[1];
  std::string sourceListValue =
    this->Makefile->GetSafeDefinition(sourceList.c_str());

  // Create a rule for all sources listed.
  for(std::vector<std::string>::iterator j = (args.begin() + 2);
      j != args.end(); ++j)
    {
    cmSourceFile *curr = this->Makefile->GetSource(j->c_str());
    // if we should wrap the class
    if(!(curr && curr->GetPropertyAsBool("WRAP_EXCLUDE")))
      {
      // Compute the name of the file to generate.
      std::string srcName =
        cmSystemTools::GetFilenameWithoutLastExtension(*j);
      std::string newName = this->Makefile->GetCurrentOutputDirectory();
      newName += "/moc_";
      newName += srcName;
      newName += ".cxx";
      cmSourceFile* sf =
        this->Makefile->GetOrCreateSource(newName.c_str(), true);
      if (curr)
        {
        sf->SetProperty("ABSTRACT", curr->GetProperty("ABSTRACT"));
        }

      // Compute the name of the header from which to generate the file.
      std::string hname;
      if(cmSystemTools::FileIsFullPath(j->c_str()))
        {
        hname = *j;
        }
      else
        {
        if(curr && curr->GetPropertyAsBool("GENERATED"))
          {
          hname = this->Makefile->GetCurrentOutputDirectory();
          }
        else
          {
          hname = this->Makefile->GetCurrentDirectory();
          }
        hname += "/";
        hname += *j;
        }

      // Append the generated source file to the list.
      if(!sourceListValue.empty())
        {
        sourceListValue += ";";
        }
      sourceListValue += newName;

      // Create the custom command to generate the file.
      cmCustomCommandLine commandLine;
      commandLine.push_back(moc_exe);
      commandLine.push_back("-o");
      commandLine.push_back(newName);
      commandLine.push_back(hname);

      cmCustomCommandLines commandLines;
      commandLines.push_back(commandLine);

      std::vector<std::string> depends;
      depends.push_back(moc_exe);
      depends.push_back(hname);

      const char* no_main_dependency = 0;
      const char* no_working_dir = 0;
      std::map<std::string,std::string> no_env_variables;
      this->Makefile->AddCustomCommandToOutput(newName.c_str(),
                                               depends,
                                               no_main_dependency,
                                               no_env_variables,
                                               commandLines,
                                               "Qt Wrapped File",
                                               no_working_dir);
      }
    }

  // Store the final list of source files.
  this->Makefile->AddDefinition(sourceList.c_str(),
                                sourceListValue.c_str());
  return true;
}
