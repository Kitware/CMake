/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmLocalVisualStudioGenerator.h"
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmSystemTools.h"
#include "cmCustomCommandGenerator.h"
#include "windows.h"

//----------------------------------------------------------------------------
cmLocalVisualStudioGenerator::cmLocalVisualStudioGenerator(VSVersion v)
{
  this->WindowsShell = true;
  this->WindowsVSIDE = true;
  this->Version = v;
}

//----------------------------------------------------------------------------
cmLocalVisualStudioGenerator::~cmLocalVisualStudioGenerator()
{
}

//----------------------------------------------------------------------------
cmsys::auto_ptr<cmCustomCommand>
cmLocalVisualStudioGenerator::MaybeCreateImplibDir(cmTarget& target,
                                                   const char* config,
                                                   bool isFortran)
{
  cmsys::auto_ptr<cmCustomCommand> pcc;

  // If an executable exports symbols then VS wants to create an
  // import library but forgets to create the output directory.
  // The Intel Fortran plugin always forgets to the directory.
  if(target.GetType() != cmTarget::EXECUTABLE &&
     !(isFortran && target.GetType() == cmTarget::SHARED_LIBRARY))
    { return pcc; }
  std::string outDir = target.GetDirectory(config, false);
  std::string impDir = target.GetDirectory(config, true);
  if(impDir == outDir) { return pcc; }

  // Add a pre-build event to create the directory.
  cmCustomCommandLine command;
  command.push_back(this->Makefile->GetRequiredDefinition("CMAKE_COMMAND"));
  command.push_back("-E");
  command.push_back("make_directory");
  command.push_back(impDir);
  std::vector<std::string> no_output;
  std::vector<std::string> no_depends;
  cmCustomCommand::EnvVariablesMap no_env_variables;
  cmCustomCommandLines commands;
  commands.push_back(command);
  pcc.reset(new cmCustomCommand(0, no_output, no_depends, no_env_variables, commands, 0, 0));
  pcc->SetEscapeOldStyle(false);
  pcc->SetEscapeAllowMakeVars(true);
  return pcc;
}

//----------------------------------------------------------------------------
const char* cmLocalVisualStudioGenerator::ReportErrorLabel() const
{
  return ":VCReportError";
}

//----------------------------------------------------------------------------
const char* cmLocalVisualStudioGenerator::GetReportErrorLabel() const
{
  return this->ReportErrorLabel();
}

//----------------------------------------------------------------------------
std::string
cmLocalVisualStudioGenerator
::ConstructScript(cmCustomCommand const& cc,
                  const char* configName,
                  const char* newline_text)
{
  bool useLocal = this->CustomCommandUseLocal();
  const char* workingDirectory = cc.GetWorkingDirectory();
  const cmCustomCommand::EnvVariablesMap &env_variables = cc.GetEnvVariables();

  cmCustomCommandGenerator ccg(cc, configName, this->Makefile);
  RelativeRoot relativeRoot = workingDirectory? NONE : START_OUTPUT;

  // Avoid leading or trailing newlines.
  const char* newline = "";

  // Line to check for error between commands.
  std::string check_error = newline_text;
  if(useLocal)
    {
    check_error += "if %errorlevel% neq 0 goto :cmEnd";
    }
  else
    {
    check_error += "if errorlevel 1 goto ";
    check_error += this->GetReportErrorLabel();
    }

  // Store the script in a string.
  std::string script;

  // Open a local context.
  if(useLocal)
    {
    script += newline;
    newline = newline_text;
    script += "setlocal";
    }

  // Construct environment variables args
  cmCustomCommandLines env_cmd_lines;
  cmCustomCommandLine env_cmd_line;
  env_cmd_line.push_back(this->Makefile->GetRequiredDefinition("CMAKE_COMMAND"));
  env_cmd_line.push_back("-E");
  env_cmd_line.push_back("env");
  
  // Get the environment varibles args
  if(!env_variables.empty())
    {
    typedef cmCustomCommand::EnvVariablesMap::const_iterator env_iter_type;
    env_iter_type env_var_it_end(env_variables.end());

    for(env_iter_type env_var_it(env_variables.begin());
        env_var_it != env_var_it_end;
        ++env_var_it)
      {
      env_cmd_line.push_back(env_var_it->first+"="+(env_var_it->second.c_str()));
      }
    }

  // Create a custom command generator for the environment modification
  std::vector<std::string> no_output, no_depends;
  cmCustomCommand::EnvVariablesMap no_env_variables;
  env_cmd_lines.push_back(env_cmd_line);
  cmCustomCommand env_cmd(0, no_output, no_depends, no_env_variables, env_cmd_lines, 0, 0);
  env_cmd.SetEscapeOldStyle(false);
  env_cmd.SetEscapeAllowMakeVars(true);
  cmCustomCommandGenerator env_cmd_generator(env_cmd, configName, this->Makefile);

  if(workingDirectory)
    {
    // Change the working directory.
    script += newline;
    newline = newline_text;
    script += "cd ";
    script += this->Convert(workingDirectory, FULL, SHELL);
    script += check_error;

    // Change the working drive.
    if(workingDirectory[0] && workingDirectory[1] == ':')
      {
      script += newline;
      newline = newline_text;
      script += workingDirectory[0];
      script += workingDirectory[1];
      script += check_error;
      }
    }

  // for visual studio IDE add extra stuff to the PATH
  // if CMAKE_MSVCIDE_RUN_PATH is set.
  if(this->Makefile->GetDefinition("MSVC_IDE"))
    {
    const char* extraPath =
      this->Makefile->GetDefinition("CMAKE_MSVCIDE_RUN_PATH");
    if(extraPath)
      {
      script += newline;
      newline = newline_text;
      script += "set PATH=";
      script += extraPath;
      script += ";%PATH%";
      }
    }

  // Write each command on a single line.
  for(unsigned int c = 0; c < ccg.GetNumberOfCommands(); ++c)
    {
    // Start a new line.
    script += newline;
    newline = newline_text;

    // Add this command line.
    std::string cmd = ccg.GetCommand(c);

    // Use "call " before any invocations of .bat or .cmd files
    // invoked as custom commands.
    //
    std::string suffix;
    if (cmd.size() > 4)
      {
      suffix = cmSystemTools::LowerCase(cmd.substr(cmd.size()-4));
      if (suffix == ".bat" || suffix == ".cmd")
        {
        script += "call ";
        }
      }

    // Modify the environment, if necessary
    if(!env_variables.empty()) {
      std::string env_cmd_str = this->Convert(env_cmd_generator.GetCommand(0).c_str(), relativeRoot, SHELL);
      script += env_cmd_str;
      env_cmd_generator.AppendArguments(0, script);
      script+=" ";
    }

    script += this->Convert(cmd.c_str(), relativeRoot, SHELL);
    ccg.AppendArguments(c, script);

    // After each custom command, check for an error result.
    // If there was an error, jump to the VCReportError label,
    // skipping the run of any subsequent commands in this
    // sequence.
    script += check_error;
    }

  // Close the local context.
  if(useLocal)
    {
    script += newline;
    script += ":cmEnd";
    script += newline;
    script += "endlocal & call :cmErrorLevel %errorlevel% & goto :cmDone";
    script += newline;
    script += ":cmErrorLevel";
    script += newline;
    script += "exit /b %1";
    script += newline;
    script += ":cmDone";
    script += newline;
    script += "if %errorlevel% neq 0 goto ";
    script += this->GetReportErrorLabel();
    }

  return script;
}
