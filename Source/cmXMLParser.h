/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

extern "C" {
void cmXMLParserStartElement(void*, char const*, char const**);
void cmXMLParserEndElement(void*, char const*);
void cmXMLParserCharacterDataHandler(void*, char const*, int);
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
  cmXMLParser(cmXMLParser const& /*other*/) = default;
  virtual ~cmXMLParser();

  //! Parse given XML string
  virtual int Parse(char const* string);

  //! Parse given XML file
  virtual int ParseFile(char const* file);

  /**
   * When parsing fragments of XML or streaming XML, use the following
   * three methods.  InitializeParser method initialize parser but does
   * not perform any actual parsing.  ParseChunk parses fragment of
   * XML. This has to match to what was already parsed. CleanupParser
   * finishes parsing. If there were errors, CleanupParser will report
   * them.
   */
  virtual int InitializeParser();
  virtual int ParseChunk(char const* inputString,
                         std::string::size_type length);
  virtual int CleanupParser();
  using ReportFunction = void (*)(int, char const*, void*);
  void SetErrorCallback(ReportFunction f, void* d)
  {
    this->ReportCallback = f;
    this->ReportCallbackData = d;
  }

protected:
  //! This variable is true if there was a parse error while parsing in
  // chunks.
  int ParseError;
  ReportFunction ReportCallback;
  void* ReportCallbackData;

  // 1 Expat parser structure.  Exists only during call to Parse().
  void* Parser;

  /**
   * Called before each block of input is read from the stream to check if
   * parsing is complete.  Can be replaced by subclasses to change the
   * terminating condition for parsing.  Parsing always stops when the end of
   * file is reached in the stream.
   */

  virtual int ParsingComplete();

  /**
   * Called when a new element is opened in the XML source.  Should be
   * replaced by subclasses to handle each element.  name = Name of new
   * element.  atts = Null-terminated array of attribute name/value pairs.
   * Even indices are attribute names, and odd indices are values.
   */
  virtual void StartElement(std::string const& name, char const** atts);

  //! Called at the end of an element in the XML source opened when
  // StartElement was called.
  virtual void EndElement(std::string const& name);

  //! Called when there is character data to handle.
  virtual void CharacterDataHandler(char const* data, int length);

  //! Called by Parse to report an XML syntax error.
  virtual void ReportXmlParseError();

  /** Called by ReportXmlParseError with basic error info.  */
  virtual void ReportError(int line, int column, char const* msg);

  //! Send the given buffer to the XML parser.
  virtual int ParseBuffer(char const* buffer, std::string::size_type length);

  //! Send the given c-style string to the XML parser.
  int ParseBuffer(char const* buffer);

  /** Helps subclasses search for attributes on elements.  */
  static char const* FindAttribute(char const** atts, char const* attribute);

  //! Callbacks for the expat
  friend void cmXMLParserStartElement(void*, char const*, char const**);
  friend void cmXMLParserEndElement(void*, char const*);
  friend void cmXMLParserCharacterDataHandler(void*, char const*, int);
};
