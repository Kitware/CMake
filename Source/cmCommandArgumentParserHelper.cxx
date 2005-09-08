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
#include "cmCommandArgumentParserHelper.h"

#include "cmSystemTools.h"
#include "cmCommandArgumentLexer.h"

#include "cmMakefile.h"

int cmCommandArgument_yyparse( yyscan_t yyscanner );
//
cmCommandArgumentParserHelper::cmCommandArgumentParserHelper()
{
  m_FileLine = -1;
  m_FileName = 0;

  m_EmptyVariable[0] = 0;
  strcpy(m_DCURLYVariable, "${");
  strcpy(m_RCURLYVariable, "}");
  strcpy(m_ATVariable,     "@");
  strcpy(m_DOLLARVariable, "$");
  strcpy(m_LCURLYVariable, "{");
  strcpy(m_BSLASHVariable, "\\");

  m_NoEscapeMode = false;
}


cmCommandArgumentParserHelper::~cmCommandArgumentParserHelper()
{
  this->CleanupParser();
}

void cmCommandArgumentParserHelper::SetLineFile(long line, const char* file)
{
  m_FileLine = line;
  m_FileName = file;
}

char* cmCommandArgumentParserHelper::AddString(const char* str)
{
  if ( !str || !*str )
    {
    return m_EmptyVariable;
    }
  char* stVal = new char[strlen(str)+1];
  strcpy(stVal, str);
  m_Variables.push_back(stVal);
  return stVal;
}

char* cmCommandArgumentParserHelper::ExpandSpecialVariable(const char* key, const char* var)
{
  if ( !key )
    {
    return this->ExpandVariable(var);
    }
  if ( strcmp(key, "ENV") == 0 )
    {
    char *ptr = getenv(var);
    if (ptr)
      {
      if (m_EscapeQuotes)
        {
        return this->AddString(cmSystemTools::EscapeQuotes(ptr).c_str());
        }
      else
        {
        return ptr;
        }
      }
    return m_EmptyVariable;
    }
  cmSystemTools::Error("Key ", key, " is not used yet. For now only $ENV{..} is allowed");
  return 0;
}

char* cmCommandArgumentParserHelper::ExpandVariable(const char* var)
{
  if(m_FileName && strcmp(var, "CMAKE_CURRENT_LIST_FILE") == 0)
    {
    return this->AddString(m_FileName);
    }
  else if(m_FileLine >= 0 && strcmp(var, "CMAKE_CURRENT_LIST_LINE") == 0)
    {
    cmOStringStream ostr;
    ostr << m_FileLine;
    return this->AddString(ostr.str().c_str());
    } 
  const char* value = m_Makefile->GetDefinition(var);
  if (m_EscapeQuotes && value)
    {
    return this->AddString(cmSystemTools::EscapeQuotes(value).c_str());
    }
  return this->AddString(value);
}

char* cmCommandArgumentParserHelper::CombineUnions(char* in1, char* in2)
{
  if ( !in1 )
    {
    return in2;
    }
  else if ( !in2 )
    {
    return in1;
    }
  int len = strlen(in1) + strlen(in2) + 1;
  char* out = new char [ len ];
  strcpy(out, in1);
  strcat(out, in2);
  m_Variables.push_back(out);
  return out;
}

void cmCommandArgumentParserHelper::AllocateParserType(cmCommandArgumentParserHelper::ParserType* pt, 
  const char* str, int len)
{
  pt->str = 0;
  if ( len == 0 )
    {
    len = strlen(str);
    }
  if ( len == 0 )
    {
    return;
    }
  pt->str = new char[ len + 1 ];
  strncpy(pt->str, str, len);
  pt->str[len] = 0;
  m_Variables.push_back(pt->str);
  // std::cout << (void*) pt->str << " " << pt->str << " JPAllocateParserType" << std::endl;
}

bool cmCommandArgumentParserHelper::HandleEscapeSymbol(cmCommandArgumentParserHelper::ParserType* pt, char symbol)
{
  if ( m_NoEscapeMode )
    {
    char buffer[3];
    buffer[0] = '\\';
    buffer[1] = symbol;
    buffer[2] = 0;
    this->AllocateParserType(pt, buffer, 2);
    return true;
    }
  switch ( symbol )
    {
  case '\\':
  case '"':
  case ' ':
  case '#':
  case '(':
  case ')':
  case '$':
  case '^':
    this->AllocateParserType(pt, &symbol, 1);
    break;
  case ';':
    this->AllocateParserType(pt, "\\;", 2);
    break;
  case 't':
    this->AllocateParserType(pt, "\t", 1);
    break;
  case 'n':
    this->AllocateParserType(pt, "\n", 1);
    break;
  case 'r':
    this->AllocateParserType(pt, "\r", 1);
    break;
  case '0':
    this->AllocateParserType(pt, "\0", 1);
    break;
  default:
    char buffer[2];
    buffer[0] = symbol;
    buffer[1] = 0;
    cmSystemTools::Error("Invalid escape sequence \\", buffer);
    return false;
    }
  return true;
}

int cmCommandArgumentParserHelper::ParseString(const char* str, int verb)
{
  if ( !str)
    {
    return 0;
    }
  //printf("Do some parsing: %s\n", str);

  this->Verbose = verb;
  this->InputBuffer = str;
  this->InputBufferPos = 0;
  this->CurrentLine = 0;
  
  m_Result = "";

  yyscan_t yyscanner;
  cmCommandArgument_yylex_init(&yyscanner);
  cmCommandArgument_yyset_extra(this, yyscanner);
  int res = cmCommandArgument_yyparse(yyscanner);
  cmCommandArgument_yylex_destroy(yyscanner);
  if ( res != 0 )
    {
    //str << "CAL_Parser returned: " << res << std::endl;
    //std::cerr << "When parsing: [" << str << "]" << std::endl;
    return 0;
    }

  this->CleanupParser();

  if ( Verbose )
    {
    std::cerr << "Expanding [" << str << "] produced: [" << m_Result.c_str() << "]" << std::endl;
    }
  return 1;
}

void cmCommandArgumentParserHelper::CleanupParser()
{
  std::vector<char*>::iterator sit;
  for ( sit = m_Variables.begin();
    sit != m_Variables.end();
    ++ sit )
    {
    delete [] *sit;
    }
  m_Variables.erase(m_Variables.begin(), m_Variables.end());
}

int cmCommandArgumentParserHelper::LexInput(char* buf, int maxlen)
{
  //std::cout << "JPLexInput ";
  //std::cout.write(buf, maxlen);
  //std::cout << std::endl;
  if ( maxlen < 1 )
    {
    return 0;
    }
  if ( this->InputBufferPos < this->InputBuffer.size() )
    {
    buf[0] = this->InputBuffer[ this->InputBufferPos++ ];
    if ( buf[0] == '\n' )
      {
      this->CurrentLine ++;
      }
    return(1);
    }
  else
    {
    buf[0] = '\n';
    return( 0 );
    }
}

void cmCommandArgumentParserHelper::Error(const char* str)
{
  unsigned long pos = static_cast<unsigned long>(this->InputBufferPos);
  //fprintf(stderr, "Argument Parser Error: %s (%lu / Line: %d)\n", str, pos, this->CurrentLine);
  cmOStringStream ostr;
  ostr << str << " (" << pos << ")";
  /*
  int cc;
  std::cerr << "String: [";
  for ( cc = 0; cc < 30 && *(this->InputBuffer.c_str() + this->InputBufferPos + cc);
    cc ++ )
    {
    std::cerr << *(this->InputBuffer.c_str() + this->InputBufferPos + cc);
    }
  std::cerr << "]" << std::endl;
  */
  m_Error = ostr.str();
}

void cmCommandArgumentParserHelper::SetMakefile(const cmMakefile* mf)
{
  m_Makefile = mf;
}

void cmCommandArgumentParserHelper::SetResult(const char* value)
{
  if ( !value )
    {
    m_Result = "";
    return;
    }
  m_Result = value;
}

