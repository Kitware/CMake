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
#include "cmXMLParser.h"

#include <cmexpat/expat.h>
#include <ctype.h>

//----------------------------------------------------------------------------
cmXMLParser::cmXMLParser()
{
  this->Parser = 0;
  this->ParseError = 0;
}

//----------------------------------------------------------------------------
cmXMLParser::~cmXMLParser()
{
  if ( this->Parser )
    {
    this->CleanupParser();
    }
}

//----------------------------------------------------------------------------
int cmXMLParser::Parse(const char* string)
{
  return this->InitializeParser() &&
    this->ParseChunk(string, strlen(string)) && 
    this->CleanupParser();
}

int cmXMLParser::ParseFile(const char* file)
{
  if ( !file )
    {
    return 0;
    }

  std::ifstream ifs(file);
  if ( !ifs )
    {
    return 0;
    }

  cmOStringStream str;
  str << ifs.rdbuf();
  return this->Parse(str.str().c_str());
}

//----------------------------------------------------------------------------
int cmXMLParser::InitializeParser()
{
  if ( this->Parser )
    {
    std::cerr << "Parser already initialized" << std::endl;
    this->ParseError = 1;
    return 0;
    }

  // Create the expat XML parser.
  this->Parser = XML_ParserCreate(0);
  XML_SetElementHandler(static_cast<XML_Parser>(this->Parser),
                        &cmXMLParserStartElement,
                        &cmXMLParserEndElement);
  XML_SetCharacterDataHandler(static_cast<XML_Parser>(this->Parser),
                              &cmXMLParserCharacterDataHandler);
  XML_SetUserData(static_cast<XML_Parser>(this->Parser), this);
  this->ParseError = 0;
  return 1;
}

//----------------------------------------------------------------------------
int cmXMLParser::ParseChunk(const char* inputString, unsigned int length)
{
  if ( !this->Parser )
    {
    std::cerr << "Parser not initialized" << std::endl;
    this->ParseError = 1;
    return 0;
    }
  int res;
  res = this->ParseBuffer(inputString, length);
  if ( res == 0 )
    {
    this->ParseError = 1;
    }
  return res;
}

//----------------------------------------------------------------------------
int cmXMLParser::CleanupParser()
{
  if ( !this->Parser )
    {
    std::cerr << "Parser not initialized" << std::endl;
    this->ParseError = 1;
    return 0;
    }
  int result = !this->ParseError;
  if(result)
    {
    // Tell the expat XML parser about the end-of-input.
    if(!XML_Parse(static_cast<XML_Parser>(this->Parser), "", 0, 1))
      {
      this->ReportXmlParseError();
      result = 0;
      }
    }
  
  // Clean up the parser.
  XML_ParserFree(static_cast<XML_Parser>(this->Parser));
  this->Parser = 0;
  
  return result;
}

//----------------------------------------------------------------------------
int cmXMLParser::ParseBuffer(const char* buffer, unsigned int count)
{
  // Pass the buffer to the expat XML parser.
  if(!XML_Parse(static_cast<XML_Parser>(this->Parser), buffer, count, 0))
    {
    this->ReportXmlParseError();
    return 0;
    }
  return 1;
}

//----------------------------------------------------------------------------
int cmXMLParser::ParseBuffer(const char* buffer)
{
  return this->ParseBuffer(buffer, static_cast<int>(strlen(buffer)));
}

//----------------------------------------------------------------------------
int cmXMLParser::ParsingComplete()
{
  // Default behavior is to parse to end of stream.
  return 0;
}

//----------------------------------------------------------------------------
void cmXMLParser::StartElement(const char * name,
  const char ** /*atts*/)
{
  std::cout << "Start element: " << name << std::endl;
}

//----------------------------------------------------------------------------
void cmXMLParser::EndElement(const char * name)
{
  std::cout << "End element: " << name << std::endl;
}

//----------------------------------------------------------------------------
void cmXMLParser::CharacterDataHandler(const char* /*inData*/,
  int /*inLength*/)
{
}

//----------------------------------------------------------------------------
int cmXMLParser::IsSpace(char c)
{
  return isspace(c);
}

//----------------------------------------------------------------------------
void cmXMLParserStartElement(void* parser, const char *name,
                              const char **atts)
{
  // Begin element handler that is registered with the XML_Parser.
  // This just casts the user data to a cmXMLParser and calls
  // StartElement.
  static_cast<cmXMLParser*>(parser)->StartElement(name, atts);
}

//----------------------------------------------------------------------------
void cmXMLParserEndElement(void* parser, const char *name)
{
  // End element handler that is registered with the XML_Parser.  This
  // just casts the user data to a cmXMLParser and calls EndElement.
  static_cast<cmXMLParser*>(parser)->EndElement(name);
}

//----------------------------------------------------------------------------
void cmXMLParserCharacterDataHandler(void* parser, const char* data,
                                      int length)
{
  // Character data handler that is registered with the XML_Parser.
  // This just casts the user data to a cmXMLParser and calls
  // CharacterDataHandler.
  static_cast<cmXMLParser*>(parser)->CharacterDataHandler(data, length);
}

//----------------------------------------------------------------------------
void cmXMLParser::ReportXmlParseError()
{
  std::cerr << "Error parsing XML in stream at line "
    << XML_GetCurrentLineNumber(static_cast<XML_Parser>(this->Parser))
    << ": " 
    << XML_ErrorString(XML_GetErrorCode(
        static_cast<XML_Parser>(this->Parser))) << std::endl;
}

