%module NSExample

%{
#include "ns_example.hpp"
%}

%nspace ns::my_class_in_namespace;
%include "ns_example.hpp"
