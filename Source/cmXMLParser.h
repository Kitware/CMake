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
#ifndef cmXMLParser_h
#define cmXMLParser_h

#include "cmStandardIncludes.h"

extern "C"
{
  void cmXMLParserStartElement(void*, const char*, const char**);
  void cmXMLParserEndElement(void*, const char*);
  void cmXMLParserCharacterDataHandler(void*, const char*, int);  
}

/** \class cmXMLParser
 * \brief Helper class for performing XML parsing
 *
 * Superclass for all XML parsers.
 */
class cmXMLParser
{
public:
  cmXMLParser();
  virtual ~cmXMLParser();

  //! Parse given XML string
  virtual int Parse(const char* string);

  //! Parse given XML file
  virtual int ParseFile(const char* file);
  
  /**
   * When parsing fragments of XML or streaming XML, use the following three
   * methods.  InitializeParser method initialize parser but does not perform
   * any actual parsing.  ParseChunk parses framgent of XML. This has to match
   * to what was already parsed. CleanupParser finishes parsing. If there were
   * errors, CleanupParser will report them.
   */
  virtual int InitializeParser();
  virtual int ParseChunk(const char* inputString, unsigned int length);
  virtual int CleanupParser();

protected:
  //! This variable is true if there was a parse error while parsing in chunks.
  int ParseError;

  //1 Expat parser structure.  Exists only during call to Parse().
  void* Parser;

  /**
   * Called before each block of input is read from the stream to check if
   * parsing is complete.  Can be replaced by subclasses to change the
   * terminating condition for parsing.  Parsing always stops when the end of
   * file is reached in the stream.
   */
  
  virtual int ParsingComplete();

  /**
   * Called when a new element is opened in the XML source.  Should be replaced
   * by subclasses to handle each element.
   *   name = Name of new element.
   *   atts = Null-terminated array of attribute name/value pairs.  Even
   *          indices are attribute names, and odd indices are values.
   */
  virtual void StartElement(const char* name, const char** atts);
  
  //! Called at the end of an element in the XML source opened when StartElement
  // was called.
  virtual void EndElement(const char* name);
  
  //! Called when there is character data to handle.
  virtual void CharacterDataHandler(const char* data, int length);  

  //! Called by Parse to report an XML syntax error.
  virtual void ReportXmlParseError();  

  //! Utility for convenience of subclasses.  Wraps isspace C library
  // routine.
  static int IsSpace(char c);  
  
  //! Send the given buffer to the XML parser.
  virtual int ParseBuffer(const char* buffer, unsigned int count);
  
  //! Send the given c-style string to the XML parser.
  int ParseBuffer(const char* buffer);

  //! Callbacks for the expat
  friend void cmXMLParserStartElement(void*, const char*, const char**);
  friend void cmXMLParserEndElement(void*, const char*);
  friend void cmXMLParserCharacterDataHandler(void*, const char*, int);
};

#endif
