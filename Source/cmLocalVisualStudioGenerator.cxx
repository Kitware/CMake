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
#include "cmLocalVisualStudioGenerator.h"
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmSystemTools.h"

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
void cmLocalVisualStudioGenerator::ComputeObjectNameRequirements
(std::vector<cmSourceGroup> const& sourceGroups)
{
  // Clear the current set of requirements.
  this->NeedObjectName.clear();

  // Count the number of object files with each name.  Note that
  // windows file names are not case sensitive.
  std::map<cmStdString, int> objectNameCounts;
  for(unsigned int i = 0; i < sourceGroups.size(); ++i)
    {
    cmSourceGroup sg = sourceGroups[i];
    std::vector<const cmSourceFile*> const& srcs = sg.GetSourceFiles();
    for(std::vector<const cmSourceFile*>::const_iterator s = srcs.begin();
        s != srcs.end(); ++s)
      {
      const cmSourceFile* sf = *s;
      if(this->SourceFileCompiles(sf))
        {
        std::string objectName =
          cmSystemTools::LowerCase(
          cmSystemTools::GetFilenameWithoutLastExtension(
              sf->GetFullPath().c_str()));
        objectName += ".obj";
        objectNameCounts[objectName] += 1;
        }
      }
    }

  // For all source files producing duplicate names we need unique
  // object name computation.
  for(unsigned int i = 0; i < sourceGroups.size(); ++i)
    {
    cmSourceGroup sg = sourceGroups[i];
    std::vector<const cmSourceFile*> const& srcs = sg.GetSourceFiles();
    for(std::vector<const cmSourceFile*>::const_iterator s = srcs.begin();
        s != srcs.end(); ++s)
      {
      const cmSourceFile* sf = *s;
      if(this->SourceFileCompiles(sf))
        {
        std::string objectName =
          cmSystemTools::LowerCase(
          cmSystemTools::GetFilenameWithoutLastExtension(
              sf->GetFullPath().c_str()));
        objectName += ".obj";
        if(objectNameCounts[objectName] > 1)
          {
          this->NeedObjectName.insert(sf);
          }
        }
      }
    }
}

//----------------------------------------------------------------------------
std::string
cmLocalVisualStudioGenerator
::ConstructScript(const cmCustomCommandLines& commandLines,
                  const char* workingDirectory,
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
    script += newline;
    newline = newline_text;
    script += "cd ";
    script += this->Convert(workingDirectory, START_OUTPUT, SHELL);

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
    if(!workingDirectory)
      {
    script += this->Convert(commandLine[0].c_str(),START_OUTPUT,SHELL);
      }
    else
      {
      script += this->Convert(commandLine[0].c_str(),NONE,SHELL);
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
    }
  return script;
}
