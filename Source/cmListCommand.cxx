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
#include "cmListCommand.h"
#include <cmsys/RegularExpression.hxx>
#include <cmsys/SystemTools.hxx>

#include <stdlib.h> // required for atoi
#include <ctype.h>
//----------------------------------------------------------------------------
bool cmListCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 1)
    {
    this->SetError("must be called with at least one argument.");
    return false;
    }
  
  const std::string &subCommand = args[0];
  if(subCommand == "LENGTH")
    {
    return this->HandleLengthCommand(args);
    }
  if(subCommand == "GET")
    {
    return this->HandleGetCommand(args);
    }
  if(subCommand == "APPEND")
    {
    return this->HandleAppendCommand(args);
    }
  if(subCommand == "FIND")
    {
    return this->HandleFindCommand(args);
    }
  if(subCommand == "INSERT")
    {
    return this->HandleInsertCommand(args);
    }
  if(subCommand == "REMOVE_AT")
    {
    return this->HandleRemoveAtCommand(args);
    }
  if(subCommand == "REMOVE_ITEM")
    {
    return this->HandleRemoveItemCommand(args);
    }
  if(subCommand == "SORT")
    {
    return this->HandleSortCommand(args);
    }
  if(subCommand == "REVERSE")
    {
    return this->HandleReverseCommand(args);
    }
  
  std::string e = "does not recognize sub-command "+subCommand;
  this->SetError(e.c_str());
  return false;
}

//----------------------------------------------------------------------------
bool cmListCommand::GetListString(std::string& listString, const char* var)
{
  if ( !var )
    {
    return false;
    }
  // get the old value
  const char* cacheValue
    = this->Makefile->GetDefinition(var);
  if(!cacheValue)
    {
    return false;
    }
  listString = cacheValue;
  return true;
}

//----------------------------------------------------------------------------
bool cmListCommand::GetList(std::vector<std::string>& list, const char* var)
{
  std::string listString;
  if ( !this->GetListString(listString, var) )
    {
    return false;
    }
  // expand the variable
  cmSystemTools::ExpandListArgument(listString, list);
  return true;
}

//----------------------------------------------------------------------------
bool cmListCommand::HandleLengthCommand(std::vector<std::string> const& args)
{
  if(args.size() != 3)
    {
    this->SetError("sub-command LENGTH requires two arguments.");
    return false;
    }

  const std::string& listName = args[1];
  const std::string& variableName = args[args.size() - 1];
  std::vector<std::string> varArgsExpanded;
  // do not check the return value here
  // if the list var is not found varArgsExpanded will have size 0
  // and we will return 0
  this->GetList(varArgsExpanded, listName.c_str());
  size_t length = varArgsExpanded.size();
  char buffer[1024];
  sprintf(buffer, "%d", static_cast<int>(length));

  this->Makefile->AddDefinition(variableName.c_str(), buffer);
  return true;
}

//----------------------------------------------------------------------------
bool cmListCommand::HandleGetCommand(std::vector<std::string> const& args)
{
  if(args.size() < 4)
    {
    this->SetError("sub-command GET requires at least three arguments.");
    return false;
    }

  const std::string& listName = args[1];
  const std::string& variableName = args[args.size() - 1];
  // expand the variable
  std::vector<std::string> varArgsExpanded;
  if ( !this->GetList(varArgsExpanded, listName.c_str()) )
    {
    this->Makefile->AddDefinition(variableName.c_str(), "NOTFOUND");
    return true;
    }

  std::string value;
  size_t cc;
  for ( cc = 2; cc < args.size()-1; cc ++ )
    {
    int item = atoi(args[cc].c_str());
    if (value.size())
      {
      value += ";";
      }
    size_t nitem = varArgsExpanded.size();
    if ( item < 0 )
      {
      item = (int)nitem + item;
      }
    if ( item < 0 || nitem <= (size_t)item )
      {
      cmOStringStream str;
      str << "index: " << item << " out of range (-" 
          << varArgsExpanded.size() << ", " 
          << varArgsExpanded.size()-1 << ")";
      this->SetError(str.str().c_str());
      return false;
      }
    value += varArgsExpanded[item];
    }

  this->Makefile->AddDefinition(variableName.c_str(), value.c_str());
  return true;
}

//----------------------------------------------------------------------------
bool cmListCommand::HandleAppendCommand(std::vector<std::string> const& args)
{
  if(args.size() < 3)
    {
    this->SetError("sub-command APPEND requires at least two arguments.");
    return false;
    }

  const std::string& listName = args[1];
  // expand the variable
  std::string listString;
  this->GetListString(listString, listName.c_str());
  size_t cc;
  for ( cc = 2; cc < args.size(); ++ cc )
    {
    if ( listString.size() )
      {
      listString += ";";
      }
    listString += args[cc];
    }

  this->Makefile->AddDefinition(listName.c_str(), listString.c_str());
  return true;
}

//----------------------------------------------------------------------------
bool cmListCommand::HandleFindCommand(std::vector<std::string> const& args)
{
  if(args.size() != 4)
    {
    this->SetError("sub-command FIND requires three arguments.");
    return false;
    }

  const std::string& listName = args[1];
  const std::string& variableName = args[args.size() - 1];
  // expand the variable
  std::vector<std::string> varArgsExpanded;
  if ( !this->GetList(varArgsExpanded, listName.c_str()) )
    {
    this->Makefile->AddDefinition(variableName.c_str(), "-1");
    return true;
    }

  std::vector<std::string>::iterator it;
  unsigned int index = 0;
  for ( it = varArgsExpanded.begin(); it != varArgsExpanded.end(); ++ it )
    {
    if ( *it == args[2] )
      {
      char indexString[32];
      sprintf(indexString, "%d", index);
      this->Makefile->AddDefinition(variableName.c_str(), indexString);
      return true;
      }
    index++;
    }

  this->Makefile->AddDefinition(variableName.c_str(), "-1");
  return true;
}

//----------------------------------------------------------------------------
bool cmListCommand::HandleInsertCommand(std::vector<std::string> const& args)
{
  if(args.size() < 4)
    {
    this->SetError("sub-command INSERT requires at least three arguments.");
    return false;
    }

  const std::string& listName = args[1];

  // expand the variable
  int item = atoi(args[2].c_str());
  std::vector<std::string> varArgsExpanded;
  if ( !this->GetList(varArgsExpanded, listName.c_str()) && item != 0)
    {
    cmOStringStream str;
    str << "index: " << item << " out of range (0, 0)";
    this->SetError(str.str().c_str());
    return false;
    }

  if ( varArgsExpanded.size() != 0 )
    {
  size_t nitem = varArgsExpanded.size();
  if ( item < 0 )
    {
    item = (int)nitem + item;
    }
  if ( item < 0 || nitem <= (size_t)item )
    {
    cmOStringStream str;
    str << "index: " << item << " out of range (-" 
        << varArgsExpanded.size() << ", " 
        << (varArgsExpanded.size() == 0?0:(varArgsExpanded.size()-1)) << ")";
    this->SetError(str.str().c_str());
    return false;
    }
    }
  size_t cc;
  size_t cnt = 0;
  for ( cc = 3; cc < args.size(); ++ cc )
    {
    varArgsExpanded.insert(varArgsExpanded.begin()+item+cnt, args[cc]);
    cnt ++;
    }

  std::string value;
  for ( cc = 0; cc < varArgsExpanded.size(); cc ++ )
    {
    if (value.size())
      {
      value += ";";
      }
    value += varArgsExpanded[cc];
    }

  this->Makefile->AddDefinition(listName.c_str(), value.c_str());
  return true;
}

//----------------------------------------------------------------------------
bool cmListCommand
::HandleRemoveItemCommand(std::vector<std::string> const& args)
{
  if(args.size() < 3)
    {
    this->SetError("sub-command REMOVE_ITEM requires two or more arguments.");
    return false;
    }

  const std::string& listName = args[1];
  // expand the variable
  std::vector<std::string> varArgsExpanded;
  if ( !this->GetList(varArgsExpanded, listName.c_str()) )
    {
    this->SetError("sub-command REMOVE_ITEM requires list to be present.");
    return false;
    }

  size_t cc;
  for ( cc = 2; cc < args.size(); ++ cc )
    {
    size_t kk = 0;
    while ( kk < varArgsExpanded.size() )
      {
      if ( varArgsExpanded[kk] == args[cc] )
        {
        varArgsExpanded.erase(varArgsExpanded.begin()+kk);
        }
      else
        {
      kk ++;
      }
    }
    }

  std::string value;
  for ( cc = 0; cc < varArgsExpanded.size(); cc ++ )
    {
    if (value.size())
      {
      value += ";";
      }
    value += varArgsExpanded[cc];
    }

  this->Makefile->AddDefinition(listName.c_str(), value.c_str());
  return true;
}

//----------------------------------------------------------------------------
bool cmListCommand
::HandleReverseCommand(std::vector<std::string> const& args)
{
  if(args.size() < 2)
    {
    this->SetError("sub-command REVERSE requires a list as an argument.");
    return false;
    }

  const std::string& listName = args[1];
  // expand the variable
  std::vector<std::string> varArgsExpanded;
  if ( !this->GetList(varArgsExpanded, listName.c_str()) )
    {
    this->SetError("sub-command REVERSE requires list to be present.");
    return false;
    }

  std::string value;
  std::vector<std::string>::reverse_iterator it;
  for ( it = varArgsExpanded.rbegin(); it != varArgsExpanded.rend(); ++ it )
    {
    if (value.size())
      {
      value += ";";
      }
    value += it->c_str();
    }

  this->Makefile->AddDefinition(listName.c_str(), value.c_str());
  return true;
}

//----------------------------------------------------------------------------
bool cmListCommand
::HandleSortCommand(std::vector<std::string> const& args)
{
  if(args.size() < 2)
    {
    this->SetError("sub-command SORT requires a list as an argument.");
    return false;
    }

  const std::string& listName = args[1];
  // expand the variable
  std::vector<std::string> varArgsExpanded;
  if ( !this->GetList(varArgsExpanded, listName.c_str()) )
    {
    this->SetError("sub-command SORT requires list to be present.");
    return false;
    }

  std::sort(varArgsExpanded.begin(), varArgsExpanded.end());

  std::string value;
  std::vector<std::string>::iterator it;
  for ( it = varArgsExpanded.begin(); it != varArgsExpanded.end(); ++ it )
    {
    if (value.size())
      {
      value += ";";
      }
    value += it->c_str();
    }

  this->Makefile->AddDefinition(listName.c_str(), value.c_str());
  return true;
}

//----------------------------------------------------------------------------
bool cmListCommand::HandleRemoveAtCommand(
  std::vector<std::string> const& args)
{
  if(args.size() < 3)
    {
    this->SetError("sub-command REMOVE_AT requires at least "
                   "two arguments.");
    return false;
    }

  const std::string& listName = args[1];
  // expand the variable
  std::vector<std::string> varArgsExpanded;
  if ( !this->GetList(varArgsExpanded, listName.c_str()) )
    {
    this->SetError("sub-command REMOVE_AT requires list to be present.");
    return false;
    }

  size_t cc;
  std::vector<size_t> removed;
  for ( cc = 2; cc < args.size(); ++ cc )
    {
    int item = atoi(args[cc].c_str());
    size_t nitem = varArgsExpanded.size();
    if ( item < 0 )
      {
      item = (int)nitem + item;
      }
    if ( item < 0 || nitem <= (size_t)item )
      {
      cmOStringStream str;
      str << "index: " << item << " out of range (-" 
          << varArgsExpanded.size() << ", " 
          << varArgsExpanded.size()-1 << ")";
      this->SetError(str.str().c_str());
      return false;
      }
    removed.push_back(static_cast<size_t>(item));
    }

  std::string value;
  for ( cc = 0; cc < varArgsExpanded.size(); ++ cc )
    {
    size_t kk;
    bool found = false;
    for ( kk = 0; kk < removed.size(); ++ kk )
      {
      if ( cc == removed[kk] )
        {
        found = true;
        }
      }

    if ( !found )
      {
      if (value.size())
        {
        value += ";";
        }
      value += varArgsExpanded[cc];
      }
    }

  this->Makefile->AddDefinition(listName.c_str(), value.c_str());
  return true;
}

