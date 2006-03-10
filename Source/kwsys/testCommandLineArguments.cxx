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

// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.
#if 0
# include "CommandLineArguments.hxx.in"
# include "kwsys_ios_iostream.h.in"
#endif

void* random_ptr = reinterpret_cast<void*>(0x123);

int argument(const char* arg, const char* value, void* call_data)
{
  kwsys_ios::cout << "Got argument: \"" << arg << "\" value: \"" << (value?value:"(null)") << "\"" << kwsys_ios::endl;
  if ( call_data != random_ptr )
    {
    kwsys_ios::cerr << "Problem processing call_data" << kwsys_ios::endl;
    return 0;
    }
  return 1;
}

int unknown_argument(const char* argument, void* call_data)
{
  kwsys_ios::cout << "Got unknown argument: \"" << argument << "\"" << kwsys_ios::endl;
  if ( call_data != random_ptr )
    {
    kwsys_ios::cerr << "Problem processing call_data" << kwsys_ios::endl;
    return 0;
    }
  return 1;
}

int main(int argc, char* argv[])
{
  // Example run: ./testCommandLineArguments --some-int-variable 4
  // --another-bool-variable --some-bool-variable=yes
  // --some-stl-string-variable=foobar --set-bool-arg1 --set-bool-arg2
  // --some-string-variable=hello

  int res = 0;
  kwsys::CommandLineArguments arg;
  arg.Initialize(argc, argv);

  // For error handling
  arg.SetClientData(random_ptr);
  arg.SetUnknownArgumentCallback(unknown_argument);

  int some_int_variable = 10;
  double some_double_variable = 10.10;
  char* some_string_variable = 0;
  kwsys_stl::string some_stl_string_variable = "";
  bool some_bool_variable = false;
  bool some_bool_variable1 = false;
  bool bool_arg1 = false;
  int bool_arg2 = 0;

  typedef kwsys::CommandLineArguments argT;

  arg.AddArgument("--some-int-variable", argT::SPACE_ARGUMENT, &some_int_variable, "Set some random int variable");
  arg.AddArgument("--some-double-variable", argT::CONCAT_ARGUMENT, &some_double_variable, "Set some random double variable");
  arg.AddArgument("--some-string-variable", argT::EQUAL_ARGUMENT, &some_string_variable, "Set some random string variable");
  arg.AddArgument("--some-stl-string-variable", argT::EQUAL_ARGUMENT, &some_stl_string_variable, "Set some random stl string variable");
  arg.AddArgument("--some-bool-variable", argT::EQUAL_ARGUMENT, &some_bool_variable, "Set some random bool variable");
  arg.AddArgument("--another-bool-variable", argT::NO_ARGUMENT, &some_bool_variable1, "Set some random bool variable 1");
  arg.AddBooleanArgument("--set-bool-arg1", &bool_arg1, "Test AddBooleanArgument 1");
  arg.AddBooleanArgument("--set-bool-arg2", &bool_arg2, "Test AddBooleanArgument 2");

  arg.AddCallback("-A", argT::NO_ARGUMENT, argument, random_ptr, "Some option -A. This option has a multiline comment. It should demonstrate how the code splits lines.");
  arg.AddCallback("-B", argT::SPACE_ARGUMENT, argument, random_ptr, "Option -B takes argument with space");
  arg.AddCallback("-C", argT::EQUAL_ARGUMENT, argument, random_ptr, "Option -C takes argument after =");
  arg.AddCallback("-D", argT::CONCAT_ARGUMENT, argument, random_ptr, "This option takes concatinated argument");
  arg.AddCallback("--long1", argT::NO_ARGUMENT, argument, random_ptr, "-A");
  arg.AddCallback("--long2", argT::SPACE_ARGUMENT, argument, random_ptr, "-B");
  arg.AddCallback("--long3", argT::EQUAL_ARGUMENT, argument, random_ptr, "Same as -C but a bit different");
  arg.AddCallback("--long4", argT::CONCAT_ARGUMENT, argument, random_ptr, "-C");

  if ( !arg.Parse() )
    {
    kwsys_ios::cerr << "Problem parsing arguments" << kwsys_ios::endl;
    res = 1;
    }
  kwsys_ios::cout << "Help: " << arg.GetHelp() << kwsys_ios::endl;

  kwsys_ios::cout << "Some int variable was set to: " << some_int_variable << kwsys_ios::endl;
  kwsys_ios::cout << "Some double variable was set to: " << some_double_variable << kwsys_ios::endl;
  if ( some_string_variable )
    {
    kwsys_ios::cout << "Some string variable was set to: " << some_string_variable << kwsys_ios::endl;
    delete [] some_string_variable;
    }
  else
    {
    kwsys_ios::cerr << "Problem setting string variable" << kwsys_ios::endl;
    res = 1;
    }
  kwsys_ios::cout << "Some STL String variable was set to: " << some_stl_string_variable.c_str() << kwsys_ios::endl;
  kwsys_ios::cout << "Some bool variable was set to: " << some_bool_variable << kwsys_ios::endl;
  kwsys_ios::cout << "Some bool variable was set to: " << some_bool_variable1 << kwsys_ios::endl;
  kwsys_ios::cout << "bool_arg1 variable was set to: " << bool_arg1 << kwsys_ios::endl;
  kwsys_ios::cout << "bool_arg2 variable was set to: " << bool_arg2 << kwsys_ios::endl;
  kwsys_ios::cout << kwsys_ios::endl;
  return res;
}
