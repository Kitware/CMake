/*=========================================================================

  Program:   KWSys - Kitware System Library
  Module:    $RCSfile$

  Copyright (c) Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "kwsysPrivate.h"
#include KWSYS_HEADER(CommandLineArguments.hxx)
#include KWSYS_HEADER(ios/iostream)
#include KWSYS_HEADER(stl/vector)

// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.
#if 0
# include "CommandLineArguments.hxx.in"
# include "kwsys_ios_iostream.h.in"
#endif

int main(int argc, char* argv[])
{
  kwsys::CommandLineArguments arg;
  arg.Initialize(argc, argv);

  int n = 0;
  char* m = 0;
  kwsys_stl::string p;
  int res = 0;

  typedef kwsys::CommandLineArguments argT;
  arg.AddArgument("-n", argT::SPACE_ARGUMENT, &n, "Argument N");
  arg.AddArgument("-m", argT::EQUAL_ARGUMENT, &m, "Argument M");
  arg.AddBooleanArgument("-p", &p, "Argument P");

  arg.StoreUnusedArguments(true);

  if ( !arg.Parse() )
    {
    kwsys_ios::cerr << "Problem parsing arguments" << kwsys_ios::endl;
    res = 1;
    }
  if ( n != 24 )
    {
    kwsys_ios::cout << "Problem setting N. Value of N: " << n << kwsys_ios::endl;
    res = 1;
    }
  if ( !m || strcmp(m, "test value") != 0 )
    {
    kwsys_ios::cout << "Problem setting M. Value of M: " << m << kwsys_ios::endl;
    res = 1;
    }
  if ( p != "1" )
    {
    kwsys_ios::cout << "Problem setting P. Value of P: " << p.c_str() << kwsys_ios::endl;
    res = 1;
    }
  kwsys_ios::cout << "Value of N: " << n << kwsys_ios::endl;
  kwsys_ios::cout << "Value of M: " << m << kwsys_ios::endl;
  kwsys_ios::cout << "Value of P: " << p.c_str() << kwsys_ios::endl;

  char** newArgv = 0;
  int newArgc = 0;
  arg.GetUnusedArguments(&newArgc, &newArgv);
  int cc;
  char* valid_unused_args[9] = {
    0, "--ignored", "--second-ignored", "third-ignored",
    "some", "junk", "at", "the", "end"
  };
  if ( newArgc != 9 )
    {
    kwsys_ios::cerr << "Bad number of unused arguments: " << newArgc << kwsys_ios::endl;
    res = 1;
    }
  for ( cc = 0; cc < newArgc; ++ cc )
    {
    kwsys_ios::cout << "Unused argument[" << cc << "] = [" << newArgv[cc] << "]"
      << kwsys_ios::endl;
    if ( cc >= 9 )
      {
      kwsys_ios::cerr << "Too many unused arguments: " << cc << kwsys_ios::endl;
      res = 1;
      }
    else if ( valid_unused_args[cc] &&
      strcmp(valid_unused_args[cc], newArgv[cc]) != 0 )
      {
      kwsys_ios::cerr << "Bad unused argument [" << cc << "] \""
        << newArgv[cc] << "\" should be: \"" << valid_unused_args[cc] << "\""
        << kwsys_ios::endl;
      res = 1;
      }
    }
  arg.DeleteRemainingArguments(newArgc, &newArgv);

  if ( m )
    {
    delete [] m;
    }
  return res;
}
 
