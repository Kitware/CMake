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

cmCommandArgumentParserHelper::cmCommandArgumentParserHelper()
{
  m_FileLine = -1;
  m_FileName = 0;
  m_EmptyVariable[0] = 0;
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
  return *(m_Variables.insert(stVal).first);
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
  if (m_EscapeQuotes)
    {
    return this->AddString(cmSystemTools::EscapeQuotes(value).c_str());
    }
  return this->AddString(value);
}

void cmCommandArgumentParserHelper::DeallocateParserType(char** pt)
{
  if (!pt)
    {
    return;
    }
  if (!*pt)
    {
    //*pt = 0;
    return;
    }
  // std::cout << (void*) *pt << " " << *pt << " this->DeallocateParserType" << std::endl;
  //delete [] *pt;
  *pt = 0;
  this->UnionsAvailable --;
}

void cmCommandArgumentParserHelper::SafePrintMissing(const char* str, int line, int cnt)
{
  if ( this->Verbose )
    {
    if ( str )
      {
      //std::cout << (void*) str << " JPSafePrintMissing" << std::endl;
      std::cout << line << " String " << cnt << " exists: ";
      unsigned int cc;
      for ( cc = 0; cc < strlen(str); cc ++ )
        {
        unsigned char ch = str[cc];
        if ( ch >= 32 && ch <= 126 )
          {
          std::cout << (char)ch;
          }
        else
          {
          std::cout << "<" << (int)ch << ">";
          break;
          }
        }
      std::cout << "- " << strlen(str) << std::endl;
      }
    }
}
void cmCommandArgumentParserHelper::Print(const char* place, const char* str)
{
  if ( this->Verbose )
    {
    std::cout << "[" << place << "=" << str << "]" << std::endl;
    }
}

char* cmCommandArgumentParserHelper::CombineUnions(const char* in1, const char* in2)
{
  int len = 1;
  if ( in1 )
    {
    len += strlen(in1);
    }
  if ( in2 )
    {
    len += strlen(in2);
    }
  char* out = new char [ len ];
  out[0] = 0;
  if ( in1 )
    {
    strcat(out, in1);
    }
  if ( in2 )
    {
    strcat(out, in2);
    }
  return *(m_Variables.insert(out).first);
}

void cmCommandArgumentParserHelper::CheckEmpty(int line, int cnt, cmCommandArgumentParserHelper::ParserType* pt)
{
  int cc;
  int kk = -cnt + 1;
  for ( cc = 1; cc <= cnt; cc ++)
    {
    cmCommandArgumentParserHelper::ParserType* cpt = pt + kk;
    this->SafePrintMissing(cpt->str, line, cc);
    kk ++;
    }
}

void cmCommandArgumentParserHelper::PrepareElement(cmCommandArgumentParserHelper::ParserType* me)
{
  // Inititalize self
  me->str = 0;
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
  this->UnionsAvailable ++;
  pt->str = new char[ len + 1 ];
  strncpy(pt->str, str, len);
  pt->str[len] = 0;
  this->Allocates.push_back(pt->str);
  // std::cout << (void*) pt->str << " " << pt->str << " JPAllocateParserType" << std::endl;
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
    std::cerr << "CAL_Parser returned: " << res << std::endl;
    std::cerr << "When parsing: [" << str << "]" << std::endl;
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
  std::vector<char*>::iterator it;
  for ( it = this->Allocates.begin(); 
    it != this->Allocates.end();
    ++ it )
    {
    delete [] *it;
    }
  std::set<char*>::iterator sit;
  for ( sit = m_Variables.begin();
    sit != m_Variables.end();
    ++ sit )
    {
    delete [] *sit;
    }
  this->Allocates.erase(this->Allocates.begin(), 
    this->Allocates.end());
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
  fprintf(stderr, "Argument Parser Error: %s (%lu / Line: %d)\n", str, pos, this->CurrentLine);
  int cc;
  std::cerr << "String: [";
  for ( cc = 0; cc < 30 && *(this->InputBuffer.c_str() + this->InputBufferPos + cc);
    cc ++ )
    {
    std::cerr << *(this->InputBuffer.c_str() + this->InputBufferPos + cc);
    }
  std::cerr << "]" << std::endl;
}

void cmCommandArgumentParserHelper::UpdateCombine(const char* str1, const char* str2)
{
  if ( this->CurrentCombine == "" && str1 != 0)
    {
    this->CurrentCombine = str1;
    }
  this->CurrentCombine += ".";
  this->CurrentCombine += str2;
}

int cmCommandArgumentParserHelper::ParseFile(const char* file)
{
  if ( !cmSystemTools::FileExists(file))
    {
    return 0;
    }
  std::ifstream ifs(file);
  if ( !ifs )
    {
    return 0;
    }

  cmStdString fullfile = "";
  cmStdString line;
  while ( cmSystemTools::GetLineFromStream(ifs, line) )
    {
    fullfile += line + "\n";
    }
  return this->ParseString(fullfile.c_str(), 0);
}

void cmCommandArgumentParserHelper::Append(const char* str)
{
  std::cout << "Append[" << str << "]" << std::endl;
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

