%module Foo

%{
#include <foo.hpp>
%}

// %nspace cs::my_class_in_namespace;
%include <foo.hpp>
