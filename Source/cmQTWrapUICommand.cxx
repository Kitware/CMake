/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmQTWrapUICommand.h"

// cmQTWrapUICommand
bool cmQTWrapUICommand::InitialPass(std::vector<std::string> const& argsIn,
                                    cmExecutionStatus &)
{
  if(argsIn.size() < 4 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // This command supports source list inputs for compatibility.
  std::vector<std::string> args;
  this->Makefile->ExpandSourceListArguments(argsIn, args, 3);

  // Get the uic and moc executables to run in the custom commands.
  const char* uic_exe =
    this->Makefile->GetRequiredDefinition("QT_UIC_EXECUTABLE");
  const char* moc_exe =
    this->Makefile->GetRequiredDefinition("QT_MOC_EXECUTABLE");

  // Get the variable holding the list of sources.
  std::string const& headerList = args[1];
  std::string const& sourceList = args[2];
  std::string headerListValue =
    this->Makefile->GetSafeDefinition(headerList.c_str());
  std::string sourceListValue =
    this->Makefile->GetSafeDefinition(sourceList.c_str());

  // Create rules for all sources listed.
  for(std::vector<std::string>::iterator j = (args.begin() + 3);
      j != args.end(); ++j)
    {
    cmSourceFile *curr = this->Makefile->GetSource(j->c_str());
    // if we should wrap the class
    if(!(curr && curr->GetPropertyAsBool("WRAP_EXCLUDE")))
      {
      // Compute the name of the files to generate.
      std::string srcName =
        cmSystemTools::GetFilenameWithoutLastExtension(*j);
      std::string hName = this->Makefile->GetCurrentOutputDirectory();
      hName += "/";
      hName += srcName;
      hName += ".h";
      std::string cxxName = this->Makefile->GetCurrentOutputDirectory();
      cxxName += "/";
      cxxName += srcName;
      cxxName += ".cxx";
      std::string mocName = this->Makefile->GetCurrentOutputDirectory();
      mocName += "/moc_";
      mocName += srcName;
      mocName += ".cxx";

      // Compute the name of the ui file from which to generate others.
      std::string uiName;
      if(cmSystemTools::FileIsFullPath(j->c_str()))
        {
        uiName = *j;
        }
      else
        {
        if(curr && curr->GetPropertyAsBool("GENERATED"))
          {
          uiName = this->Makefile->GetCurrentOutputDirectory();
          }
        else
          {
          uiName = this->Makefile->GetCurrentDirectory();
          }
        uiName += "/";
        uiName += *j;
        }

      // create the list of headers
      if(!headerListValue.empty())
        {
        headerListValue += ";";
        }
      headerListValue += hName;

      // create the list of sources
      if(!sourceListValue.empty())
        {
        sourceListValue += ";";
        }
      sourceListValue += cxxName;
      sourceListValue += ";";
      sourceListValue += mocName;

      // set up .ui to .h and .cxx command
      cmCustomCommandLine hCommand;
      hCommand.push_back(uic_exe);
      hCommand.push_back("-o");
      hCommand.push_back(hName);
      hCommand.push_back(uiName);
      cmCustomCommandLines hCommandLines;
      hCommandLines.push_back(hCommand);

      cmCustomCommandLine cxxCommand;
      cxxCommand.push_back(uic_exe);
      cxxCommand.push_back("-impl");
      cxxCommand.push_back(hName);
      cxxCommand.push_back("-o");
      cxxCommand.push_back(cxxName);
      cxxCommand.push_back(uiName);
      cmCustomCommandLines cxxCommandLines;
      cxxCommandLines.push_back(cxxCommand);

      cmCustomCommandLine mocCommand;
      mocCommand.push_back(moc_exe);
      mocCommand.push_back("-o");
      mocCommand.push_back(mocName);
      mocCommand.push_back(hName);
      cmCustomCommandLines mocCommandLines;
      mocCommandLines.push_back(mocCommand);

      std::vector<std::string> depends;
      depends.push_back(uiName);
      const char* no_main_dependency = 0;
      const char* no_comment = 0;
      const char* no_working_dir = 0;
      std::map<std::string,std::string> no_env_variables;
      this->Makefile->AddCustomCommandToOutput(hName.c_str(),
                                               depends,
                                               no_main_dependency,
                                               no_env_variables,
                                               hCommandLines,
                                               no_comment,
                                               no_working_dir);

      depends.push_back(hName);
      this->Makefile->AddCustomCommandToOutput(cxxName.c_str(),
                                               depends,
                                               no_main_dependency,
                                               no_env_variables,
                                               cxxCommandLines,
                                               no_comment,
                                               no_working_dir);

      depends.clear();
      depends.push_back(hName);
      this->Makefile->AddCustomCommandToOutput(mocName.c_str(),
                                               depends,
                                               no_main_dependency,
                                               no_env_variables,
                                               mocCommandLines,
                                               no_comment,
                                               no_working_dir);
      }
    }

  // Store the final list of source files and headers.
  this->Makefile->AddDefinition(sourceList.c_str(),
                                sourceListValue.c_str());
  this->Makefile->AddDefinition(headerList.c_str(),
                                headerListValue.c_str());
  return true;
}
