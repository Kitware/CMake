/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) 2000 National Library of Medicine
  All rights reserved.

  See COPYRIGHT.txt for copyright details.

=========================================================================*/
/// Original Copyright notice:
// Copyright (C) 1991 Texas Instruments Incorporated.
//
// Permission is granted to any individual or institution to use, copy, modify,
// and distribute this software, provided that this complete copyright and
// permission notice is maintained, intact, in all copies and supporting
// documentation.
//
// Texas Instruments Incorporated provides this software "as is" without
// express or implied warranty.
//
// .LIBRARY vbl
// .HEADER Basics Package
// .INCLUDE cmRegularExpression.h
// .FILE cmRegularExpression.cxx
//
#ifndef cmRegularExpression_h
#define cmRegularExpression_h

#include "cmStandardIncludes.h"

const int NSUBEXP = 10;

//: Pattern matching with regular expressions
//  A regular expression allows a programmer to specify  complex
//  patterns  that  can  be searched for and matched against the
//  character string of a string object. In its simplest form, a
//  regular  expression  is  a  sequence  of  characters used to
//  search for exact character matches. However, many times  the
//  exact  sequence to be found is not known, or only a match at
//  the beginning or end of a string is desired. The vbl  regu-
//  lar  expression  class implements regular expression pattern
//  matching as is found and implemented in many  UNIX  commands
//  and utilities.
//
//  Example: The perl code
//  
//     $filename =~ m"([a-z]+)\.cc";
//     print $1;
//     
//  Is written as follows in C++
//
//     vbl_reg_exp re("([a-z]+)\\.cc");
//     re.find(filename);
//     cerr << re.match(1);
//
//
//  The regular expression class provides a convenient mechanism
//  for  specifying  and  manipulating  regular expressions. The
//  regular expression object allows specification of such  pat-
//  terns  by using the following regular expression metacharac-
//  ters:
// 
//   ^        Matches at beginning of a line
//
//   $        Matches at end of a line
//
//  .         Matches any single character
//
//  [ ]       Matches any character(s) inside the brackets
//
//  [^ ]      Matches any character(s) not inside the brackets
//
//   -        Matches any character in range on either side of a dash
//
//   *        Matches preceding pattern zero or more times
//
//   +        Matches preceding pattern one or more times
//
//   ?        Matches preceding pattern zero or once only
//
//  ()        Saves a matched expression and uses it in a  later match
// 
//  Note that more than one of these metacharacters can be  used
//  in  a  single  regular expression in order to create complex
//  search patterns. For example, the pattern [^ab1-9]  says  to
//  match  any  character  sequence that does not begin with the
//  characters "ab"  followed  by  numbers  in  the  series  one
//  through nine.
//
class cmRegularExpression {
public:
  inline cmRegularExpression ();			// cmRegularExpression with program=NULL
  inline cmRegularExpression (char const*);	// cmRegularExpression with compiled char*
  cmRegularExpression (cmRegularExpression const&);	// Copy constructor
  inline ~cmRegularExpression();			// Destructor 

  void compile (char const*);		// Compiles char* --> regexp
  bool find (char const*);		// true if regexp in char* arg
  bool find (std::string const&);		// true if regexp in char* arg
  inline long start() const;		// Index to start of first find
  inline long end() const;		// Index to end of first find

  bool operator== (cmRegularExpression const&) const;	// Equality operator
  inline bool operator!= (cmRegularExpression const&) const; // Inequality operator
  bool deep_equal (cmRegularExpression const&) const;	// Same regexp and state?
  
  inline bool is_valid() const;		// true if compiled regexp
  inline void set_invalid();		// Invalidates regexp

  // awf added
  int start(int n) const;
  int end(int n) const;
  std::string match(int n) const;
  
private: 
  const char* startp[NSUBEXP];
  const char* endp[NSUBEXP];
  char  regstart;			// Internal use only
  char  reganch;			// Internal use only
  const char* regmust;			// Internal use only
  int   regmlen;			// Internal use only
  char* program;   
  int   progsize;
  const char* searchstring;
}; 

// cmRegularExpression -- Creates an empty regular expression.

inline cmRegularExpression::cmRegularExpression () { 
  this->program = NULL;
}


// cmRegularExpression -- Creates a regular expression from string s, and
// compiles s.


inline cmRegularExpression::cmRegularExpression (const char* s) {  
  this->program = NULL;
  compile(s);
}

// ~cmRegularExpression -- Frees space allocated for regular expression.

inline cmRegularExpression::~cmRegularExpression () {
//#ifndef WIN32
  delete [] this->program;
//#endif
}

// Start -- 

inline long cmRegularExpression::start () const {
  return(this->startp[0] - searchstring);
}


// End -- Returns the start/end index of the last item found.


inline long cmRegularExpression::end () const {
  return(this->endp[0] - searchstring);
}


// operator!= //

inline bool cmRegularExpression::operator!= (const cmRegularExpression& r) const {
  return(!(*this == r));
}


// is_valid -- Returns true if a valid regular expression is compiled
// and ready for pattern matching.

inline bool cmRegularExpression::is_valid () const {
  return (this->program != NULL);
}


// set_invalid -- Invalidates regular expression.

inline void cmRegularExpression::set_invalid () {
//#ifndef WIN32
  delete [] this->program;
//#endif
  this->program = NULL;
}

// -- Return start index of nth submatch. start(0) is the start of the full match.
inline int cmRegularExpression::start(int n) const
{
  return this->startp[n] - searchstring;
}

// -- Return end index of nth submatch. end(0) is the end of the full match.
inline int cmRegularExpression::end(int n) const
{
  return this->endp[n] - searchstring;
}

// -- Return nth submatch as a string.
inline std::string cmRegularExpression::match(int n) const
{
  return std::string(this->startp[n], this->endp[n] - this->startp[n]);
}

#endif // cmRegularExpressionh
