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

//----------------------------------------------------------------------------
struct cmListFileParser
{
  cmListFileParser(cmListFile* lf, cmMakefile* mf, const char* filename);
  ~cmListFileParser();
  bool ParseFile();
  bool ParseFunction(const char* name, long line);
  void AddArgument(cmListFileLexer_Token* token,
                   cmListFileArgument::Delimiter delim);
  cmListFile* ListFile;
  cmMakefile* Makefile;
  const char* FileName;
  cmListFileLexer* Lexer;
  cmListFileFunction Function;
  enum { SeparationOkay, SeparationWarning } Separation;
};

//----------------------------------------------------------------------------
cmListFileParser::cmListFileParser(cmListFile* lf, cmMakefile* mf,
                                   const char* filename):
  ListFile(lf), Makefile(mf), FileName(filename),
  Lexer(cmListFileLexer_New())
{
}

//----------------------------------------------------------------------------
cmListFileParser::~cmListFileParser()
{
  cmListFileLexer_Delete(this->Lexer);
}

//----------------------------------------------------------------------------
bool cmListFileParser::ParseFile()
{
  // Open the file.
  if(!cmListFileLexer_SetFileName(this->Lexer, this->FileName))
    {
    cmSystemTools::Error("cmListFileCache: error can not open file ",
                         this->FileName);
    return false;
    }

  // Use a simple recursive-descent parser to process the token
  // stream.
  bool haveNewline = true;
  while(cmListFileLexer_Token* token =
        cmListFileLexer_Scan(this->Lexer))
    {
    if(token->type == cmListFileLexer_Token_Space)
      {
      }
    else if(token->type == cmListFileLexer_Token_Newline)
      {
      haveNewline = true;
      }
    else if(token->type == cmListFileLexer_Token_Identifier)
      {
      if(haveNewline)
        {
        haveNewline = false;
        if(this->ParseFunction(token->text, token->line))
          {
          this->ListFile->Functions.push_back(this->Function);
          }
        else
          {
          return false;
          }
        }
      else
        {
        cmOStringStream error;
        error << "Error in cmake code at\n"
              << this->FileName << ":" << token->line << ":\n"
              << "Parse error.  Expected a newline, got "
              << cmListFileLexer_GetTypeAsString(this->Lexer, token->type)
              << " with text \"" << token->text << "\".";
        cmSystemTools::Error(error.str().c_str());
        return false;
        }
      }
    else
      {
      cmOStringStream error;
      error << "Error in cmake code at\n"
            << this->FileName << ":" << token->line << ":\n"
            << "Parse error.  Expected a command name, got "
            << cmListFileLexer_GetTypeAsString(this->Lexer, token->type)
            << " with text \""
            << token->text << "\".";
      cmSystemTools::Error(error.str().c_str());
      return false;
      }
    }
  return true;
}

//----------------------------------------------------------------------------
bool cmListFile::ParseFile(const char* filename,
                           bool topLevel,
                           cmMakefile *mf)
{
  if(!cmSystemTools::FileExists(filename))
    {
    return false;
    }

  bool parseError = false;
  this->ModifiedTime = cmSystemTools::ModifiedTime(filename);

  {
  cmListFileParser parser(this, mf, filename);
  parseError = !parser.ParseFile();
  }

  if(parseError)
    {
    this->ModifiedTime = 0;
    }

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
      cmListFileArgument prj("Project", cmListFileArgument::Unquoted,
                             filename, 0);
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

//----------------------------------------------------------------------------
bool cmListFileParser::ParseFunction(const char* name, long line)
{
  // Inintialize a new function call.
  this->Function = cmListFileFunction();
  this->Function.FilePath = this->FileName;
  this->Function.Name = name;
  this->Function.Line = line;

  // Command name has already been parsed.  Read the left paren.
  cmListFileLexer_Token* token;
  while((token = cmListFileLexer_Scan(this->Lexer)) &&
        token->type == cmListFileLexer_Token_Space) {}
  if(!token)
    {
    cmOStringStream error;
    error << "Error in cmake code at\n" << this->FileName << ":"
          << cmListFileLexer_GetCurrentLine(this->Lexer) << ":\n"
          << "Parse error.  Function missing opening \"(\".";
    cmSystemTools::Error(error.str().c_str());
    return false;
    }
  if(token->type != cmListFileLexer_Token_ParenLeft)
    {
    cmOStringStream error;
    error << "Error in cmake code at\n" << this->FileName << ":"
          << cmListFileLexer_GetCurrentLine(this->Lexer) << ":\n"
          << "Parse error.  Expected \"(\", got "
          << cmListFileLexer_GetTypeAsString(this->Lexer, token->type)
          << " with text \"" << token->text << "\".";
    cmSystemTools::Error(error.str().c_str());
    return false;
    }

  // Arguments.
  unsigned long lastLine;
  unsigned long parenDepth = 0;
  this->Separation = SeparationOkay;
  while((lastLine = cmListFileLexer_GetCurrentLine(this->Lexer),
         token = cmListFileLexer_Scan(this->Lexer)))
    {
    if(token->type == cmListFileLexer_Token_Space ||
       token->type == cmListFileLexer_Token_Newline)
      {
      this->Separation = SeparationOkay;
      continue;
      }
    if(token->type == cmListFileLexer_Token_ParenLeft)
      {
      parenDepth++;
      this->Separation = SeparationOkay;
      this->AddArgument(token, cmListFileArgument::Unquoted);
      }
    else if(token->type == cmListFileLexer_Token_ParenRight)
      {
      if (parenDepth == 0)
        {
        return true;
        }
      parenDepth--;
      this->Separation = SeparationOkay;
      this->AddArgument(token, cmListFileArgument::Unquoted);
      this->Separation = SeparationWarning;
      }
    else if(token->type == cmListFileLexer_Token_Identifier ||
            token->type == cmListFileLexer_Token_ArgumentUnquoted)
      {
      this->AddArgument(token, cmListFileArgument::Unquoted);
      this->Separation = SeparationWarning;
      }
    else if(token->type == cmListFileLexer_Token_ArgumentQuoted)
      {
      this->AddArgument(token, cmListFileArgument::Quoted);
      this->Separation = SeparationWarning;
      }
    else
      {
      // Error.
      cmOStringStream error;
      error << "Error in cmake code at\n" << this->FileName << ":"
            << cmListFileLexer_GetCurrentLine(this->Lexer) << ":\n"
            << "Parse error.  Function missing ending \")\".  "
            << "Instead found "
            << cmListFileLexer_GetTypeAsString(this->Lexer, token->type)
            << " with text \"" << token->text << "\".";
      cmSystemTools::Error(error.str().c_str());
      return false;
      }
    }

  cmOStringStream error;
  error << "Error in cmake code at\n"
        << this->FileName << ":" << lastLine << ":\n"
        << "Parse error.  Function missing ending \")\".  "
        << "End of file reached.";
  cmSystemTools::Error(error.str().c_str());

  return false;
}

//----------------------------------------------------------------------------
void cmListFileParser::AddArgument(cmListFileLexer_Token* token,
                                   cmListFileArgument::Delimiter delim)
{
  cmListFileArgument a(token->text, delim, this->FileName, token->line);
  this->Function.Arguments.push_back(a);
  if(delim == cmListFileArgument::Unquoted)
    {
    // Warn about a future behavior change.
    const char* c = a.Value.c_str();
    if(*c++ == '[')
      {
      while(*c == '=')
        { ++c; }
      if(*c == '[')
        {
        cmOStringStream m;
        m << "Syntax Warning in cmake code at\n"
          << "  " << this->FileName << ":" << token->line << ":"
          << token->column << "\n"
          << "A future version of CMake may treat unquoted argument:\n"
          << "  " << a.Value << "\n"
          << "as an opening long bracket.  Double-quote the argument.";
        this->Makefile->IssueMessage(cmake::AUTHOR_WARNING, m.str().c_str());
        }
      }
    }
  if(this->Separation == SeparationOkay)
    {
    return;
    }
  cmOStringStream m;
  m << "Syntax Warning in cmake code at\n"
    << "  " << this->FileName << ":" << token->line << ":"
    << token->column << "\n"
    << "Argument not separated from preceding token by whitespace.";
  this->Makefile->IssueMessage(cmake::AUTHOR_WARNING, m.str().c_str());
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
