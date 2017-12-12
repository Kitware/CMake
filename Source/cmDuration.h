/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <chrono>
#include <ratio>

typedef std::chrono::duration<double, std::ratio<1>> cmDuration;
