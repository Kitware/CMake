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
#include "cmCableClassSet.h"


/**
 * Add to the set of required sources to define the class.
 */
void cmCableClass::AddSources(const Sources& sources)
{
  for(Sources::const_iterator s = sources.begin(); s != sources.end(); ++s)
    {
    m_Sources.insert(*s);
    }
}


/**
 * Add to the set of required sources to define the class.
 */
void cmCableClass::AddSource(const char* source)
{
  m_Sources.insert(source);
}


/**
 * The destructor frees all the cmCableClass instances in the set.
 */
cmCableClassSet::~cmCableClassSet()
{
  for(CableClassMap::const_iterator i = m_CableClassMap.begin();
      i != m_CableClassMap.end(); ++i)
    {
    delete i->second;
    }
}


/**
 * Add a class to the set.
 * Automatically replace ">>" with "> >" to prevent template class name
 * problems after replacements.
 */
void cmCableClassSet::AddClass(const char* in_name,
                               cmCableClass* cableClass)
{
  cmStdString name = in_name;
  for(cmStdString::size_type pos = name.find(">>");
      pos != cmStdString::npos; pos = name.find(">>", pos+2))
    {
    name.replace(pos, 2, "> >");
    }
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
    c->second->AddSource(name);
    }
}


/**
 * Get the size of the internal CableClassMap used to store the set.
 */
size_t cmCableClassSet::Size() const
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
 * A utility class to generate element combinations from all possible
 * substitutions of set members into a $ token.
 */
class ElementCombinationGenerator
{
public:
  ElementCombinationGenerator(const char* in_element, cmMakefile* in_makefile,
                              cmCableClassSet* out_set):
    m_Makefile(in_makefile), m_OutputSet(out_set)
    {
      this->ParseInputElement(in_element);
    }
  ~ElementCombinationGenerator();
  
  void Generate();
  
public:  
  /**
   * Represent a substitution.
   */
  class Substitution
  {
  public:
    Substitution() {}
    void Bind(const cmStdString& in_code, const cmCableClass* in_class)
      {
        m_Code = in_code;
        m_Class = in_class;
      }
    const cmCableClass* GetClass() const
      { return m_Class; }
    const cmStdString& GetCode() const
      { return m_Code; }    
    
  private:
    /**
     * The cmCableClass associated with this substitution.
     */
    const cmCableClass* m_Class;

    /**
     * The code to be used for the substitution.
     */
    cmStdString m_Code;
  };
  
  
  /**
   * Interface to the parts of an input string of code, possibly with
   * $SomeSetName tokens in it.  An indivitual Portion will be either
   * a StringPortion, which has no substitutions, or a ReplacePortion,
   * which has only a substitution, and no hard-coded text.
   *
   * This is used by cmCableClassSet::GenerateElementCombinations() to
   * hold the pieces of a string after the set substitution tokens
   * have been extracted.
   */
  class Portion
  {
  public:
    /**
     * Get the C++ code corresponding to this Portion of a string.
     */
    virtual cmStdString GetCode() const =0;
    /**
     * Get the class corresponding to this Portion of a string.  This is NULL
     * for StringPortion, and points to a cmCableClass for ReplacePortion.
     */
    virtual const cmCableClass* GetClass() const
      { return NULL; }
    virtual ~Portion() {}
  };
  
  
  /**
   * Represent a hard-coded part of an input string, that has no substitutions
   * in it.  The tag for this part of a string is always empty.
   */
  class StringPortion: public Portion
  {
  public:
    StringPortion(const cmStdString& in_code): m_Code(in_code) {}
    virtual cmStdString GetCode() const
      { return m_Code; }
    virtual const cmCableClass* GetClass() const
      { return NULL; }
    virtual ~StringPortion() {}
  private:
    /**
     * Hold this Portion's contribution to the output string.
     */
    cmStdString m_Code;
  };
  

  /**
   * Represent the "$SomeSetName" portion of an input string.  This has a
   * reference to the Substitution holding the real output to generate.
   */
  class ReplacePortion;
  friend class ReplacePortion;
  class ReplacePortion: public Portion
  {
  public:
    ReplacePortion(const Substitution& in_substitution):
      m_Substitution(in_substitution) {}
    virtual cmStdString GetCode() const
      { return m_Substitution.GetCode(); }
    virtual const cmCableClass* GetClass() const
      { return m_Substitution.GetClass(); }
    virtual ~ReplacePortion() {}
  private:
    /**
     * Refer to the real Substitution for this Portion's contribution.
     */
    const Substitution& m_Substitution;
  };
  
  typedef std::list<Portion*>  Portions;
  typedef std::map<const cmCableClassSet*, Substitution*>  Substitutions;
  
  /**
   * The makefile in which to lookup set names.
   */
  cmMakefile* m_Makefile;

  /**
   * The cmCableClassSet instance to be filled with combinations.
   */
  cmCableClassSet* m_OutputSet;
  
  /**
   * The class name parsed out for this element, before set expansion.
   */
  cmStdString m_ClassName;
  
  /**
   * The tag name parsed out or generated for this element.
   */
  cmStdString m_Tag;
  
  /**
   * The set of sources parsed out for this element.
   */
  cmCableClass::Sources m_Sources;
  
  /**
   * The parts of the input string after parsing of the tokens.
   */
  Portions m_Portions;
  
  /**
   * Map from substitution's Set to actual Substitution.
   */
  Substitutions m_Substitutions;  
  
private:
  void Generate(Substitutions::const_iterator);
  void ParseInputElement(const char*);
  void SplitClassName();
  cmStdString ParseSetName(cmStdString::const_iterator&,
                           cmStdString::const_iterator) const;
  void FindTagSource();
  bool GenerateTag(const cmStdString&);
};


/**
 * Destructor frees portions and substitutions that were allocated by
 * constructor.
 */
ElementCombinationGenerator
::~ElementCombinationGenerator()
{
  // Free the string portions that were allocated.
  for(Portions::iterator portion = m_Portions.begin();
      portion != m_Portions.end(); ++portion)
    {
    delete *portion;
    }
  
  // Free the substitutions that were allocated.
  for(Substitutions::iterator sub = m_Substitutions.begin();
      sub != m_Substitutions.end(); ++sub)
    {
    delete sub->second;
    }
}


/**
 * Generate all element combinations possible with the set of
 * substitutions available.  The given output set is filled with
 * all the combinations.
 */
void
ElementCombinationGenerator
::Generate()
{
  // If there are no substitutions to be made, just generate this
  // single combination.
  if(m_Substitutions.empty())
    {
    cmCableClass* cableClass = new cmCableClass(m_Tag);
    cableClass->AddSources(m_Sources);
    m_OutputSet->AddClass(m_ClassName.c_str(), cableClass);
    return;
    }
  
  // We must generate all combinations of substitutions.
  // Begin the recursion with the first substitution.
  this->Generate(m_Substitutions.begin());
}


/**
 * Internal helper to Generate() which generates all
 * combinations in a recursive, depth-first order.
 */
void
ElementCombinationGenerator
::Generate(Substitutions::const_iterator substitution)
{
  // Test our position in the list of substitutions to be bound.
  if(substitution == m_Substitutions.end())
    {
    // All substitutions have been prepared.  Generate this combination.
    cmStdString tag = m_Tag;
    cmStdString code = "";
    
    // The set of sources for the generated combination.  It will
    // always include the sources parsed from the original element
    // string.
    cmCableClass::Sources sources = m_Sources;
    
    // Put together all the pieces, with substitutions.
    for(Portions::const_iterator i = m_Portions.begin();
        i != Portions::const_iterator(m_Portions.end()); ++i)
      {
      // See if there is a class associated with this portion.
      const cmCableClass* curClassPortion = (*i)->GetClass();
      if(curClassPortion)
        {
        // Append the tag from the class portion.
        tag.append(curClassPortion->GetTag());
        
        // Include any sources needed by the class in this combination's set.
        for(cmCableClass::Sources::const_iterator
              s = curClassPortion->SourcesBegin();
            s != curClassPortion->SourcesEnd(); ++s)
          {
          sources.insert(*s);
          }
        }
      
      // Append the portion's code to this combination's code.
      code.append((*i)->GetCode());
      }
    
    // Add this combination to the output set.
    cmCableClass* cableClass = new cmCableClass(tag);
    cableClass->AddSources(sources);
    m_OutputSet->AddClass(code.c_str(), cableClass);
    }
  else
    {
    // Get the set for this substitution.
    const cmCableClassSet* set = substitution->first;
    if(set == m_OutputSet)
      {
      // We cannot iterate over the set currently being defined.
      cmSystemTools::Error("CABLE class set self-reference!");
      return;
      }
    
    // Prepare an iterator to the next substitution.
    Substitutions::const_iterator nextSubstitution = substitution;
    ++nextSubstitution;
    
    // We must iterate over all possible values for this substitution.
    for(cmCableClassSet::CableClassMap::const_iterator element = set->Begin();
        element != set->End(); ++element)
      {
      // Bind the substitution to this element.
      substitution->second->Bind(element->first, element->second);
      
      // Move on to the next substitution.
      this->Generate(nextSubstitution);
      }
    }
}


/**
 * Called from constructor.  Parses the given string to extract the
 * class information specified.
 *
 * The format of the string is
 *   [tag:]class_name[;source1;source2;...]
 */
void
ElementCombinationGenerator
::ParseInputElement(const char* in_element)
{
  // A regular expression to match the tagged element specification.
  cmRegularExpression taggedElement =
    "^([A-Za-z_0-9]*)[ \t]*:[ \t]*([^:].*|::.*)$";

  // A regular expression to match the element when more source files are given.
  cmRegularExpression sourcesRemain("^([^;]*);(.*)$");
  
  cmStdString elementWithoutTag;
  cmStdString sourceString;
  bool tagGiven = false;
  
  // See if the element was tagged, and if so, pull off the tag.
  if(taggedElement.find(in_element))
    {
    // A tag was given.  Use it.
    tagGiven = true;
    m_Tag = taggedElement.match(1);
    elementWithoutTag = taggedElement.match(2);
    }
  else
    {
    // No tag was given.  We will try to generate it later.
    elementWithoutTag = in_element;
    }

  // Separate the class name.
  if(sourcesRemain.find(elementWithoutTag.c_str()))
    {
    m_ClassName = sourcesRemain.match(1);
    sourceString = sourcesRemain.match(2);
    }
  else
    {
    m_ClassName = elementWithoutTag;
    }
  
  // Find any source files specified with the ";source" syntax.
  while(sourcesRemain.find(sourceString.c_str()))
    {
    m_Sources.insert(sourcesRemain.match(1));
    sourceString = sourcesRemain.match(2);
    }
  if(sourceString != "")
    {
    m_Sources.insert(sourceString);
    }

  // If no tag was given, try to generate one.
  if(!tagGiven)
    {
    if(!this->GenerateTag(m_ClassName))
      {
      cmSystemTools::Error("Cannot generate tag for class name: ",
                           m_ClassName.c_str(),
                           "\nPlease supply one with the \"tag:..\" syntax.");
      }
    }
  
  // If there is a .h with the name of the tag, add it as a source.
  this->FindTagSource();
  
  // Split the class name up into portions for the combination
  // generation method.
  this->SplitClassName();
}


/**
 * Parses the class name into portions.  Plain text in the string is
 * held by a StringPortion, and a $ token for replacement is
 * represented by a ReplacePortion.
 */
void
ElementCombinationGenerator
::SplitClassName()
{
  // Break the input code into blocks alternating between literal code and
  // set-substitution tokens (like $SomeSetName).
  cmStdString currentPortion = "";
  for(cmStdString::const_iterator c=m_ClassName.begin();
      c != m_ClassName.end(); ++c)
    {
    // Look for the '$' to mark the beginning of a token.
    if(*c != '$')
      {
      currentPortion.insert(currentPortion.end(), *c);
      }
    else
      {
      // If there is a portion of the string, record it.
      if(currentPortion.length() > 0)
        {
        m_Portions.push_back(new StringPortion(currentPortion));
        currentPortion = "";
        }
      // Skip over the '$' character.
      ++c;
      // Get element set name token.
      cmStdString setName = this->ParseSetName(c, m_ClassName.end());

      // We have a complete set name.  Look it up in makefile's data
      // collection.
      cmData* d = m_Makefile->LookupData(setName.c_str());
      // This should be a dynamic_cast, but we don't want to require RTTI.
      cmCableClassSet* set = static_cast<cmCableClassSet*>(d);
      if(set)
        {
        // We have a valid set name.  Prepare the substitution entry
        // for it.
        Substitution* sub;
        if(m_Substitutions.count(set) == 0)
          {
          sub = new Substitution();
          m_Substitutions[set] = sub;
          }
        else
          {
          sub = m_Substitutions[set];
          }
        m_Portions.push_back(new ReplacePortion(*sub));
        setName = "";
        }
      else
        {
        // Invalid set name.  Complain.
        cmSystemTools::Error("Unknown name of CABLE class set: ",
                             setName.c_str());
        }
      
      // Let the loop look at this character again.
      --c;
      }
    }
  
  // If there is a final portion of the string, record it.
  if(currentPortion.length() > 0)
    {
    m_Portions.push_back(new StringPortion(currentPortion));
    }
}


/**
 * Parse out the name of a Set specified after a $ in the element's string.
 * This is called with "c" pointing to the first character after the $,
 * and "end" equal to the string's end iterator.
 *
 * Returns the set name after parsing.  "c" will point to the first
 * character after the end of the set name.
 */
cmStdString
ElementCombinationGenerator
::ParseSetName(cmStdString::const_iterator& c, cmStdString::const_iterator end) const
{
  cmStdString setName = "";
  
  // Check for the $(setName) syntax.
  // If the first character after the '$' is a left paren, we scan for the
  // matching paren, and take everything in-between as the set name.
  if((c != end) && (*c == '('))
    {
    unsigned int depth = 1;
    ++c;
    while(c != end)
      {
      char ch = *c++;
      if(ch == '(') { ++depth; }
      else if(ch == ')') { --depth; }
      if(depth == 0) { break; }
      setName.insert(setName.end(), ch);
      }
    return setName;
    }
  
  // The $(setName) syntax was not used.
  // Look for all characters that can be part of a qualified C++
  // identifier.
  while(c != end)
    {
    char ch = *c;
    if(((ch >= 'a') && (ch <= 'z'))
       || ((ch >= 'A') && (ch <= 'Z'))
       || ((ch >= '0') && (ch <= '9'))
       || (ch == '_') || (ch == ':'))
      {
      setName.insert(setName.end(), ch);
      ++c;
      }
    else
      {
      break;
      }
    }
  return setName;
}


/**
 * After the tag for an element has been determined, but before
 * combination expansion is done, this is called to search for a
 * header file in the makefile's include path with the name of the
 * tag.  This makes specifying lists of classes that are declared in
 * header files with their own name very convenient.
 */
void ElementCombinationGenerator::FindTagSource()
{
  // If there is no tag, don't bother with this step.
  if(m_Tag == "")
    {
    return;
    }

  // Get the makefile's include path.
  const std::vector<std::string>& includePath =
    m_Makefile->GetIncludeDirectories();

  // Search the path for a file called "(m_Tag).h".
  for(std::vector<std::string>::const_iterator dir = includePath.begin();
      dir != includePath.end(); ++dir)
    {
    cmStdString filePath = *dir;
    m_Makefile->ExpandVariablesInString(filePath);
    filePath += "/"+m_Tag+".h";
    if(cmSystemTools::FileExists(filePath.c_str()))
      {
      m_Sources.insert(m_Tag+".h");
      return;
      }
    }
}


/**
 * Given the string representing a set element, automatically generate
 * the element tag for it.  This function determines how the output
 * language of all CABLE-generated wrappers will look.
 */
bool ElementCombinationGenerator::GenerateTag(const cmStdString& element)
{
  // Hold the regular expressions for matching against the element.
  cmRegularExpression regex;
  
  // If the element's code begins in a $, it is referring to a set name.
  // The set's elements have their own tags, so we don't need one.
  regex.compile("^[ \t]*\\$");
  if(regex.find(element))
    { m_Tag = ""; return true; }
  
  // Test for simple integer
  regex.compile("^[ \t]*([0-9]*)[ \t]*$");
  if(regex.find(element))
    {
    m_Tag = "_";
    m_Tag.append(regex.match(1));
    return true;
    }

  // Test for basic integer type
  regex.compile("^[ \t]*(unsigned[ ]|signed[ ])?[ \t]*(char|short|int|long|long[ ]long)[ \t]*$");
  if(regex.find(element))
    {
    m_Tag = "_";
    if(regex.match(1) == "unsigned ")
      { m_Tag.append("u"); }
    if(regex.match(2) == "long long")
      { m_Tag.append("llong"); }
    else
      { m_Tag.append(regex.match(2)); }
    return true;
    }

  // Test for basic floating-point type
  regex.compile("^[ \t]*(long[ ])?[ \t]*(float|double)[ \t]*$");
  if(regex.find(element))
    {
    m_Tag = "_";
    if(static_cast<int>(regex.start(1)) > 0 && regex.match(1) == "long ")
      {
      m_Tag.append("l");
      }
    m_Tag.append(regex.match(2));
    return true;
    }

  // Test for basic wide-character type
  regex.compile("^[ \t]*(wchar_t)[ \t]*$");
  if(regex.find(element))
    {
    m_Tag = "_wchar";
    return true;
    }

  // Test for type name (possibly with template arguments).
  regex.compile("^[ \t]*([A-Za-z_][A-Za-z0-9_]*)(<.*)?[ \t]*$");
  if(regex.find(element))
    {
    // The tag is the same as the type.  If there were template arguments,
    // they are ignored since they may have their own tags.
    m_Tag = regex.match(1);
    return true;
    }
  
  // Test for a name with a single namespace qualifier.
  regex.compile("^[ \t]*([A-Za-z_][A-Za-z0-9_]*)::([A-Za-z_][A-Za-z0-9_]*)(<.*)?[ \t]*$");
  if(regex.find(element))
    {
    // The tag is the same as the namespace and type concatenated together.
    m_Tag = regex.match(1);
    m_Tag.append(regex.match(2));
    return true;
    }
  
  // We can't generate a tag.
  m_Tag = "";  
  return false;
}


/**
 * Given an element in string form, parse out the information from it,
 * generate the combinations of set substitutions, and add all the
 * elements that result.
 */
void cmCableClassSet::ParseAndAddElement(const char* in_element,
                                         cmMakefile* makefile)
{  
  // Create an object to handle the generation.
  ElementCombinationGenerator combinationGenerator(in_element, makefile, this);
  
  // Generate the combinations.
  combinationGenerator.Generate();
}

