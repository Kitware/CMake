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
#include "windows.h"

//----------------------------------------------------------------------------
cmLocalVisualStudioGenerator::cmLocalVisualStudioGenerator()
{
  this->WindowsShell = true;
  this->WindowsVSIDE = true;
}

//----------------------------------------------------------------------------
cmLocalVisualStudioGenerator::~cmLocalVisualStudioGenerator()
{
}

//----------------------------------------------------------------------------
cmsys::auto_ptr<cmCustomCommand>
cmLocalVisualStudioGenerator::MaybeCreateImplibDir(cmTarget& target,
                                                   const char* config)
{
  cmsys::auto_ptr<cmCustomCommand> pcc;

  // If an executable exports symbols then VS wants to create an
  // import library but forgets to create the output directory.
  if(target.GetType() != cmTarget::EXECUTABLE) { return pcc; }
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
  cmCustomCommandLines commands;
  commands.push_back(command);
  pcc.reset(new cmCustomCommand(no_output, no_depends, commands, 0, 0));
  pcc->SetEscapeOldStyle(false);
  pcc->SetEscapeAllowMakeVars(true);
  return pcc;
}

//----------------------------------------------------------------------------
bool cmLocalVisualStudioGenerator::SourceFileCompiles(const cmSourceFile* sf)
{
  // Identify the language of the source file.
  if(const char* lang = this->GetSourceFileLanguage(*sf))
    {
    // Check whether this source will actually be compiled.
    return (!sf->GetCustomCommand() &&
            !sf->GetPropertyAsBool("HEADER_FILE_ONLY") &&
            !sf->GetPropertyAsBool("EXTERNAL_OBJECT"));
    }
  else
    {
    // Unknown source file language.  Assume it will not be compiled.
    return false;
    }
}

//----------------------------------------------------------------------------
void cmLocalVisualStudioGenerator::CountObjectNames(
    const std::vector<cmSourceGroup>& groups,
    std::map<cmStdString, int>& counts)
{
  for(unsigned int i = 0; i < groups.size(); ++i)
    {
    cmSourceGroup sg = groups[i];
    std::vector<const cmSourceFile*> const& srcs = sg.GetSourceFiles();
    for(std::vector<const cmSourceFile*>::const_iterator s = srcs.begin();
        s != srcs.end(); ++s)
      {
      const cmSourceFile* sf = *s;
      if(this->SourceFileCompiles(sf))
        {
        std::string objectName = cmSystemTools::LowerCase(
            cmSystemTools::GetFilenameWithoutLastExtension(
              sf->GetFullPath()));
        objectName += ".obj";
        counts[objectName] += 1;
        }
      }
    this->CountObjectNames(sg.GetGroupChildren(), counts);
    }
}

//----------------------------------------------------------------------------
void cmLocalVisualStudioGenerator::InsertNeedObjectNames(
   const std::vector<cmSourceGroup>& groups,
    std::map<cmStdString, int>& count)
{
  for(unsigned int i = 0; i < groups.size(); ++i)
    {
    cmSourceGroup sg = groups[i];
    std::vector<const cmSourceFile*> const& srcs = sg.GetSourceFiles();
    for(std::vector<const cmSourceFile*>::const_iterator s = srcs.begin();
        s != srcs.end(); ++s)
      {
      const cmSourceFile* sf = *s;
      if(this->SourceFileCompiles(sf))
        {
        std::string objectName = cmSystemTools::LowerCase(
           cmSystemTools::GetFilenameWithoutLastExtension(sf->GetFullPath()));
        objectName += ".obj";
        if(count[objectName] > 1)
          {
          this->NeedObjectName.insert(sf);
          }
        }
      }
    this->InsertNeedObjectNames(sg.GetGroupChildren(), count);
    }
}


//----------------------------------------------------------------------------
void cmLocalVisualStudioGenerator::ComputeObjectNameRequirements
(std::vector<cmSourceGroup> const& sourceGroups)
{
  // Clear the current set of requirements.
  this->NeedObjectName.clear();

  // Count the number of object files with each name.  Note that
  // windows file names are not case sensitive.
  std::map<cmStdString, int> objectNameCounts;
  this->CountObjectNames(sourceGroups, objectNameCounts);

  // For all source files producing duplicate names we need unique
  // object name computation.
  this->InsertNeedObjectNames(sourceGroups, objectNameCounts);
}

//----------------------------------------------------------------------------
std::string
cmLocalVisualStudioGenerator
::ConstructScript(const cmCustomCommandLines& commandLines,
                  const char* workingDirectory,
                  const char* configName,
                  bool escapeOldStyle,
                  bool escapeAllowMakeVars,
                  const char* newline_text)
{
  // Avoid leading or trailing newlines.
  const char* newline = "";

  // Store the script in a string.
  std::string script;
  if(workingDirectory)
    {
    // Change the working directory.
    script += newline;
    newline = newline_text;
    script += "cd ";
    script += this->Convert(workingDirectory, FULL, SHELL);

    // Change the working drive.
    if(workingDirectory[0] && workingDirectory[1] == ':')
      {
      script += newline;
      newline = newline_text;
      script += workingDirectory[0];
      script += workingDirectory[1];
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
  for(cmCustomCommandLines::const_iterator cl = commandLines.begin();
      cl != commandLines.end(); ++cl)
    {
    // Start a new line.
    script += newline;
    newline = newline_text;

    // Start with the command name.
    const cmCustomCommandLine& commandLine = *cl;
    std::string commandName = this->GetRealLocation(commandLine[0].c_str(), 
                                                    configName);
    if(!workingDirectory)
      {
      script += this->Convert(commandName.c_str(),START_OUTPUT,SHELL);
      }
    else
      {
      script += this->Convert(commandName.c_str(),NONE,SHELL);
      }

    // Add the arguments.
    for(unsigned int j=1;j < commandLine.size(); ++j)
      {
      script += " ";
      if(escapeOldStyle)
        {
        script += this->EscapeForShellOldStyle(commandLine[j].c_str());
        }
      else
        {
        script += this->EscapeForShell(commandLine[j].c_str(),
                                       escapeAllowMakeVars);
        }
      }

    // After each custom command, check for an error result.
    // If there was an error, jump to the VCReportError label,
    // skipping the run of any subsequent commands in this
    // sequence.
    //
    script += newline_text;
    script += "if errorlevel 1 goto VCReportError";
    }

  return script;
}
