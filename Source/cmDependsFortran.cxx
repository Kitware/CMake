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
#include "cmDependsFortran.h"

#include "cmSystemTools.h"

#include "cmDependsFortranLexer.h"  /* Interface to lexer object.  */
#include "cmDependsFortranParser.h" /* Interface to parser object.  */

#include <stack>

//----------------------------------------------------------------------------
// Parser methods not included in generated interface.
extern "C"
{
  // Get the current buffer processed by the lexer.
  YY_BUFFER_STATE cmDependsFortranLexer_GetCurrentBuffer(yyscan_t yyscanner);

  // The parser entry point.
  int cmDependsFortran_yyparse(yyscan_t);
}

// Define parser object internal structure.
struct cmDependsFortranFile
{
  FILE* File;
  YY_BUFFER_STATE Buffer;
  std::string Directory;
};
struct cmDependsFortranParser_s
{
  cmDependsFortranParser_s(cmDependsFortran* self);
  ~cmDependsFortranParser_s();

  // Pointer back to the main class.
  cmDependsFortran* Self;

  // Lexical scanner instance.
  yyscan_t Scanner;

  // Stack of open files in the translation unit.
  std::stack<cmDependsFortranFile> FileStack;

  // Buffer for string literals.
  std::string TokenString;

  // Flag for whether lexer is reading from inside an interface.
  int InInterface;

  // Set of provided and required modules.
  std::set<cmStdString> Provides;
  std::set<cmStdString> Requires;

  // Set of files included in the translation unit.
  std::set<cmStdString> Includes;
};

//----------------------------------------------------------------------------
cmDependsFortran::cmDependsFortran(const char* dir, const char* targetFile):
  cmDepends(dir, targetFile),
  m_SourceFile(),
  m_IncludePath(0)
{
}

//----------------------------------------------------------------------------
cmDependsFortran::cmDependsFortran(const char* dir, const char* targetFile,
                                   const char* sourceFile,
                                   std::vector<std::string> const& includes):
  cmDepends(dir, targetFile),
  m_SourceFile(sourceFile),
  m_IncludePath(&includes)
{
}

//----------------------------------------------------------------------------
cmDependsFortran::~cmDependsFortran()
{
}

//----------------------------------------------------------------------------
bool cmDependsFortran::WriteDependencies(std::ostream& os)
{
  // Make sure this is a scanning instance.
  if(m_SourceFile == "")
    {
    cmSystemTools::Error("Cannot scan dependencies without an source file.");
    return false;
    }
  if(!m_IncludePath)
    {
    cmSystemTools::Error("Cannot scan dependencies without an include path.");
    return false;
    }

  // Create the parser object.
  cmDependsFortranParser parser(this);

  // Push on the starting file.
  cmDependsFortranParser_FilePush(&parser, m_SourceFile.c_str());

  // Parse the translation unit.
  if(cmDependsFortran_yyparse(parser.Scanner) != 0)
    {
    // Failed to parse the file.  Report failure to write dependencies.
    return false;
    }

  // Write the dependencies to the output stream.
  const std::set<cmStdString>& dependencies = parser.Includes;
  for(std::set<cmStdString>::const_iterator i=dependencies.begin();
      i != dependencies.end(); ++i)
    {
    os << m_TargetFile.c_str() << ": "
       << cmSystemTools::ConvertToOutputPath(i->c_str()).c_str()
       << std::endl;
    }
  os << std::endl;

  return true;
}

//----------------------------------------------------------------------------
bool cmDependsFortran::CheckDependencies(std::istream&)
{
  return true;
}

//----------------------------------------------------------------------------
bool cmDependsFortran::FindIncludeFile(const char* dir,
                                       const char* includeName,
                                       std::string& fileName)
{
  // If the file is a full path, include it directly.
  if(cmSystemTools::FileIsFullPath(includeName))
    {
    fileName = includeName;
    return cmSystemTools::FileExists(fileName.c_str());
    }
  else
    {
    // Check for the file in the directory containing the including
    // file.
    std::string fullName = dir;
    fullName += "/";
    fullName += includeName;
    if(cmSystemTools::FileExists(fullName.c_str()))
      {
      fileName = fullName;
      return true;
      }

    // Search the include path for the file.
    for(std::vector<std::string>::const_iterator i = m_IncludePath->begin();
        i != m_IncludePath->end(); ++i)
      {
      fullName = *i;
      fullName += "/";
      fullName += includeName;
      if(cmSystemTools::FileExists(fullName.c_str()))
        {
        fileName = fullName;
        return true;
        }
      }
    }
  return false;
}

//----------------------------------------------------------------------------
cmDependsFortranParser_s::cmDependsFortranParser_s(cmDependsFortran* self):
  Self(self)
{
  this->InInterface = 0;

  // Initialize the lexical scanner.
  cmDependsFortran_yylex_init(&this->Scanner);
  cmDependsFortran_yyset_extra(this, this->Scanner);

  // Create a dummy buffer that is never read but is the fallback
  // buffer when the last file is popped off the stack.
  YY_BUFFER_STATE buffer =
    cmDependsFortran_yy_create_buffer(0, 4, this->Scanner);
  cmDependsFortran_yy_switch_to_buffer(buffer, this->Scanner);
}

//----------------------------------------------------------------------------
cmDependsFortranParser_s::~cmDependsFortranParser_s()
{
  cmDependsFortran_yylex_destroy(this->Scanner);
}

//----------------------------------------------------------------------------
int cmDependsFortranParser_FilePush(cmDependsFortranParser* parser,
                                    const char* fname)
{
  // Open the new file and push it onto the stack.  Save the old
  // buffer with it on the stack.
  if(FILE* file = fopen(fname, "rb"))
    {
    YY_BUFFER_STATE current =
      cmDependsFortranLexer_GetCurrentBuffer(parser->Scanner);
    std::string dir = cmSystemTools::GetParentDirectory(fname);
    cmDependsFortranFile f = {file, current, dir};
    YY_BUFFER_STATE buffer =
      cmDependsFortran_yy_create_buffer(0, 16384, parser->Scanner);
    cmDependsFortran_yy_switch_to_buffer(buffer, parser->Scanner);
    parser->FileStack.push(f);
    return 1;
    }
  else
    {
    return 0;
    }
}

//----------------------------------------------------------------------------
int cmDependsFortranParser_FilePop(cmDependsFortranParser* parser)
{
  // Pop one file off the stack and close it.  Switch the lexer back
  // to the next one on the stack.
  if(parser->FileStack.empty())
    {
    return 0;
    }
  else
    {
    cmDependsFortranFile f = parser->FileStack.top(); parser->FileStack.pop();
    fclose(f.File);
    YY_BUFFER_STATE current =
      cmDependsFortranLexer_GetCurrentBuffer(parser->Scanner);
    cmDependsFortran_yy_delete_buffer(current, parser->Scanner);
    cmDependsFortran_yy_switch_to_buffer(f.Buffer, parser->Scanner);
    return 1;
    }
}

//----------------------------------------------------------------------------
int cmDependsFortranParser_Input(cmDependsFortranParser* parser,
                                 char* buffer, size_t bufferSize)
{
  // Read from the file on top of the stack.  If the stack is empty,
  // the end of the translation unit has been reached.
  if(!parser->FileStack.empty())
    {
    FILE* file = parser->FileStack.top().File;
    return (int)fread(buffer, 1, bufferSize, file);
    }
  return 0;
}

//----------------------------------------------------------------------------
void cmDependsFortranParser_StringStart(cmDependsFortranParser* parser)
{
  parser->TokenString = "";
}

//----------------------------------------------------------------------------
const char* cmDependsFortranParser_StringEnd(cmDependsFortranParser* parser)
{
  return parser->TokenString.c_str();
}

//----------------------------------------------------------------------------
void cmDependsFortranParser_StringAppend(cmDependsFortranParser* parser,
                                         char c)
{
  parser->TokenString += c;
}

//----------------------------------------------------------------------------
void cmDependsFortranParser_SetInInterface(cmDependsFortranParser* parser,
                                           int in)
{
  parser->InInterface = in;
}

//----------------------------------------------------------------------------
int cmDependsFortranParser_GetInInterface(cmDependsFortranParser* parser)
{
  return parser->InInterface;
}

//----------------------------------------------------------------------------
void cmDependsFortranParser_Error(cmDependsFortranParser*, const char*)
{
  // If there is a parser error just ignore it.  The source will not
  // compile and the user will edit it.  Then dependencies will have
  // to be regenerated anyway.
}

//----------------------------------------------------------------------------
void cmDependsFortranParser_RuleUse(cmDependsFortranParser* parser,
                                    const char* name)
{
  parser->Requires.insert(name);
}

//----------------------------------------------------------------------------
void cmDependsFortranParser_RuleInclude(cmDependsFortranParser* parser,
                                        const char* name)
{
  // If processing an include statement there must be an open file.
  assert(!parser->FileStack.empty());

  // Get the directory containing the source in which the include
  // statement appears.  This is always the first search location for
  // Fortran include files.
  std::string dir = parser->FileStack.top().Directory;

  // Find the included file.  If it cannot be found just ignore the
  // problem because either the source will not compile or the user
  // does not care about depending on this included source.
  std::string fullName;
  if(parser->Self->FindIncludeFile(dir.c_str(), name, fullName))
    {
    // Found the included file.  Save it in the set of included files.
    parser->Includes.insert(fullName);

    // Parse it immediately to translate the source inline.
    cmDependsFortranParser_FilePush(parser, fullName.c_str());
    }
}

//----------------------------------------------------------------------------
void cmDependsFortranParser_RuleModule(cmDependsFortranParser* parser,
                                       const char* name)
{
  parser->Provides.insert(name);
}

//----------------------------------------------------------------------------
void cmDependsFortranParser_RuleDefine(cmDependsFortranParser*, const char*)
{
}

//----------------------------------------------------------------------------
void cmDependsFortranParser_RuleUndef(cmDependsFortranParser*, const char*)
{
}

//----------------------------------------------------------------------------
void cmDependsFortranParser_RuleIfdef(cmDependsFortranParser*, const char*)
{
}

//----------------------------------------------------------------------------
void cmDependsFortranParser_RuleIfndef(cmDependsFortranParser*, const char*)
{
}

//----------------------------------------------------------------------------
void cmDependsFortranParser_RuleIf(cmDependsFortranParser*)
{
}

//----------------------------------------------------------------------------
void cmDependsFortranParser_RuleElif(cmDependsFortranParser*)
{
}

//----------------------------------------------------------------------------
void cmDependsFortranParser_RuleElse(cmDependsFortranParser*)
{
}

//----------------------------------------------------------------------------
void cmDependsFortranParser_RuleEndif(cmDependsFortranParser*)
{
}
