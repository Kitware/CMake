/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <string>

enum class cmDepfileFormat
{
  GccDepfile,
  MakeDepfile,
  MSBuildAdditionalInputs,
};

class cmLocalGenerator;

bool cmTransformDepfile(cmDepfileFormat format, cmLocalGenerator const& lg,
                        std::string const& infile, std::string const& outfile);
