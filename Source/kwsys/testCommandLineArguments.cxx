/*=========================================================================

  Program:   KWSys - Kitware System Library
  Module:    $RCSfile$

  Copyright (c) Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include <kwsys/CommandLineArguments.hxx>

void* random_ptr = (void*)0x123;

int argument(const char* arg, const char* value, void* call_data)
{
  cout << "Got argument: \"" << arg << "\" value: \"" << (value?value:"(null)") << "\"" << endl;
  if ( call_data != random_ptr )
    {
    cerr << "Problem processing call_data" << endl;
    return 0;
    }
  return 1;
}

int unknown_argument(const char* argument, void* call_data)
{
  cout << "Got unknown argument: \"" << argument << "\"" << endl;
  if ( call_data != random_ptr )
    {
    cerr << "Problem processing call_data" << endl;
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
    cerr << "Problem parsing arguments" << endl;
    res = 1;
    }
  cout << "Help: " << arg.GetHelp() << endl;

  cout << "Some int variable was set to: " << some_int_variable << endl;
  cout << "Some double variable was set to: " << some_double_variable << endl;
  if ( some_string_variable )
    {
    cout << "Some string variable was set to: " << some_string_variable << endl;
    delete [] some_string_variable;
    }
  else
    {
    cerr << "Problem setting string variable" << endl;
    res = 1;
    }
  cout << "Some STL String variable was set to: " << some_stl_string_variable.c_str() << endl;
  cout << "Some bool variable was set to: " << some_bool_variable << endl;
  cout << "Some bool variable was set to: " << some_bool_variable1 << endl;
  cout << "bool_arg1 variable was set to: " << bool_arg1 << endl;
  cout << "bool_arg2 variable was set to: " << bool_arg2 << endl;
  cout << endl;
  return res;
}
