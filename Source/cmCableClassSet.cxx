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
#include "cmCableClassSet.h"


/**
 * Add to the set of required sources to define the class.
 */
void cmCableClass::AddSource(const char* source)
{
  m_Sources.insert(source);
}


/**
 * Add a class to the set.
 */
void cmCableClassSet::AddClass(const char* name,
                               const cmCableClass& cableClass)
{
  m_CableClassMap.insert(CableClassMap::value_type(name, cableClass));
}


/**
 * Add a source to every class in the set.  This should only be done after
 * all classes have been inserted.
 */
void cmCableClassSet::AddSource(const char* name)
{
  for(CableClassMap::iterator c = m_CableClassMap.begin();
      c != m_CableClassMap.end(); ++c)
    {
    c->second.AddSource(name);
    }
}

/**
 * Get the size of the internal CableClassMap used to store the set.
 */
unsigned int cmCableClassSet::Size() const
{
  return m_CableClassMap.size();
}


/**
 * Get a begin iterator to the internal CableClassMap used to store the
 * set.
 */
cmCableClassSet::CableClassMap::const_iterator cmCableClassSet::Begin() const
{
  return m_CableClassMap.begin();
}


/**
 * Get an end iterator to the internal CableClassMap used to store the
 * set.
 */
cmCableClassSet::CableClassMap::const_iterator cmCableClassSet::End() const
{
  return m_CableClassMap.end();
}

/**
 * Parse the given string to extract the class information specified.
 *
 * The format of the string is
 *   [tag:]class_name[;source1;source2;...]
 *
 */
void cmCableClassSet::ParseAndAddElement(const char* element,
                                         cmMakefile* makefile)
{
  // A regular expression to match the tagged element specification.
  cmRegularExpression tagGiven("^([A-Za-z_0-9]*)[ \t]*:[ \t]*([^:].*|::.*)$");

  // A regular expression to match the element when more source files are given.
  cmRegularExpression sourcesRemain("^([^;]*);(.*)$");

  std::string tag;
  std::string elementWithoutTag;
  std::string className;
  std::string sourceString;

  if(tagGiven.find(element))
    {
    // A tag was given.  Use it.
    tag = tagGiven.match(1);
    elementWithoutTag = tagGiven.match(2);
    }
  else
    {
    // No tag was given.  Try to generate one.
    //if(!this->GenerateTag(element, tag))
    //  { return false; }
    elementWithoutTag = element;
    }
  
  if(sourcesRemain.find(elementWithoutTag.c_str()))
    {
    className = sourcesRemain.match(1);
    sourceString = sourcesRemain.match(2);
    }
  else
    {
    className = elementWithoutTag;
    }
  
  cmCableClass::Sources sources;
  
  while(sourcesRemain.find(sourceString.c_str()))
    {
    sources.insert(sourcesRemain.match(1));
    sourceString = sourcesRemain.match(2);
    }
  if(sourceString != "")
    {
    sources.insert(sourceString);
    }
  
  // A regular expression to match a class name that is just a set.
  cmRegularExpression setDereference("^\\$(.*)$");
  if(setDereference.find(className))
    {
    std::string setName = setDereference.match(1);
    cmData* d = makefile->LookupData(setName.c_str());
    cmCableClassSet* classSet = dynamic_cast<cmCableClassSet*>(d);
    if(classSet)
      {
      this->AddCableClassSet(*classSet, sources);
      }
    else
      {
      cmSystemTools::Error("Unknown CABLE class set ", setName.c_str());
      }
    }
  else
    {
    cmCableClass cableClass;
    cableClass.AddSources(sources.begin(), sources.end());
    this->AddClass(className.c_str(), cableClass);
    }
}


/**
 * Add all elements from the given cmCableClassSet to this set, with the given
 * sources added to each element.
 */
void cmCableClassSet::AddCableClassSet(const cmCableClassSet& set,
                                       const cmCableClass::Sources& sources)
{
  for(CableClassMap::const_iterator c = set.Begin(); c != set.End(); ++c)
    {
    cmCableClass cableClass = c->second;
    cableClass.AddSources(sources.begin(), sources.end());
    this->AddClass(c->first.c_str(), cableClass);
    }
}
