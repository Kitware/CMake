/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmAddSubDirectoryCommand.h"

// cmAddSubDirectoryCommand
bool cmAddSubDirectoryCommand::InitialPass
(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  // store the binpath
  std::string srcArg = args[0];
  std::string binArg;
  
  bool excludeFromAll = false;

  // process the rest of the arguments looking for optional args
  std::vector<std::string>::const_iterator i = args.begin();
  ++i;
  for(;i != args.end(); ++i)
    {
    if(*i == "EXCLUDE_FROM_ALL")
      {
      excludeFromAll = true;
      continue;
      }
    else if (!binArg.size())
      {
      binArg = *i;
      }
    else
      {
      this->SetError("called with incorrect number of arguments");
      return false;
      }
    }

  // Compute the full path to the specified source directory.
  // Interpret a relative path with respect to the current source directory.
  std::string srcPath;
  if(cmSystemTools::FileIsFullPath(srcArg.c_str()))
    {
    srcPath = srcArg;
    }
  else
    {
    srcPath = this->Makefile->GetCurrentDirectory();
    srcPath += "/";
    srcPath += srcArg;
    }
  if(!cmSystemTools::FileIsDirectory(srcPath.c_str()))
    {
    std::string error = "given source \"";
    error += srcArg;
    error += "\" which is not an existing directory.";
    this->SetError(error.c_str());
    return false;
    }
  srcPath = cmSystemTools::CollapseFullPath(srcPath.c_str());

  // Compute the full path to the binary directory.
  std::string binPath;
  if(binArg.empty())
    {
    // No binary directory was specified.  If the source directory is
    // not a subdirectory of the current directory then it is an
    // error.
    if(!cmSystemTools::FindLastString(srcPath.c_str(),
                                      this->Makefile->GetCurrentDirectory()))
      {
      cmOStringStream e;
      e << "not given a binary directory but the given source directory "
        << "\"" << srcPath << "\" is not a subdirectory of \""
        << this->Makefile->GetCurrentDirectory() << "\".  "
        << "When specifying an out-of-tree source a binary directory "
        << "must be explicitly specified.";
      this->SetError(e.str().c_str());
      return false;
      }

    // Remove the CurrentDirectory from the srcPath and replace it
    // with the CurrentOutputDirectory.
    binPath = srcPath;
    cmSystemTools::ReplaceString(binPath,
                                 this->Makefile->GetCurrentDirectory(),
                                 this->Makefile->GetCurrentOutputDirectory());
    }
  else
    {
    // Use the binary directory specified.
    // Interpret a relative path with respect to the current binary directory.
    if(cmSystemTools::FileIsFullPath(binArg.c_str()))
      {
      binPath = binArg;
      }
    else
      {
      binPath = this->Makefile->GetCurrentOutputDirectory();
      binPath += "/";
      binPath += binArg;
      }
    }
  binPath = cmSystemTools::CollapseFullPath(binPath.c_str());

  // Add the subdirectory using the computed full paths.
  this->Makefile->AddSubDirectory(srcPath.c_str(), binPath.c_str(),
                                  excludeFromAll, false, true);

  return true;
}
