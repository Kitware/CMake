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
#include "cmListFileCache.h"

#include "cmListFileLexer.h"
#include "cmSystemTools.h"

#include <cmsys/RegularExpression.hxx>

bool cmListFileCacheParseFunction(cmListFileLexer* lexer,
                                  cmListFileFunction& function,
                                  const char* filename);

bool cmListFile::ParseFile(const char* filename, bool requireProjectCommand)
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
    cmSystemTools::Error("cmListFileCache: error can not open file ", filename);
    return false;
    }

  // Use a simple recursive-descent parser to process the token
  // stream.
  this->m_ModifiedTime = cmSystemTools::ModifiedTime(filename);
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
        inFunction.m_Name = token->text;
        inFunction.m_FilePath = filename;
        inFunction.m_Line = token->line;
        if(cmListFileCacheParseFunction(lexer, inFunction, filename))
          {
          this->m_Functions.push_back(inFunction);
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
    this->m_ModifiedTime = 0;
    }

  cmListFileLexer_Delete(lexer);

  if(requireProjectCommand)
    {
    bool hasProject = false;
    // search for a project command
    for(std::vector<cmListFileFunction>::iterator i 
          = this->m_Functions.begin();
        i != this->m_Functions.end(); ++i)
      {
      if(cmSystemTools::LowerCase(i->m_Name) == "project")
        {
        hasProject = true;
        break;
        }
      }
    // if no project command is found, add one
    if(!hasProject)
      {
      cmListFileFunction project;
      project.m_Name = "PROJECT";
      cmListFileArgument prj("Project", false, filename, 0);
      project.m_Arguments.push_back(prj);
      this->m_Functions.insert(this->m_Functions.begin(),project);
      }
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
  while((token = cmListFileLexer_Scan(lexer)))
    {
    if(token->type == cmListFileLexer_Token_ParenRight)
      {
      return true;
      }
    else if(token->type == cmListFileLexer_Token_Identifier ||
            token->type == cmListFileLexer_Token_ArgumentUnquoted)
      {
      cmListFileArgument a(token->text,
                           false, filename, token->line);
      function.m_Arguments.push_back(a);
      }
    else if(token->type == cmListFileLexer_Token_ArgumentQuoted)
      {
      cmListFileArgument a(token->text,
                           true, filename, token->line);
      function.m_Arguments.push_back(a);
      }
    else if(token->type != cmListFileLexer_Token_Newline)
      {
      // Error.
      cmOStringStream error;
      error << "Error in cmake code at\n"
            << filename << ":" << cmListFileLexer_GetCurrentLine(lexer) << ":\n"
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
