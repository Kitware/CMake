/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmData_h
#define cmData_h

#include "cmStandardIncludes.h"

/** \class cmData
 * \brief Hold extra data on a cmMakefile instance for a command.
 *
 * When CMake commands need to store extra information in a cmMakefile
 * instance, but the information is not needed by the makefile generators,
 * it can be held in a subclass of cmData.  The cmMakefile class has a map
 * from std::string to cmData*.  On its destruction, it destroys all the
 * extra data through the virtual destructor of cmData.
 */
class cmData
{
public:
  cmData(const char* name): Name(name) {}
  virtual ~cmData() {}
  
  const std::string& GetName() const
    { return this->Name; }
protected:
  std::string Name;
};

#endif
