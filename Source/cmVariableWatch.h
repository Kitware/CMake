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
#ifndef cmVariableWatch_h
#define cmVariableWatch_h

#include "cmStandardIncludes.h"

/** \class cmVariableWatch
 * \brief Helper class for watching of variable accesses.
 *
 * Calls function when variable is accessed
 */
class cmVariableWatch
{
public:
  typedef void (*WatchMethod)(const std::string& variable, int access_type, void* client_data);

  cmVariableWatch();
  ~cmVariableWatch();

  /**
   * Add watch to the variable
   */
  void AddWatch(const std::string& variable, WatchMethod method, void* client_data=0);
  void RemoveWatch(const std::string& variable, WatchMethod method);
  
  /**
   * This method is called when variable is accessed
   */
  void VariableAccessed(const std::string& variable, int access_type) const;

  /**
   * Different access types.
   */
  enum
    {
      VARIABLE_READ_ACCESS,
      UNKNOWN_VARIABLE_READ_ACCESS,
      ALLOWED_UNKNOWN_VARIABLE_READ_ACCESS,
      VARIABLE_MODIFIED_ACCESS,
      VARIABLE_REMOVED_ACCESS,
      NO_ACCESS
    };
  
protected:
  struct Pair
  {
    WatchMethod m_Method;
    void*        m_ClientData;
    Pair() : m_Method(0), m_ClientData(0) {}
  };

  typedef std::vector< Pair > VectorOfPairs;
  typedef std::map<cmStdString, VectorOfPairs > StringToVectorOfPairs;

  StringToVectorOfPairs m_WatchMap;
};


#endif
