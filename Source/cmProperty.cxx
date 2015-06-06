/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmProperty.h"
#include "cmSystemTools.h"

void cmProperty::Set(const char *value)
{
  this->Value = value;
  this->ValueHasBeenSet = true;
}

void cmProperty::Append(const char *value, bool asString)
{
  if(!this->Value.empty() && *value && !asString)
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
