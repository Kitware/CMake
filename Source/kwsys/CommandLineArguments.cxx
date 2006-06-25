/*=========================================================================

  Program:   KWSys - Kitware System Library
  Module:    $RCSfile$

  Copyright (c) Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "kwsysPrivate.h"
#include KWSYS_HEADER(CommandLineArguments.hxx)

#include KWSYS_HEADER(Configure.hxx)

#include KWSYS_HEADER(stl/vector)
#include KWSYS_HEADER(stl/map)
#include KWSYS_HEADER(stl/set)
#include KWSYS_HEADER(ios/sstream)
#include KWSYS_HEADER(ios/iostream)

// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.
#if 0
# include "CommandLineArguments.hxx.in"
# include "Configure.hxx.in"
# include "kwsys_stl.hxx.in"
# include "kwsys_ios_sstream.h.in"
# include "kwsys_ios_iostream.h.in"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER
# pragma warning (disable: 4786)
#endif

#if defined(__sgi) && !defined(__GNUC__)
# pragma set woff 1375 /* base class destructor not virtual */
#endif

namespace KWSYS_NAMESPACE
{

//----------------------------------------------------------------------------
//============================================================================
class CommandLineArgumentsString : public kwsys_stl::string 
{
public:
  typedef kwsys_stl::string StdString;
  CommandLineArgumentsString(): StdString() {}
  CommandLineArgumentsString(const value_type* s): StdString(s) {}
  CommandLineArgumentsString(const value_type* s, size_type n): StdString(s, n) {}
  CommandLineArgumentsString(const StdString& s, size_type pos=0, size_type n=npos):
    StdString(s, pos, n) {}
};

struct CommandLineArgumentsCallbackStructure
{
  const char* Argument;
  int ArgumentType;
  CommandLineArguments::CallbackType Callback;
  void* CallData;
  void* Variable;
  int VariableType;
  const char* Help;
};
 
class CommandLineArgumentsVectorOfStrings : 
  public kwsys_stl::vector<CommandLineArgumentsString> {};
class CommandLineArgumentsSetOfStrings :
  public kwsys_stl::set<CommandLineArgumentsString> {};
class CommandLineArgumentsMapOfStrucs : 
  public kwsys_stl::map<CommandLineArgumentsString,
    CommandLineArgumentsCallbackStructure> {};

class CommandLineArgumentsInternal
{
public:
  CommandLineArgumentsInternal()
    {
    this->UnknownArgumentCallback = 0;
    this->ClientData = 0;
    this->LastArgument = 0;
    }

  typedef CommandLineArgumentsVectorOfStrings VectorOfStrings;
  typedef CommandLineArgumentsMapOfStrucs CallbacksMap;
  typedef CommandLineArgumentsString String;
  typedef CommandLineArgumentsSetOfStrings SetOfStrings;

  VectorOfStrings Argv;
  String Argv0;
  CallbacksMap Callbacks;

  CommandLineArguments::ErrorCallbackType UnknownArgumentCallback;
  void*             ClientData;

  VectorOfStrings::size_type LastArgument;
};
//============================================================================
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
CommandLineArguments::CommandLineArguments()
{
  this->Internals = new CommandLineArguments::Internal;
  this->Help = "";
  this->LineLength = 80;
}

//----------------------------------------------------------------------------
CommandLineArguments::~CommandLineArguments()
{
  delete this->Internals;
}

//----------------------------------------------------------------------------
void CommandLineArguments::Initialize(int argc, const char* const argv[])
{
  int cc;

  this->Initialize();
  this->Internals->Argv0 = argv[0];
  for ( cc = 1; cc < argc; cc ++ )
    {
    this->ProcessArgument(argv[cc]);
    }
}

//----------------------------------------------------------------------------
void CommandLineArguments::Initialize(int argc, char* argv[])
{
  this->Initialize(argc, static_cast<const char* const*>(argv));
}

//----------------------------------------------------------------------------
void CommandLineArguments::Initialize()
{
  this->Internals->Argv.clear();
  this->Internals->LastArgument = 0;
}

//----------------------------------------------------------------------------
void CommandLineArguments::ProcessArgument(const char* arg)
{
  this->Internals->Argv.push_back(arg);
}

//----------------------------------------------------------------------------
int CommandLineArguments::Parse()
{
  CommandLineArguments::Internal::VectorOfStrings::size_type cc;
  CommandLineArguments::Internal::VectorOfStrings matches;
  for ( cc = 0; cc < this->Internals->Argv.size(); cc ++ )
    {
    this->Internals->LastArgument = cc;
    matches.clear();
    CommandLineArguments::Internal::String& arg = this->Internals->Argv[cc];
    CommandLineArguments::Internal::CallbacksMap::iterator it;

    // Does the argument match to any we know about?
    for ( it = this->Internals->Callbacks.begin();
      it != this->Internals->Callbacks.end();
      it ++ )
      {
      const CommandLineArguments::Internal::String& parg = it->first;
      CommandLineArgumentsCallbackStructure *cs = &it->second;
      if (cs->ArgumentType == CommandLineArguments::NO_ARGUMENT ||
        cs->ArgumentType == CommandLineArguments::SPACE_ARGUMENT) 
        {
        if ( arg == parg )
          {
          matches.push_back(parg);
          }
        }
      else if ( arg.find( parg ) == 0 )
        {
        matches.push_back(parg);
        }
      }
    if ( matches.size() > 0 )
      {
      // Ok, we found one or more arguments that match what user specified.
      // Let's find the longest one.
      CommandLineArguments::Internal::VectorOfStrings::size_type kk;
      CommandLineArguments::Internal::VectorOfStrings::size_type maxidx = 0;
      CommandLineArguments::Internal::String::size_type maxlen = 0;
      for ( kk = 0; kk < matches.size(); kk ++ )
        {
        if ( matches[kk].size() > maxlen )
          {
          maxlen = matches[kk].size();
          maxidx = kk;
          }
        }
      // So, the longest one is probably the right one. Now see if it has any
      // additional value
      const char* value = 0;
      CommandLineArgumentsCallbackStructure *cs 
        = &this->Internals->Callbacks[matches[maxidx]];
      const CommandLineArguments::Internal::String& sarg = matches[maxidx];
      if ( cs->ArgumentType == NO_ARGUMENT )
        {
        // No value
        }
      else if ( cs->ArgumentType == SPACE_ARGUMENT )
        {
        if ( cc == this->Internals->Argv.size()-1 )
          {
          this->Internals->LastArgument --;
          return 0;
          }
        // Value is the next argument
        value = this->Internals->Argv[cc+1].c_str();
        cc ++;
        }
      else if ( cs->ArgumentType == EQUAL_ARGUMENT )
        {
        if ( arg.size() == sarg.size() || *(arg.c_str() + sarg.size()) != '=' )
          {
          this->Internals->LastArgument --;
          return 0;
          }
        // Value is everythng followed the '=' sign
        value = arg.c_str() + sarg.size()+1;
        }
      else if ( cs->ArgumentType == CONCAT_ARGUMENT )
        {
        // Value is whatever follows the argument
        value = arg.c_str() + sarg.size();
        }

      // Call the callback
      if ( cs->Callback )
        {
        if ( !cs->Callback(sarg.c_str(), value, cs->CallData) )
          {
          this->Internals->LastArgument --;
          return 0;
          }
        }
      if ( cs->Variable )
        {
        kwsys_stl::string var = "1";
        if ( value )
          {
          var = value;
          }
        if ( cs->VariableType == CommandLineArguments::INT_TYPE )
          {
          int* variable = static_cast<int*>(cs->Variable);
          char* res = 0;
          *variable = strtol(var.c_str(), &res, 10);
          //if ( res && *res )
          //  {
          //  Can handle non-int
          //  }
          }
        else if ( cs->VariableType == CommandLineArguments::DOUBLE_TYPE )
          {
          double* variable = static_cast<double*>(cs->Variable);
          char* res = 0;
          *variable = strtod(var.c_str(), &res);
          //if ( res && *res )
          //  {
          //  Can handle non-int
          //  }
          }
        else if ( cs->VariableType == CommandLineArguments::STRING_TYPE )
          {
          char** variable = static_cast<char**>(cs->Variable);
          if ( *variable )
            {
            delete [] *variable;
            *variable = 0;
            }
          *variable = new char[ strlen(var.c_str()) + 1 ];
          strcpy(*variable, var.c_str());
          }
        else if ( cs->VariableType == CommandLineArguments::STL_STRING_TYPE )
          {
          kwsys_stl::string* variable = static_cast<kwsys_stl::string*>(cs->Variable);
          *variable = var;
          }
        else if ( cs->VariableType == CommandLineArguments::BOOL_TYPE )
          {
          bool* variable = static_cast<bool*>(cs->Variable);
          if ( var == "1" || var == "ON" || var == "TRUE" || var == "true" || var == "on" ||
            var == "True" || var == "yes" || var == "Yes" || var == "YES" )
            {
            *variable = true;
            }
          else
            {
            *variable = false;
            }
          }
        else
          {
          kwsys_ios::cerr << "Got unknown argument type: \"" << cs->VariableType << "\"" << kwsys_ios::endl;
          this->Internals->LastArgument --;
          return 0;
          }
        }
      }
    else
      {
      // Handle unknown arguments
      if ( this->Internals->UnknownArgumentCallback )
        {
        if ( !this->Internals->UnknownArgumentCallback(arg.c_str(), 
            this->Internals->ClientData) )
          {
          this->Internals->LastArgument --;
          return 0;
          }
        return 1;
        }
      else
        {
        kwsys_ios::cerr << "Got unknown argument: \"" << arg.c_str() << "\"" << kwsys_ios::endl;
        this->Internals->LastArgument --;
        return 0;
        }
      }
    }
  return 1;
}

//----------------------------------------------------------------------------
void CommandLineArguments::GetRemainingArguments(int* argc, char*** argv)
{
  CommandLineArguments::Internal::VectorOfStrings::size_type size 
    = this->Internals->Argv.size() - this->Internals->LastArgument + 1;
  CommandLineArguments::Internal::VectorOfStrings::size_type cc;

  // Copy Argv0 as the first argument
  char** args = new char*[ size ];
  args[0] = new char[ this->Internals->Argv0.size() + 1 ];
  strcpy(args[0], this->Internals->Argv0.c_str());
  int cnt = 1;

  // Copy everything after the LastArgument, since that was not parsed.
  for ( cc = this->Internals->LastArgument+1; 
    cc < this->Internals->Argv.size(); cc ++ )
    {
    args[cnt] = new char[ this->Internals->Argv[cc].size() + 1];
    strcpy(args[cnt], this->Internals->Argv[cc].c_str());
    cnt ++;
    }
  *argc = cnt;
  *argv = args;
}

//----------------------------------------------------------------------------
void CommandLineArguments::DeleteRemainingArguments(int argc, char*** argv)
{
  int cc;
  for ( cc = 0; cc < argc; ++ cc )
    {
    delete [] (*argv)[cc];
    }
  delete [] *argv;
}

//----------------------------------------------------------------------------
void CommandLineArguments::AddCallback(const char* argument, ArgumentTypeEnum type, 
  CallbackType callback, void* call_data, const char* help)
{
  CommandLineArgumentsCallbackStructure s;
  s.Argument     = argument;
  s.ArgumentType = type;
  s.Callback     = callback;
  s.CallData     = call_data;
  s.VariableType = CommandLineArguments::NO_VARIABLE_TYPE;
  s.Variable     = 0;
  s.Help         = help;

  this->Internals->Callbacks[argument] = s;
  this->GenerateHelp();
}

//----------------------------------------------------------------------------
void CommandLineArguments::AddArgument(const char* argument, ArgumentTypeEnum type,
  VariableTypeEnum vtype, void* variable, const char* help)
{
  CommandLineArgumentsCallbackStructure s;
  s.Argument     = argument;
  s.ArgumentType = type;
  s.Callback     = 0;
  s.CallData     = 0;
  s.VariableType = vtype;
  s.Variable     = variable;
  s.Help         = help;

  this->Internals->Callbacks[argument] = s;
  this->GenerateHelp();
}

//----------------------------------------------------------------------------
void CommandLineArguments::AddArgument(const char* argument, ArgumentTypeEnum type,
  int* variable, const char* help)
{
  this->AddArgument(argument, type, CommandLineArguments::INT_TYPE, variable, help);
}

//----------------------------------------------------------------------------
void CommandLineArguments::AddArgument(const char* argument, ArgumentTypeEnum type,
  double* variable, const char* help)
{
  this->AddArgument(argument, type, CommandLineArguments::DOUBLE_TYPE, variable, help);
}

//----------------------------------------------------------------------------
void CommandLineArguments::AddArgument(const char* argument, ArgumentTypeEnum type,
  char** variable, const char* help)
{
  this->AddArgument(argument, type, CommandLineArguments::STRING_TYPE, variable, help);
}

//----------------------------------------------------------------------------
void CommandLineArguments::AddArgument(const char* argument, ArgumentTypeEnum type,
  kwsys_stl::string* variable, const char* help)
{
  this->AddArgument(argument, type, CommandLineArguments::STL_STRING_TYPE, variable, help);
}

//----------------------------------------------------------------------------
void CommandLineArguments::AddArgument(const char* argument, ArgumentTypeEnum type,
  bool* variable, const char* help)
{
  this->AddArgument(argument, type, CommandLineArguments::BOOL_TYPE, variable, help);
}

//----------------------------------------------------------------------------
void CommandLineArguments::AddBooleanArgument(const char* argument, bool*
  variable, const char* help)
{
  this->AddArgument(argument, CommandLineArguments::NO_ARGUMENT,
    CommandLineArguments::BOOL_TYPE, variable, help);
}

//----------------------------------------------------------------------------
void CommandLineArguments::AddBooleanArgument(const char* argument, int*
  variable, const char* help)
{
  this->AddArgument(argument, CommandLineArguments::NO_ARGUMENT,
    CommandLineArguments::INT_TYPE, variable, help);
}

//----------------------------------------------------------------------------
void CommandLineArguments::SetClientData(void* client_data)
{
  this->Internals->ClientData = client_data;
}

//----------------------------------------------------------------------------
void CommandLineArguments::SetUnknownArgumentCallback(
  CommandLineArguments::ErrorCallbackType callback)
{
  this->Internals->UnknownArgumentCallback = callback;
}

//----------------------------------------------------------------------------
const char* CommandLineArguments::GetHelp(const char* arg)
{
  CommandLineArguments::Internal::CallbacksMap::iterator it 
    = this->Internals->Callbacks.find(arg);
  if ( it == this->Internals->Callbacks.end() )
    {
    return 0;
    }

  // Since several arguments may point to the same argument, find the one this
  // one point to if this one is pointing to another argument.
  CommandLineArgumentsCallbackStructure *cs = &(it->second);
  for(;;)
    {
    CommandLineArguments::Internal::CallbacksMap::iterator hit 
      = this->Internals->Callbacks.find(cs->Help);
    if ( hit == this->Internals->Callbacks.end() )
      {
      break;
      }
    cs = &(hit->second);
    }
  return cs->Help;
}

//----------------------------------------------------------------------------
void CommandLineArguments::SetLineLength(unsigned int ll)
{
  if ( ll < 9 || ll > 1000 )
    {
    return;
    }
  this->LineLength = ll;
  this->GenerateHelp();
}

//----------------------------------------------------------------------------
const char* CommandLineArguments::GetArgv0()
{
  return this->Internals->Argv0.c_str();
}

//----------------------------------------------------------------------------
unsigned int CommandLineArguments::GetLastArgument()
{
  return (unsigned int)this->Internals->LastArgument + 1;
}

//----------------------------------------------------------------------------
void CommandLineArguments::GenerateHelp()
{
  kwsys_ios::ostringstream str;

  // Collapse all arguments into the map of vectors of all arguments that do
  // the same thing.
  CommandLineArguments::Internal::CallbacksMap::iterator it;
  typedef kwsys_stl::map<CommandLineArguments::Internal::String, 
     CommandLineArguments::Internal::SetOfStrings > MapArgs;
  MapArgs mp;
  MapArgs::iterator mpit, smpit;
  for ( it = this->Internals->Callbacks.begin();
    it != this->Internals->Callbacks.end();
    it ++ )
    {
    CommandLineArgumentsCallbackStructure *cs = &(it->second);
    mpit = mp.find(cs->Help);
    if ( mpit != mp.end() )
      {
      mpit->second.insert(it->first);
      mp[it->first].insert(it->first);
      }
    else
      {
      mp[it->first].insert(it->first);
      }
    }
  for ( it = this->Internals->Callbacks.begin();
    it != this->Internals->Callbacks.end();
    it ++ )
    {
    CommandLineArgumentsCallbackStructure *cs = &(it->second);
    mpit = mp.find(cs->Help);
    if ( mpit != mp.end() )
      {
      mpit->second.insert(it->first);
      smpit = mp.find(it->first);
      CommandLineArguments::Internal::SetOfStrings::iterator sit;
      for ( sit = smpit->second.begin(); sit != smpit->second.end(); sit++ )
        {
        mpit->second.insert(*sit);
        }
      mp.erase(smpit);
      }
    else
      {
      mp[it->first].insert(it->first);
      }
    }
 
  // Find the length of the longest string
  CommandLineArguments::Internal::String::size_type maxlen = 0;
  for ( mpit = mp.begin();
    mpit != mp.end();
    mpit ++ )
    {
    CommandLineArguments::Internal::SetOfStrings::iterator sit;
    for ( sit = mpit->second.begin(); sit != mpit->second.end(); sit++ )
      {
      CommandLineArguments::Internal::String::size_type clen = sit->size();
      switch ( this->Internals->Callbacks[*sit].ArgumentType )
        {
        case CommandLineArguments::NO_ARGUMENT:     clen += 0; break;
        case CommandLineArguments::CONCAT_ARGUMENT: clen += 3; break;
        case CommandLineArguments::SPACE_ARGUMENT:  clen += 4; break;
        case CommandLineArguments::EQUAL_ARGUMENT:  clen += 4; break;
        }
      if ( clen > maxlen )
        {
        maxlen = clen;
        }
      }
    }

  // Create format for that string
  char format[80];
  sprintf(format, "  %%-%ds  ", static_cast<unsigned int>(maxlen));

  maxlen += 4; // For the space before and after the option

  // Print help for each option
  for ( mpit = mp.begin();
    mpit != mp.end();
    mpit ++ )
    {
    CommandLineArguments::Internal::SetOfStrings::iterator sit;
    for ( sit = mpit->second.begin(); sit != mpit->second.end(); sit++ )
      {
      str << kwsys_ios::endl;
      char argument[100];
      sprintf(argument, sit->c_str());
      switch ( this->Internals->Callbacks[*sit].ArgumentType )
        {
        case CommandLineArguments::NO_ARGUMENT: break;
        case CommandLineArguments::CONCAT_ARGUMENT: strcat(argument, "opt"); break;
        case CommandLineArguments::SPACE_ARGUMENT:  strcat(argument, " opt"); break;
        case CommandLineArguments::EQUAL_ARGUMENT:  strcat(argument, "=opt"); break;
        }
      char buffer[80];
      sprintf(buffer, format, argument);
      str << buffer;
      }
    const char* ptr = this->Internals->Callbacks[mpit->first].Help;
    size_t len = strlen(ptr);
    int cnt = 0;
    while ( len > 0)
      {
      // If argument with help is longer than line length, split it on previous
      // space (or tab) and continue on the next line
      CommandLineArguments::Internal::String::size_type cc;
      for ( cc = 0; ptr[cc]; cc ++ )
        {
        if ( *ptr == ' ' || *ptr == '\t' )
          {
          ptr ++;
          len --;
          }
        }
      if ( cnt > 0 )
        {
        for ( cc = 0; cc < maxlen; cc ++ )
          {
          str << " ";
          }
        }
      CommandLineArguments::Internal::String::size_type skip = len;
      if ( skip > this->LineLength - maxlen )
        {
        skip = this->LineLength - maxlen;
        for ( cc = skip-1; cc > 0; cc -- )
          {
          if ( ptr[cc] == ' ' || ptr[cc] == '\t' )
            {
            break;
            }
          }
        if ( cc != 0 )
          {
          skip = cc;
          }
        }
      str.write(ptr, skip);
      str << kwsys_ios::endl;
      ptr += skip;
      len -= skip;
      cnt ++;
      }
    }
  /*
  // This can help debugging help string
  str << endl;
  unsigned int cc;
  for ( cc = 0; cc < this->LineLength; cc ++ )
    {
    str << cc % 10;
    }
  str << endl;
  */
  this->Help = str.str();
}

} // namespace KWSYS_NAMESPACE
