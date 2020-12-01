/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cm/string_view>

/**
 * Process AUTOMOC and AUTOUIC
 * @return true on success
 */
bool cmQtAutoMocUic(cm::string_view infoFile, cm::string_view config);
