/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmListFileCache.h"

#include "cmListFileLexer.h"
#include "cmSystemTools.h"
#include "cmMakefile.h"
#include "cmVersion.h"

#include <cmsys/RegularExpression.hxx>

#ifdef __BORLANDC__
# pragma warn -8060 /* possibly incorrect assignment */
#endif

bool cmListFileCacheParseFunction(cmListFileLexer* lexer,
                                  cmListFileFunction& function,
                                  const char* filename);

bool cmListFile::ParseFile(const char* filename, 
                           bool topLevel,
                           cmMakefile *mf)
{
  if(!cmSystemTools::FileExists(filename))
    {
    return false;
    }

  // Create the scanner.
  cmListFileLexer* lexer = cmListFileLexer_New();
  if(!lexer)
    {
    cmSystemTools::Error("cmListFileCache: error allocating lexer ");
    return false;
    }

  // Open the file.
  if(!cmListFileLexer_SetFileName(lexer, filename))
    {
    cmListFileLexer_Delete(lexer);
    cmSystemTools::Error("cmListFileCache: error can not open file ", 
                         filename);
    return false;
    }

  // Use a simple recursive-descent parser to process the token
  // stream.
  this->ModifiedTime = cmSystemTools::ModifiedTime(filename);
  bool parseError = false;
  bool haveNewline = true;
  cmListFileLexer_Token* token;
  while(!parseError && (token = cmListFileLexer_Scan(lexer)))
    {
    if(token->type == cmListFileLexer_Token_Newline)
      {
      haveNewline = true;
      }
    else if(token->type == cmListFileLexer_Token_Identifier)
      {
      if(haveNewline)
        {
        haveNewline = false;
        cmListFileFunction inFunction;
        inFunction.Name = token->text;
        inFunction.FilePath = filename;
        inFunction.Line = token->line;
        if(cmListFileCacheParseFunction(lexer, inFunction, filename))
          {
          this->Functions.push_back(inFunction);
          }
        else
          {
          parseError = true;
          }
        }
      else
        {
        cmOStringStream error;
        error << "Error in cmake code at\n"
              << filename << ":" << token->line << ":\n"
              << "Parse error.  Expected a newline, got "
              << cmListFileLexer_GetTypeAsString(lexer, token->type)
              << " with text \"" << token->text << "\".";
        cmSystemTools::Error(error.str().c_str());
        parseError = true;
        }
      }
    else
      {
      cmOStringStream error;
      error << "Error in cmake code at\n"
            << filename << ":" << token->line << ":\n"
            << "Parse error.  Expected a command name, got "
            << cmListFileLexer_GetTypeAsString(lexer, token->type)
            << " with text \""
            << token->text << "\".";
      cmSystemTools::Error(error.str().c_str());
      parseError = true;
      }
    }
  if (parseError)
    {
    this->ModifiedTime = 0;
    }

  cmListFileLexer_Delete(lexer);

  // do we need a cmake_policy(VERSION call?
  if(topLevel)
  {
    bool hasVersion = false;
    // search for the right policy command
    for(std::vector<cmListFileFunction>::iterator i 
          = this->Functions.begin();
        i != this->Functions.end(); ++i)
    {
      if (cmSystemTools::LowerCase(i->Name) == "cmake_minimum_required")
      {
        hasVersion = true;
        break;
      }
    }
    // if no policy command is found this is an error if they use any 
    // non advanced functions or a lot of functions
    if(!hasVersion)
    {
      bool isProblem = true;
      if (this->Functions.size() < 30)
      {
        // the list of simple commands DO NOT ADD TO THIS LIST!!!!!
        // these commands must have backwards compatibility forever and
        // and that is a lot longer than your tiny mind can comprehend mortal
        std::set<std::string> allowedCommands;
        allowedCommands.insert("project");
        allowedCommands.insert("set");
        allowedCommands.insert("if");
        allowedCommands.insert("endif");
        allowedCommands.insert("else");
        allowedCommands.insert("elseif");
        allowedCommands.insert("add_executable");
        allowedCommands.insert("add_library");
        allowedCommands.insert("target_link_libraries");
        allowedCommands.insert("option");
        allowedCommands.insert("message");
        isProblem = false;
        for(std::vector<cmListFileFunction>::iterator i 
              = this->Functions.begin();
            i != this->Functions.end(); ++i)
        {
          std::string name = cmSystemTools::LowerCase(i->Name);
          if (allowedCommands.find(name) == allowedCommands.end())
          {
            isProblem = true;
            break;
          }       
        }
      }
      
      if (isProblem)
      {
      // Tell the top level cmMakefile to diagnose
      // this violation of CMP0000.
      mf->SetCheckCMP0000(true);

      // Implicitly set the version for the user.
      mf->SetPolicyVersion("2.4");
      }
    }
  }

  if(topLevel)
    {
    bool hasProject = false;
    // search for a project command
    for(std::vector<cmListFileFunction>::iterator i 
          = this->Functions.begin();
        i != this->Functions.end(); ++i)
      {
      if(cmSystemTools::LowerCase(i->Name) == "project")
        {
        hasProject = true;
        break;
        }
      }
    // if no project command is found, add one
    if(!hasProject)
      {
      cmListFileFunction project;
      project.Name = "PROJECT";
      cmListFileArgument prj("Project", false, filename, 0);
      project.Arguments.push_back(prj);
      this->Functions.insert(this->Functions.begin(),project);
      }
    }
  if(parseError)
    {
    return false;
    }
  return true;
}

bool cmListFileCacheParseFunction(cmListFileLexer* lexer,
                                  cmListFileFunction& function,
                                  const char* filename)
{
  // Command name has already been parsed.  Read the left paren.
  cmListFileLexer_Token* token;
  if(!(token = cmListFileLexer_Scan(lexer)))
    {
    cmOStringStream error;
    error << "Error in cmake code at\n"
          << filename << ":" << cmListFileLexer_GetCurrentLine(lexer) << ":\n"
          << "Parse error.  Function missing opening \"(\".";
    cmSystemTools::Error(error.str().c_str());
    return false;
    }
  if(token->type != cmListFileLexer_Token_ParenLeft)
    {
    cmOStringStream error;
    error << "Error in cmake code at\n"
          << filename << ":" << cmListFileLexer_GetCurrentLine(lexer) << ":\n"
          << "Parse error.  Expected \"(\", got "
          << cmListFileLexer_GetTypeAsString(lexer, token->type)
          << " with text \"" << token->text << "\".";
    cmSystemTools::Error(error.str().c_str());
    return false;
    }

  // Arguments.
  unsigned long lastLine = cmListFileLexer_GetCurrentLine(lexer);
  unsigned long parenDepth = 0;
  while((token = cmListFileLexer_Scan(lexer)))
    {
    if(token->type == cmListFileLexer_Token_ParenLeft)
      {
      parenDepth++;
      cmListFileArgument a("(",
                           false, filename, token->line);
      function.Arguments.push_back(a);
      }
    else if(token->type == cmListFileLexer_Token_ParenRight)
      {
      if (parenDepth == 0)
        {
        return true;
        }
      parenDepth--;
      cmListFileArgument a(")",
                           false, filename, token->line);
      function.Arguments.push_back(a);        
      }
    else if(token->type == cmListFileLexer_Token_Identifier ||
            token->type == cmListFileLexer_Token_ArgumentUnquoted)
      {
      cmListFileArgument a(token->text,
                           false, filename, token->line);
      function.Arguments.push_back(a);
      }
    else if(token->type == cmListFileLexer_Token_ArgumentQuoted)
      {
      cmListFileArgument a(token->text,
                           true, filename, token->line);
      function.Arguments.push_back(a);
      }
    else if(token->type != cmListFileLexer_Token_Newline)
      {
      // Error.
      cmOStringStream error;
      error << "Error in cmake code at\n"
            << filename << ":" << cmListFileLexer_GetCurrentLine(lexer) 
            << ":\n"
            << "Parse error.  Function missing ending \")\".  "
            << "Instead found "
            << cmListFileLexer_GetTypeAsString(lexer, token->type)
            << " with text \"" << token->text << "\".";
      cmSystemTools::Error(error.str().c_str());
      return false;
      }
    lastLine = cmListFileLexer_GetCurrentLine(lexer);
    }

  cmOStringStream error;
  error << "Error in cmake code at\n"
        << filename << ":" << lastLine << ":\n"
        << "Parse error.  Function missing ending \")\".  "
        << "End of file reached.";
  cmSystemTools::Error(error.str().c_str());

  return false;
}

//----------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, cmListFileContext const& lfc)
{
  os << lfc.FilePath;
  if(lfc.Line)
    {
    os << ":" << lfc.Line;
    if(!lfc.Name.empty())
      {
      os << " (" << lfc.Name << ")";
      }
    }
  return os;
}
