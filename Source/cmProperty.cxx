/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmProperty.h"
#include "cmSystemTools.h"

void cmProperty::Set(const char *name, const char *value)
{
  this->Name = name;
  this->Value = value;
  this->ValueHasBeenSet = true;
}

void cmProperty::Append(const char *name, const char *value)
{
  this->Name = name;
  if(!this->Value.empty() && *value)
    {
    this->Value += ";";
    }
  this->Value += value;
  this->ValueHasBeenSet = true;
}

const char *cmProperty::GetValue() const
{
  if (this->ValueHasBeenSet)
    {
    return this->Value.c_str();
    }
  return 0;
}
