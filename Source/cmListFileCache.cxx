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

cmListFileCache* cmListFileCache::Instance = 0;


cmListFileCache* cmListFileCache::GetInstance()
{
  if(!cmListFileCache::Instance)
    {
    cmListFileCache::Instance = new cmListFileCache;
    }
  return cmListFileCache::Instance;
}


void cmListFileCache::ClearCache()
{
  delete cmListFileCache::Instance;
  cmListFileCache::Instance = 0;
}



cmListFile* cmListFileCache::GetFileCache(const char* path,
                                          bool requireProjectCommand)
{
  ListFileMap::iterator sl = m_ListFileCache.find(path);
  if (sl == m_ListFileCache.end())
    {
    // if not already in the map, then parse and store the 
    // file
    if(!this->CacheFile(path, requireProjectCommand))
      {
      return 0;
      }
    sl = m_ListFileCache.find(path);
    if (sl == m_ListFileCache.end())
      {
      cmSystemTools::Error("Fatal error, in cmListFileCache CacheFile failed",
                           path);
      return 0;
      }
    }
  cmListFile& ret = sl->second;
  if(cmSystemTools::ModifiedTime(path) > ret.m_ModifiedTime )
    {
    if(!this->CacheFile(path, requireProjectCommand))
      {
      return 0;
      }
    else
      {
      sl = m_ListFileCache.find(path);
      return &sl->second;
      }
    } 
  return &ret;
}


bool cmListFileCache::CacheFile(const char* path, bool requireProjectCommand)
{
  if(!cmSystemTools::FileExists(path))
    {
    return false;
    }
  // Get a pointer to a persistent copy of the name.
  const char* filename = this->GetUniqueStringPointer(path);

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
  cmListFile inFile;
  inFile.m_ModifiedTime = cmSystemTools::ModifiedTime(filename);
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
          inFile.m_Functions.push_back(inFunction);
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
    inFile.m_ModifiedTime = 0;
    }

  cmListFileLexer_Delete(lexer);

  if(requireProjectCommand)
    {
    bool hasProject = false;
    // search for a project command
    for(std::vector<cmListFileFunction>::iterator i 
          = inFile.m_Functions.begin();
        i != inFile.m_Functions.end(); ++i)
      {
      if(i->m_Name == "PROJECT")
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
      inFile.m_Functions.insert(inFile.m_Functions.begin(),project);
      }
    }
  m_ListFileCache[filename] = inFile;
  return true;
}

void cmListFileCache::FlushCache(const char* path)
{
  ListFileMap::iterator it = m_ListFileCache.find(path);
  if ( it != m_ListFileCache.end() )
    {
    m_ListFileCache.erase(it);
    return;
    }
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
  while((token = cmListFileLexer_Scan(lexer)))
    {
    if(token->type == cmListFileLexer_Token_ParenRight)
      {
      return true;
      }
    else if(token->type == cmListFileLexer_Token_Identifier ||
            token->type == cmListFileLexer_Token_ArgumentUnquoted)
      {
      cmListFileArgument a(cmSystemTools::RemoveEscapes(token->text),
                           false, filename, token->line);
      function.m_Arguments.push_back(a);
      }
    else if(token->type == cmListFileLexer_Token_ArgumentQuoted)
      {
      cmListFileArgument a(cmSystemTools::RemoveEscapes(token->text),
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
    }

  cmOStringStream error;
  error << "Error in cmake code at\n"
        << filename << ":" << cmListFileLexer_GetCurrentLine(lexer) << ":\n"
        << "Parse error.  Function missing ending \")\".  "
        << "End of file reached.";
  cmSystemTools::Error(error.str().c_str());

  return false;
}

//----------------------------------------------------------------------------
const char* cmListFileCache::GetUniqueStringPointer(const char* name)
{
  UniqueStrings::iterator i = m_UniqueStrings.find(name);
  if(i == m_UniqueStrings.end())
    {
    char* str = new char[strlen(name)+1];
    strcpy(str, name);
    i = m_UniqueStrings.insert(UniqueStrings::value_type(name, str)).first;
    }
  return i->second;
}

//----------------------------------------------------------------------------
cmListFileCache::~cmListFileCache()
{
  for(UniqueStrings::iterator i = m_UniqueStrings.begin();
      i != m_UniqueStrings.end(); ++i)
    {
    delete [] i->second;
    }
}
