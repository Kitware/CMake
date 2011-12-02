/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmStringCommand.h"
#include "cmCryptoHash.h"

#include <cmsys/RegularExpression.hxx>
#include <cmsys/SystemTools.hxx>

#include <stdlib.h> // required for atoi
#include <ctype.h>
#include <time.h>

//----------------------------------------------------------------------------
bool cmStringCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 1)
    {
    this->SetError("must be called with at least one argument.");
    return false;
    }
  
  const std::string &subCommand = args[0];
  if(subCommand == "REGEX")
    {
    return this->HandleRegexCommand(args);
    }
  else if(subCommand == "REPLACE")
    {
    return this->HandleReplaceCommand(args);
    }
  else if ( subCommand == "MD5" ||
            subCommand == "SHA1" ||
            subCommand == "SHA224" ||
            subCommand == "SHA256" ||
            subCommand == "SHA384" ||
            subCommand == "SHA512" )
    {
    return this->HandleHashCommand(args);
    }
  else if(subCommand == "TOLOWER")
    {
    return this->HandleToUpperLowerCommand(args, false);
    }
  else if(subCommand == "TOUPPER")
    {
    return this->HandleToUpperLowerCommand(args, true);
    }
  else if(subCommand == "COMPARE")
    {
    return this->HandleCompareCommand(args);
    }
  else if(subCommand == "ASCII")
    {
    return this->HandleAsciiCommand(args);
    }
  else if(subCommand == "CONFIGURE")
    {
    return this->HandleConfigureCommand(args);
    }
  else if(subCommand == "LENGTH")
    {
    return this->HandleLengthCommand(args);
    }
  else if(subCommand == "SUBSTRING")
    {
    return this->HandleSubstringCommand(args);
    }
  else if(subCommand == "STRIP")
    {
    return this->HandleStripCommand(args);
    }
  else if(subCommand == "RANDOM")
    {
    return this->HandleRandomCommand(args);
    }
  else if(subCommand == "FIND")
    {
    return this->HandleFindCommand(args);
    }

  std::string e = "does not recognize sub-command "+subCommand;
  this->SetError(e.c_str());
  return false;
}

//----------------------------------------------------------------------------
bool cmStringCommand::HandleHashCommand(std::vector<std::string> const& args)
{
#if defined(CMAKE_BUILD_WITH_CMAKE)
  if(args.size() != 3)
    {
    cmOStringStream e;
    e << args[0] << " requires an output variable and an input string";
    this->SetError(e.str().c_str());
    return false;
    }

  cmsys::auto_ptr<cmCryptoHash> hash(cmCryptoHash::New(args[0].c_str()));
  if(hash.get())
    {
    std::string out = hash->HashString(args[2].c_str());
    this->Makefile->AddDefinition(args[1].c_str(), out.c_str());
    return true;
    }
  return false;
#else
  cmOStringStream e;
  e << args[0] << " not available during bootstrap";
  this->SetError(e.str().c_str());
  return false;
#endif
}

//----------------------------------------------------------------------------
bool cmStringCommand::HandleToUpperLowerCommand(
  std::vector<std::string> const& args, bool toUpper)
{
  if ( args.size() < 3 )
    {
    this->SetError("no output variable specified");
    return false;
    }

  std::string outvar = args[2];
  std::string output;

  if (toUpper) 
    {
    output = cmSystemTools::UpperCase(args[1]);
    } 
  else 
    {
    output = cmSystemTools::LowerCase(args[1]);
    }

  // Store the output in the provided variable.
  this->Makefile->AddDefinition(outvar.c_str(), output.c_str());
  return true;
}

//----------------------------------------------------------------------------
bool cmStringCommand::HandleAsciiCommand(std::vector<std::string> const& args)
{
  if ( args.size() < 3 )
    {
    this->SetError("No output variable specified");
    return false;
    }
  std::string::size_type cc;
  std::string outvar = args[args.size()-1];
  std::string output = "";
  for ( cc = 1; cc < args.size()-1; cc ++ )
    {
    int ch = atoi(args[cc].c_str());
    if ( ch > 0 && ch < 256 )
      {
      output += static_cast<char>(ch);
      }
    else
      {
      std::string error = "Character with code ";
      error += args[cc];
      error += " does not exist.";
      this->SetError(error.c_str());
      return false;
      }
    }
  // Store the output in the provided variable.
  this->Makefile->AddDefinition(outvar.c_str(), output.c_str());
  return true;
}

//----------------------------------------------------------------------------
bool cmStringCommand::HandleConfigureCommand(
  std::vector<std::string> const& args)
{
  if ( args.size() < 2 )
    {
    this->SetError("No input string specified.");
    return false;
    }
  else if ( args.size() < 3 )
    {
    this->SetError("No output variable specified.");
    return false;
    }

  // Parse options.
  bool escapeQuotes = false;
  bool atOnly = false;
  for(unsigned int i = 3; i < args.size(); ++i)
    {
    if(args[i] == "@ONLY")
      {
      atOnly = true;
      }
    else if(args[i] == "ESCAPE_QUOTES")
      {
      escapeQuotes = true;
      }
    else
      {
      cmOStringStream err;
      err << "Unrecognized argument \"" << args[i] << "\"";
      this->SetError(err.str().c_str());
      return false;
      }
    }

  // Configure the string.
  std::string output;
  this->Makefile->ConfigureString(args[1], output, atOnly, escapeQuotes);

  // Store the output in the provided variable.
  this->Makefile->AddDefinition(args[2].c_str(), output.c_str());

  return true;
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
      this->SetError("sub-command REGEX, mode REPLACE needs "
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
  //"STRING(REGEX MATCH <regular_expression> <output variable>
  // <input> [<input>...])\n";
  std::string regex = args[2];
  std::string outvar = args[3];
  
  // Concatenate all the last arguments together.
  std::string input = args[4];
  for(unsigned int i=5; i < args.size(); ++i)
    {
    input += args[i];
    }
  
  this->ClearMatches(this->Makefile);
  // Compile the regular expression.
  cmsys::RegularExpression re;
  if(!re.compile(regex.c_str()))
    {
    std::string e = 
      "sub-command REGEX, mode MATCH failed to compile regex \""+regex+"\".";
    this->SetError(e.c_str());
    return false;
    }
  
  // Scan through the input for all matches.
  std::string output;
  if(re.find(input.c_str()))
    {
    this->StoreMatches(this->Makefile, re);
    std::string::size_type l = re.start();
    std::string::size_type r = re.end();
    if(r-l == 0)
      {
      std::string e = 
        "sub-command REGEX, mode MATCH regex \""+regex+
        "\" matched an empty string.";
      this->SetError(e.c_str());
      return false;
      }
    output = input.substr(l, r-l);
    }
  
  // Store the output in the provided variable.
  this->Makefile->AddDefinition(outvar.c_str(), output.c_str());
  return true;
}

//----------------------------------------------------------------------------
bool cmStringCommand::RegexMatchAll(std::vector<std::string> const& args)
{
  //"STRING(REGEX MATCHALL <regular_expression> <output variable> <input> 
  // [<input>...])\n";
  std::string regex = args[2];
  std::string outvar = args[3];
  
  // Concatenate all the last arguments together.
  std::string input = args[4];
  for(unsigned int i=5; i < args.size(); ++i)
    {
    input += args[i];
    }
  
  this->ClearMatches(this->Makefile);
  // Compile the regular expression.
  cmsys::RegularExpression re;
  if(!re.compile(regex.c_str()))
    {
    std::string e =
      "sub-command REGEX, mode MATCHALL failed to compile regex \""+
      regex+"\".";
    this->SetError(e.c_str());
    return false;
    }
  
  // Scan through the input for all matches.
  std::string output;
  const char* p = input.c_str();
  while(re.find(p))
    {
    this->StoreMatches(this->Makefile, re);
    std::string::size_type l = re.start();
    std::string::size_type r = re.end();
    if(r-l == 0)
      {
      std::string e = "sub-command REGEX, mode MATCHALL regex \""+
        regex+"\" matched an empty string.";
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
  this->Makefile->AddDefinition(outvar.c_str(), output.c_str());
  return true;
}

//----------------------------------------------------------------------------
bool cmStringCommand::RegexReplace(std::vector<std::string> const& args)
{
  //"STRING(REGEX REPLACE <regular_expression> <replace_expression> 
  // <output variable> <input> [<input>...])\n"
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
        e += "\" in replace-expression.";
        this->SetError(e.c_str());
        return false;
        }
      r += 2;
      }
    l = r;
    }
  
  // Concatenate all the last arguments together.
  std::string input = args[5];
  for(unsigned int i=6; i < args.size(); ++i)
    {
    input += args[i];
    }
  
  this->ClearMatches(this->Makefile);
  // Compile the regular expression.
  cmsys::RegularExpression re;
  if(!re.compile(regex.c_str()))
    {
    std::string e = 
      "sub-command REGEX, mode REPLACE failed to compile regex \""+
      regex+"\".";
    this->SetError(e.c_str());
    return false;
    }
  
  // Scan through the input for all matches.
  std::string output;
  std::string::size_type base = 0;
  while(re.find(input.c_str()+base))
    {
    this->StoreMatches(this->Makefile, re);
    std::string::size_type l2 = re.start();
    std::string::size_type r = re.end();
    
    // Concatenate the part of the input that was not matched.
    output += input.substr(base, l2);
    
    // Make sure the match had some text.
    if(r-l2 == 0)
      {
      std::string e = "sub-command REGEX, mode REPLACE regex \""+
        regex+"\" matched an empty string.";
      this->SetError(e.c_str());
      return false;
      }
    
    // Concatenate the replacement for the match.
    for(unsigned int i=0; i < replacement.size(); ++i)
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
           (start <= len) && (end <= len))
          {
          output += input.substr(base+start, end-start);
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
  this->Makefile->AddDefinition(outvar.c_str(), output.c_str());
  return true;
}

//----------------------------------------------------------------------------
void cmStringCommand::ClearMatches(cmMakefile* mf)
{
  for (unsigned int i=0; i<10; i++)
    {
    char name[128];
    sprintf(name, "CMAKE_MATCH_%d", i);
    mf->AddDefinition(name, "");
    mf->MarkVariableAsUsed(name);
    }
}

//----------------------------------------------------------------------------
void cmStringCommand::StoreMatches(cmMakefile* mf,cmsys::RegularExpression& re)
{
  for (unsigned int i=0; i<10; i++)
    {
    char name[128];
    sprintf(name, "CMAKE_MATCH_%d", i);
    mf->AddDefinition(name, re.match(i).c_str());
    mf->MarkVariableAsUsed(name);
    }
}

//----------------------------------------------------------------------------
bool cmStringCommand::HandleFindCommand(std::vector<std::string> const&
                                           args)
{
  // check if all required parameters were passed
  if(args.size() < 4 || args.size() > 5)
    {
    this->SetError("sub-command FIND requires 3 or 4 parameters.");
    return false;
    }

  // check if the reverse flag was set or not
  bool reverseMode = false;
  if(args.size() == 5 && args[4] == "REVERSE")
    {
    reverseMode = true;
    }

  // if we have 5 arguments the last one must be REVERSE
  if(args.size() == 5 && args[4] != "REVERSE")
    {
    this->SetError("sub-command FIND: unknown last parameter");
    return false;
    }

  // local parameter names.
  const std::string& sstring = args[1];
  const std::string& schar = args[2];
  const std::string& outvar = args[3];

  // ensure that the user cannot accidentally specify REVERSE as a variable
  if(outvar == "REVERSE")
    {
    this->SetError("sub-command FIND does not allow to select REVERSE as "
                   "the output variable.  "
                   "Maybe you missed the actual output variable?");
    return false;
    }

  // try to find the character and return its position
  size_t pos;
  if(!reverseMode)
    {
    pos = sstring.find(schar);
    }
  else
    {
    pos = sstring.rfind(schar);
    }
  if(std::string::npos != pos)
    {
    cmOStringStream s;
    s << pos;
    this->Makefile->AddDefinition(outvar.c_str(), s.str().c_str());
    return true;
    }

  // the character was not found, but this is not really an error
  this->Makefile->AddDefinition(outvar.c_str(), "-1");
  return true;
}

//----------------------------------------------------------------------------
bool cmStringCommand::HandleCompareCommand(std::vector<std::string> const&
                                           args)
{
  if(args.size() < 2)
    {
    this->SetError("sub-command COMPARE requires a mode to be specified.");
    return false;
    }
  std::string mode = args[1];
  if((mode == "EQUAL") || (mode == "NOTEQUAL") ||
     (mode == "LESS") || (mode == "GREATER"))
    {
    if(args.size() < 5)
      {
      std::string e = "sub-command COMPARE, mode ";
      e += mode;
      e += " needs at least 5 arguments total to command.";
      this->SetError(e.c_str());
      return false;
      }
    
    const std::string& left = args[2];
    const std::string& right = args[3];  
    const std::string& outvar = args[4];
    bool result;
    if(mode == "LESS")
      {
      result = (left < right);
      }
    else if(mode == "GREATER")
      {
      result = (left > right);
      }
    else if(mode == "EQUAL")
      {
      result = (left == right);
      }
    else // if(mode == "NOTEQUAL")
      {
      result = !(left == right);
      }
    if(result)
      {
      this->Makefile->AddDefinition(outvar.c_str(), "1");
      }
    else
      {
      this->Makefile->AddDefinition(outvar.c_str(), "0");
      }
    return true;
    }  
  std::string e = "sub-command COMPARE does not recognize mode "+mode;
  this->SetError(e.c_str());
  return false;
}

//----------------------------------------------------------------------------
bool cmStringCommand::HandleReplaceCommand(std::vector<std::string> const&
                                           args)
{
  if(args.size() < 5)
    {
    this->SetError("sub-command REPLACE requires at least four arguments.");
    return false;
    }

  const std::string& matchExpression = args[1];
  const std::string& replaceExpression = args[2];
  const std::string& variableName = args[3];

  std::string input = args[4];
  for(unsigned int i=5; i < args.size(); ++i)
    {
    input += args[i];
    }

  cmsys::SystemTools::ReplaceString(input, matchExpression.c_str(), 
                                    replaceExpression.c_str());

  this->Makefile->AddDefinition(variableName.c_str(), input.c_str());
  return true;
}

//----------------------------------------------------------------------------
bool cmStringCommand::HandleSubstringCommand(std::vector<std::string> const& 
                                             args)
{
  if(args.size() != 5)
    {
    this->SetError("sub-command SUBSTRING requires four arguments.");
    return false;
    }

  const std::string& stringValue = args[1];
  int begin = atoi(args[2].c_str());
  int end = atoi(args[3].c_str());
  const std::string& variableName = args[4];

  size_t stringLength = stringValue.size();
  int intStringLength = static_cast<int>(stringLength);
  if ( begin < 0 || begin > intStringLength )
    {
    cmOStringStream ostr;
    ostr << "begin index: " << begin << " is out of range 0 - "
         << stringLength;
    this->SetError(ostr.str().c_str());
    return false;
    }
  int leftOverLength = intStringLength - begin;
  if ( end < -1 || end > leftOverLength )
    {
    cmOStringStream ostr;
    ostr << "end index: " << end << " is out of range -1 - "
         << leftOverLength;
    this->SetError(ostr.str().c_str());
    return false;
    }

  this->Makefile->AddDefinition(variableName.c_str(), 
                                stringValue.substr(begin, end).c_str());
  return true;
}

//----------------------------------------------------------------------------
bool cmStringCommand
::HandleLengthCommand(std::vector<std::string> const& args)
{
  if(args.size() != 3)
    {
    this->SetError("sub-command LENGTH requires two arguments.");
    return false;
    }

  const std::string& stringValue = args[1];
  const std::string& variableName = args[2];

  size_t length = stringValue.size();
  char buffer[1024];
  sprintf(buffer, "%d", static_cast<int>(length));

  this->Makefile->AddDefinition(variableName.c_str(), buffer);
  return true;
}

//----------------------------------------------------------------------------
bool cmStringCommand::HandleStripCommand(
  std::vector<std::string> const& args)
{
 if(args.size() != 3)
    {
    this->SetError("sub-command STRIP requires two arguments.");
    return false;
    }

  const std::string& stringValue = args[1];
  const std::string& variableName = args[2];
  size_t inStringLength = stringValue.size();
  size_t startPos = inStringLength + 1;
  size_t endPos = 0;
  const char* ptr = stringValue.c_str();
  size_t cc;
  for ( cc = 0; cc < inStringLength; ++ cc )
    {
    if ( !isspace(*ptr) )
      {
      if ( startPos > inStringLength )
        {
        startPos = cc;
        }
      endPos = cc;
      }
    ++ ptr;
    }

  size_t outLength = 0;

  // if the input string didn't contain any non-space characters, return 
  // an empty string
  if (startPos > inStringLength)
    {
    outLength = 0;
    startPos = 0;
    }
  else
    {
    outLength=endPos - startPos + 1;
    }

  this->Makefile->AddDefinition(variableName.c_str(),
    stringValue.substr(startPos, outLength).c_str());
  return true;
}

//----------------------------------------------------------------------------
bool cmStringCommand
::HandleRandomCommand(std::vector<std::string> const& args)
{
  if(args.size() < 2 || args.size() == 3 || args.size() == 5)
    {
    this->SetError("sub-command RANDOM requires at least one argument.");
    return false;
    }

  static bool seeded = false;
  bool force_seed = false;
  unsigned int seed = 0;
  int length = 5;
  const char cmStringCommandDefaultAlphabet[] = "qwertyuiopasdfghjklzxcvbnm"
    "QWERTYUIOPASDFGHJKLZXCVBNM"
    "0123456789";
  std::string alphabet;

  if ( args.size() > 3 )
    {
    size_t i = 1;
    size_t stopAt = args.size() - 2;

    for ( ; i < stopAt; ++i )
      {
      if ( args[i] == "LENGTH" )
        {
        ++i;
        length = atoi(args[i].c_str());
        }
      else if ( args[i] == "ALPHABET" )
        {
        ++i;
        alphabet = args[i];
        }
      else if ( args[i] == "RANDOM_SEED" )
        {
        ++i;
        seed = static_cast<unsigned int>(atoi(args[i].c_str()));
        force_seed = true;
        }
      }
    }
  if ( !alphabet.size() )
    {
    alphabet = cmStringCommandDefaultAlphabet;
    }

  double sizeofAlphabet = static_cast<double>(alphabet.size());
  if ( sizeofAlphabet < 1 )
    {
    this->SetError("sub-command RANDOM invoked with bad alphabet.");
    return false;
    }
  if ( length < 1 )
    {
    this->SetError("sub-command RANDOM invoked with bad length.");
    return false;
    }
  const std::string& variableName = args[args.size()-1];

  std::vector<char> result;

  if (!seeded || force_seed)
    {
    seeded = true;
    srand(force_seed? seed : cmSystemTools::RandomSeed());
    }

  const char* alphaPtr = alphabet.c_str();
  int cc;
  for ( cc = 0; cc < length; cc ++ )
    {
    int idx=(int) (sizeofAlphabet* rand()/(RAND_MAX+1.0));
    result.push_back(*(alphaPtr + idx));
    }
  result.push_back(0);

  this->Makefile->AddDefinition(variableName.c_str(), &*result.begin());
  return true;
}
