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
#ifndef cmListCommand_h
#define cmListCommand_h

#include "cmCommand.h"

/** \class cmListCommand
 * \brief Common list operations
 *
 */
class cmListCommand : public cmCommand
{
public:
  /**
   * This is a virtual constructor for the command.
   */
  virtual cmCommand* Clone()
    {
    return new cmListCommand;
    }

  /**
   * This is called when the command is first encountered in
   * the CMakeLists.txt file.
   */
  virtual bool InitialPass(std::vector<std::string> const& args);

  /**
   * This determines if the command is invoked when in script mode.
   */
  virtual bool IsScriptable() { return true; }

  /**
   * The name of the command as specified in CMakeList.txt.
   */
  virtual const char* GetName() { return "list";}

  /**
   * Succinct documentation.
   */
  virtual const char* GetTerseDocumentation()
    {
    return "List operations.";
    }

  /**
   * More documentation.
   */
  virtual const char* GetFullDocumentation()
    {
    return
      "  list(LENGTH <list> <output variable>)\n"
      "  list(GET <list> <element index> [<element index> ...] "
      "<output variable>)\n"
      "  list(APPEND <list> <element> [<element> ...])\n"
      "  list(FIND <list> <value> <output variable>)\n"
      "  list(INSERT <list> <element_index> <element> [<element> ...])\n"
      "  list(REMOVE_ITEM <list> <value> [<value> ...])\n"
      "  list(REMOVE_AT <list> <index> [<index> ...])\n"
      "  list(REVERSE <list>)\n"
      "  list(SORT <list>)\n"
      "LENGTH will return a given list's length.\n"
      "GET will return list of elements specified by indices from the list.\n"
      "APPEND will append elements to the list.\n"
      "FIND will return the index of the element specified in the list or -1 "
      "if it wasn't found.\n"
      "INSERT will insert elements to the list to the specified location.\n"
      "REMOVE_AT and REMOVE_ITEM will remove items from the list. The "
      "difference is that REMOVE_ITEM will remove the given items, while "
      "REMOVE_AT will remove the items at the given indices.\n"
      "REVERSE reverses the contents of the list in-place.\n"
      "SORT sorts the list in-place alphabetically.\n"
      "NOTES: A list in cmake is a ; separated group of strings. "
      "To create a list the set command can be used. For example, "
      "set(var a b c d e)  creates a list with a;b;c;d;e, and "
      "set(var \"a b c d e\") creates a string or a list with one "
      "item in it.\n"
      "When specifying index values, if <element index> is 0 or"
      " greater, it is indexed from the "
      "beginning of the list, with 0 representing the first list element. "
      "If <element index> is -1 or lesser, it is indexed from the end of "
      "the list, with -1 representing the last list element. Be careful "
      "when counting with negative indices: they do not start from 0. "
      "-0 is equivalent to 0, the first list element.\n"
      ;
    }

  cmTypeMacro(cmListCommand, cmCommand);
protected:
  bool HandleLengthCommand(std::vector<std::string> const& args);
  bool HandleGetCommand(std::vector<std::string> const& args);
  bool HandleAppendCommand(std::vector<std::string> const& args);
  bool HandleFindCommand(std::vector<std::string> const& args);
  bool HandleInsertCommand(std::vector<std::string> const& args);
  bool HandleRemoveAtCommand(std::vector<std::string> const& args);
  bool HandleRemoveItemCommand(std::vector<std::string> const& args);
  bool HandleSortCommand(std::vector<std::string> const& args);
  bool HandleReverseCommand(std::vector<std::string> const& args);


  bool GetList(std::vector<std::string>& list, const char* var);
  bool GetListString(std::string& listString, const char* var);
};


#endif
