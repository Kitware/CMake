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
#include "cmSystemTools.h"

#include <cmsys/RegularExpression.hxx>

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
    
  std::ifstream fin(path);
  if(!fin)
    {
    cmSystemTools::Error("cmListFileCache: error can not open file ", path);
    return false;
    }
  long line=0;
  cmListFile inFile;
  inFile.m_ModifiedTime = cmSystemTools::ModifiedTime(path);
  bool parseError;
  while ( fin )
    {
    cmListFileFunction inFunction;
    if(cmListFileCache::ParseFunction(fin, inFunction, path, parseError,
                                      line))
      {
      inFunction.m_FilePath = path;
      inFile.m_Functions.push_back(inFunction);
      }
    if (parseError)
      {
      inFile.m_ModifiedTime = 0;
      }
    }
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
      cmListFileArgument prj("Project", false);
      project.m_Arguments.push_back(prj);
      inFile.m_Functions.insert(inFile.m_Functions.begin(),project);
      }
    }
  m_ListFileCache[path] = inFile;
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

//----------------------------------------------------------------------------
inline bool cmListFileCachePreprocessLine(std::string& line)
{
  // Keep track of whether characters are inside a quoted argument.
  bool quoted = false;
  
  // Keep track of whether the line is blank.
  bool blank = true;
  
  // Loop over every character in the line.
  std::string::iterator c;
  for(c = line.begin(); c != line.end(); ++c)
    {
    if((*c == '\\') && (c < line.end()-1))
      {
      // A backslash escapes any character, so skip the next
      // character.
      ++c;
      
      // We have encountered a non-whitespace character.
      blank = false;
      }
    else if(*c == '"')
      {
      // A double-quote either starts or ends a quoted argument.
      quoted = !quoted;
      
      // We have encountered a non-whitespace character.
      blank = false;
      }
    else if(*c == '#' && !quoted)
      {
      // A pound character outside a double-quoted argument marks the
      // rest of the line as a comment.  Skip it.
      break;
      }
    else if((*c != ' ') && (*c != '\t') && (*c != '\r'))
      {
      // We have encountered a non-whitespace character.
      blank = false;
      }
    }
  
  // Erase from the comment character to the end of the line.  If no
  // comment was present, both iterators are end() iterators and this
  // does nothing.
  line.erase(c, line.end());
  
  // Return true if there is anything useful on this line.
  return !blank;
}

//----------------------------------------------------------------------------
bool cmListFileCache::ParseFunction(std::ifstream& fin,
                                    cmListFileFunction& function,
                                    const char* filename,
                                    bool& parseError,
                                    long& line)
{
  parseError = false;
  std::string& name = function.m_Name;
  std::vector<cmListFileArgument>& arguments = function.m_Arguments;
  name = "";
  arguments = std::vector<cmListFileArgument>();
  std::string inbuffer;
  if(!fin)
    {
    return false;
    }
  if(cmSystemTools::GetLineFromStream(fin, inbuffer) )
    {
    // Count this line in line numbering.
    ++line;
    
    // Preprocess the line to remove comments.  Only use it if there
    // is non-whitespace.
    if(!cmListFileCachePreprocessLine(inbuffer))
      {
      return false;
      }
    
    // Regular expressions to match portions of a command invocation.
    cmsys::RegularExpression oneLiner("^[ \t]*([A-Za-z_0-9]*)[ \t]*\\((.*)\\)[ \t\r]*$");
    cmsys::RegularExpression multiLine("^[ \t]*([A-Za-z_0-9]*)[ \t]*\\((.*)$");
    cmsys::RegularExpression lastLine("^(.*)\\)[ \t\r]*$");

    // look for a oneline fun(arg arg2) 
    if(oneLiner.find(inbuffer.c_str()))
      {
      // the arguments are the second match
      std::string args = oneLiner.match(2);
      name = oneLiner.match(1);
      // break up the arguments
      cmListFileCache::GetArguments(args, arguments);
        function.m_Line = line;
      return true;
      }
    // look for a start of a multiline with no trailing ")"  fun(arg arg2 
    else if(multiLine.find(inbuffer.c_str()))
      {
      name = multiLine.match(1);
      std::string args = multiLine.match(2);
      cmListFileCache::GetArguments(args, arguments);
      function.m_Line = line;
      // Read lines until the closing paren is hit
      bool done = false;
      while(!done)
        {
        // read lines until the end paren is found
        if(cmSystemTools::GetLineFromStream(fin, inbuffer) )
          {
          // Count this line in line numbering.
          ++line;
          
          // Preprocess the line to remove comments.  Only use it if there
          // is non-whitespace.
          if(!cmListFileCachePreprocessLine(inbuffer))
            {
            continue;
            }
    
          // Is this the last line?
          if(lastLine.find(inbuffer.c_str()))
            {
            done = true;
            std::string gargs = lastLine.match(1);
            cmListFileCache::GetArguments(gargs, arguments);
            }
          else
            {
            cmListFileCache::GetArguments(inbuffer, arguments);
            }
          }
        else
          {
          parseError = true;
          cmOStringStream error;
          error << "Error in cmake code at\n"
                << filename << ":" << line << ":\n"
                << "Parse error.  Function missing ending \")\".";
          cmSystemTools::Error(error.str().c_str());
          return false;
          }
        }
      return true;
      }
    else
      {
      parseError = true;
      cmOStringStream error;
      error << "Error in cmake code at\n"
            << filename << ":" << line << ":\n"
            << "Parse error.";
      cmSystemTools::Error(error.str().c_str());
      return false;
      }
    }
  return false;

}

void cmListFileCache::GetArguments(std::string& line,
                                 std::vector<cmListFileArgument>& arguments)
{
  // Match a normal argument (not quoted, no spaces).
  cmsys::RegularExpression normalArgument("[ \t]*(([^ \t\r\\]|[\\].)+)[ \t\r]*");
  // Match a quoted argument (surrounded by double quotes, spaces allowed).
  cmsys::RegularExpression quotedArgument("[ \t]*(\"([^\"\\]|[\\].)*\")[ \t\r]*");

  bool done = false;
  while(!done)
    {
    std::string arg;
    std::string::size_type endpos=0;
    bool quoted = false;
    bool foundQuoted = quotedArgument.find(line.c_str());
    bool foundNormal = normalArgument.find(line.c_str());

    if(foundQuoted && foundNormal)
      {
      // Both matches were found.  Take the earlier one.
      // Favor double-quoted version if there is a tie.
      if(normalArgument.start(1) < quotedArgument.start(1))
        {
        arg = normalArgument.match(1);
        endpos = normalArgument.end(1);
        }
      else
        {
        arg = quotedArgument.match(1);
        endpos = quotedArgument.end(1);
        // Strip off the double quotes on the ends.
        arg = arg.substr(1, arg.length()-2);
        quoted = true;
        }
      }    
    else if(foundQuoted)
      {
      arg = quotedArgument.match(1);
      endpos = quotedArgument.end(1);
      // Strip off the double quotes on the ends.
      arg = arg.substr(1, arg.length()-2);
      quoted = true;
      }
    else if(foundNormal)
      {
      arg = normalArgument.match(1);
      endpos = normalArgument.end(1);
      }
    else
      {
      done = true;
      }
    if(!done)
      {
      cmListFileArgument a(cmSystemTools::RemoveEscapes(arg.c_str()), quoted);
      arguments.push_back(a);
      line = line.substr(endpos, line.length() - endpos);
      }
    }
}
