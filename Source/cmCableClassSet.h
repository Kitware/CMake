/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

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
