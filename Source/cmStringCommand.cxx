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
#include "cmStringCommand.h"

//----------------------------------------------------------------------------
bool cmStringCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 1)
    {
    this->SetError("must be called with at least one argument.");
    return false;
    }
  
  std::string subCommand = args[0];
  if(subCommand == "REGEX")
    {
    return this->HandleRegexCommand(args);
    }  
  
  std::string e = "does not recognize sub-command "+subCommand;
  this->SetError(e.c_str());
  return false;
}

//----------------------------------------------------------------------------
bool cmStringCommand::HandleRegexCommand(std::vector<std::string> const& args)
{
  if(args.size() < 2)
    {
    this->SetError("sub-command REGEX requires a mode to be specified.");
    return false;
    }
  std::string mode = args[1];
  if(mode == "MATCH")
    {
    if(args.size() < 5)
      {
      this->SetError("sub-command REGEX, mode MATCH needs "
                     "at least 5 arguments total to command.");
      return false;
      }
    return this->RegexMatch(args);
    }
  else if(mode == "MATCHALL")
    {
    if(args.size() < 5)
      {
      this->SetError("sub-command REGEX, mode MATCHALL needs "
                     "at least 5 arguments total to command.");
      return false;
      }
    return this->RegexMatchAll(args);
    }
  else if(mode == "REPLACE")
    {
    if(args.size() < 6)
      {
      this->SetError("sub-command REGEX, mode MATCH needs "
                     "at least 6 arguments total to command.");
      return false;
      }
    return this->RegexReplace(args);
    }
  
  std::string e = "sub-command REGEX does not recognize mode "+mode;
  this->SetError(e.c_str());
  return false;
}

//----------------------------------------------------------------------------
bool cmStringCommand::RegexMatch(std::vector<std::string> const& args)
{
  //"STRING(REGEX MATCH <regular_expression> <output variable> <input> [<input>...])\n";
  std::string regex = args[2];
  std::string outvar = args[3];
  
  // Concatenate all the last arguments together.
  std::string input = args[4];
  for(int i=5; i < args.size(); ++i)
    {
    input += args[i];
    }
  
  // Compile the regular expression.
  cmRegularExpression re;
  if(!re.compile(regex.c_str()))
    {
    std::string e = "sub-command REGEX, mode MATCH failed to compile regex \""+regex+"\".";
    this->SetError(e.c_str());
    return false;
    }
  
  // Scan through the input for all matches.
  std::string output;
  if(re.find(input.c_str()))
    {
    std::string::size_type l = re.start();
    std::string::size_type r = re.end();
    if(r-l == 0)
      {
      std::string e = "sub-command REGEX, mode MATCH regex \""+regex+"\" matched an empty string.";
      this->SetError(e.c_str());
      return false;
      }
    output = input.substr(l, r-l);
    }
  
  // Store the output in the provided variable.
  m_Makefile->AddDefinition(outvar.c_str(), output.c_str());
  return true;
}

//----------------------------------------------------------------------------
bool cmStringCommand::RegexMatchAll(std::vector<std::string> const& args)
{
  //"STRING(REGEX MATCHALL <regular_expression> <output variable> <input> [<input>...])\n";
  std::string regex = args[2];
  std::string outvar = args[3];
  
  // Concatenate all the last arguments together.
  std::string input = args[4];
  for(int i=5; i < args.size(); ++i)
    {
    input += args[i];
    }
  
  // Compile the regular expression.
  cmRegularExpression re;
  if(!re.compile(regex.c_str()))
    {
    std::string e = "sub-command REGEX, mode MATCHALL failed to compile regex \""+regex+"\".";
    this->SetError(e.c_str());
    return false;
    }
  
  // Scan through the input for all matches.
  std::string output;
  const char* p = input.c_str();
  while(re.find(p))
    {
    std::string::size_type l = re.start();
    std::string::size_type r = re.end();
    if(r-l == 0)
      {
      std::string e = "sub-command REGEX, mode MATCHALL regex \""+regex+"\" matched an empty string.";
      this->SetError(e.c_str());
      return false;
      }
    if(output.length() > 0)
      {
      output += ";";
      }
    output += std::string(p+l, r-l);
    p += r;
    }
  
  // Store the output in the provided variable.
  m_Makefile->AddDefinition(outvar.c_str(), output.c_str());
  return true;
}

//----------------------------------------------------------------------------
bool cmStringCommand::RegexReplace(std::vector<std::string> const& args)
{
  //"STRING(REGEX REPLACE <regular_expression> <replace_expression> <output variable> <input> [<input>...])\n"
  std::string regex = args[2];
  std::string replace = args[3];  
  std::string outvar = args[4];
  
  // Pull apart the replace expression to find the escaped [0-9] values.
  std::vector<RegexReplacement> replacement;
  std::string::size_type l = 0;
  while(l < replace.length())
    {
    std::string::size_type r = replace.find("\\", l);
    if(r == std::string::npos)
      {
      r = replace.length();
      replacement.push_back(replace.substr(l, r-l));
      }
    else
      {
      if(r-l > 0)
        {
        replacement.push_back(replace.substr(l, r-l));
        }
      if(r == (replace.length()-1))
        {
        this->SetError("sub-command REGEX, mode REPLACE: "
                       "replace-expression ends in a backslash.");
        return false;
        }
      if((replace[r+1] >= '0') && (replace[r+1] <= '9'))
        {
        replacement.push_back(replace[r+1]-'0');
        }
      else if(replace[r+1] == 'n')
        {
        replacement.push_back("\n");
        }
      else if(replace[r+1] == '\\')
        {
        replacement.push_back("\\");
        }
      else
        {
        std::string e = "sub-command REGEX, mode REPLACE: Unknown escape \"";
        e += replace.substr(r, 2);
        e += "\"in replace-expression.";
        this->SetError(e.c_str());
        return false;
        }
      r += 2;
      }
    l = r;
    }
  
  // Concatenate all the last arguments together.
  std::string input = args[5];
  for(int i=6; i < args.size(); ++i)
    {
    input += args[i];
    }
  
  // Compile the regular expression.
  cmRegularExpression re;
  if(!re.compile(regex.c_str()))
    {
    std::string e = "sub-command REGEX, mode REPLACE failed to compile regex \""+regex+"\".";
    this->SetError(e.c_str());
    return false;
    }
  
  // Scan through the input for all matches.
  std::string output;
  std::string::size_type base = 0;
  while(re.find(input.c_str()+base))
    {
    std::string::size_type l = re.start();
    std::string::size_type r = re.end();
    
    // Concatenate the part of the input that was not matched.
    output += input.substr(base, l-base);
    
    // Make sure the match had some text.
    if(r-l == 0)
      {
      std::string e = "sub-command REGEX, mode REPLACE regex \""+regex+"\" matched an empty string.";
      this->SetError(e.c_str());
      return false;
      }
    
    // Concatenate the replacement for the match.
    for(int i=0; i < replacement.size(); ++i)
      {
      if(replacement[i].number < 0)
        {
        // This is just a plain-text part of the replacement.
        output += replacement[i].value;
        }
      else
        {
        // Replace with part of the match.
        int n = replacement[i].number;
        std::string::size_type start = re.start(n);
        std::string::size_type end = re.end(n);
        std::string::size_type len = input.length()-base;
        if((start != std::string::npos) && (end != std::string::npos) &&
           (start >= 0) && (start <= len) && (end >= 0) && (end <= len))
          {
          output += input.substr(base+l, r-l);
          }
        else
          {
          std::string e =
            "sub-command REGEX, mode REPLACE: replace expression \""+
            replace+"\" contains an out-of-range escape for regex \""+
            regex+"\".";
          this->SetError(e.c_str());
          return false;
          }
        }
      }
    
    // Move past the match.
    base += r;
    }
  
  // Concatenate the text after the last match.
  output += input.substr(base, input.length()-base);
  
  // Store the output in the provided variable.
  m_Makefile->AddDefinition(outvar.c_str(), output.c_str());
  return true;
}
