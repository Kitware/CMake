/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <string>

enum class cmDepfileFormat
{
  GccDepfile,
  MakeDepfile,
  MSBuildAdditionalInputs,
};

class cmLocalGenerator;

bool cmTransformDepfile(cmDepfileFormat format, const cmLocalGenerator& lg,
                        const std::string& infile, const std::string& outfile);
