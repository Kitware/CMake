/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) 2001 Insight Consortium
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * The name of the Insight Consortium, nor the names of any consortium members,
   nor of any contributors, may be used to endorse or promote products derived
   from this software without specific prior written permission.

  * Modified source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#ifndef cmCableClassSet_h
#define cmCableClassSet_h

#include "cmStandardIncludes.h"
#include "cmData.h"
#include "cmMakefile.h"


/** \class cmCableClass
 * \brief Holds one class and the set of header files needed to use it.
 */
class cmCableClass
{
public:
  typedef std::set<cmStdString> Sources;
  cmCableClass() {}
  cmCableClass(const cmStdString& tag): m_Tag(tag) {}
  
  void AddSources(const Sources& sources);
  void AddSource(const char*);

  Sources::const_iterator SourcesBegin() const { return m_Sources.begin(); }
  Sources::const_iterator SourcesEnd() const { return m_Sources.end(); }

  const cmStdString& GetTag() const { return m_Tag; }
  
private:  
  /**
   * The tag name of this class.
   */
  cmStdString m_Tag;
  
  /**
   * Store the set of source files (headers) needed to define this class.
   */
  Sources m_Sources;
};


/** \class cmCableClassSet
 * \brief Holds a set of classes, each with their own set of required headers.
 */
class cmCableClassSet: public cmData
{
public:
  cmCableClassSet(const char* name): cmData(name) {}
  virtual ~cmCableClassSet();

  /**
   * The set is stored internally as a map from class name to cmCableClass
   * instance.
   */
  typedef std::map<cmStdString, cmCableClass*> CableClassMap;
  
  void AddClass(const char*, cmCableClass*);
  void AddSource(const char* name);
  unsigned int Size() const;
  CableClassMap::const_iterator Begin() const;
  CableClassMap::const_iterator End() const;

  void ParseAndAddElement(const char*, cmMakefile*);
  
private:
  /**
   * The set is stored internally as a map from class name to cmCableClass
   * instance.
   */
  CableClassMap m_CableClassMap;
};

#endif
