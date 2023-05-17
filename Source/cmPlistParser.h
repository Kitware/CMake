/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <string>

#include <cm/optional>

namespace Json {
class Value;
}

cm::optional<Json::Value> cmParsePlist(const std::string& filename);
