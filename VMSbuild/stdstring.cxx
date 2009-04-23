// This file is a hack to get the VMS CC to define the templates for basic_string

#include <string>

#pragma define_template std::basic_string<char, std::char_traits<char >, std::allocator<char > >



