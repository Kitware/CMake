/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2013 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef cmCPackWIXPatchParser_h
#define cmCPackWIXPatchParser_h

#include <cmXMLParser.h>

#include <CPack/cmCPackLog.h>

#include <map>
#include <list>

struct cmWIXPatchNode
{
  enum Type
  {
    TEXT,
    ELEMENT
  };

  virtual ~cmWIXPatchNode();

  virtual Type type() = 0;
};

struct cmWIXPatchText : public cmWIXPatchNode
{
  virtual Type type();

  std::string text;
};

struct cmWIXPatchElement : cmWIXPatchNode
{
  virtual Type type();

  ~cmWIXPatchElement();

  typedef std::list<cmWIXPatchNode*> child_list_t;
  typedef std::map<std::string, std::string> attributes_t;

  std::string name;
  child_list_t children;
  attributes_t attributes;
};

/** \class cmWIXPatchParser
 * \brief Helper class that parses XML patch files (CPACK_WIX_PATCH_FILE)
 */
class cmWIXPatchParser : public cmXMLParser
{
public:
  typedef std::map<std::string, cmWIXPatchElement> fragment_map_t;

  cmWIXPatchParser(fragment_map_t& Fragments, cmCPackLog* logger);

private:
  virtual void StartElement(const std::string& name, const char **atts);

  void StartFragment(const char **attributes);

  virtual void EndElement(const std::string& name);

  virtual void CharacterDataHandler(const char* data, int length);

  virtual void ReportError(int line, int column, const char* msg);

  void ReportValidationError(std::string const& message);

  bool IsValid() const;

  cmCPackLog* Logger;

  enum ParserState
  {
    BEGIN_DOCUMENT,
    BEGIN_FRAGMENTS,
    INSIDE_FRAGMENT
  };

  ParserState State;

  bool Valid;

  fragment_map_t& Fragments;

  std::list<cmWIXPatchElement*> ElementStack;
};

#endif
