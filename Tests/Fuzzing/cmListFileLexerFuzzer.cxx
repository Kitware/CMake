/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

/*
 * Fuzzer for CMake's ListFile lexer (CMakeLists.txt parser)
 *
 * This fuzzer targets cmListFileLexer which tokenizes CMakeLists.txt files.
 * It's a critical attack surface as malicious CMakeLists.txt files could be
 * encountered when building untrusted projects.
 *
 * Coverage targets:
 * - Token parsing (identifiers, strings, brackets, comments)
 * - BOM handling (UTF-8, UTF-16, UTF-32)
 * - Bracket argument/comment parsing
 * - Error recovery for malformed input
 */

#include <cstddef>
#include <cstdint>

#include "cmListFileLexer.h"

// Limit input size to avoid timeouts on complex inputs
static constexpr size_t kMaxInputSize = 64 * 1024; // 64KB

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* data, size_t size)
{
  // Skip overly large inputs
  if (size == 0 || size > kMaxInputSize) {
    return 0;
  }

  cmListFileLexer* lexer = cmListFileLexer_New();
  if (!lexer) {
    return 0;
  }

  // Parse from string (not file) for efficiency
  if (cmListFileLexer_SetString(lexer, reinterpret_cast<char const*>(data),
                                size)) {
    // Consume all tokens until EOF or error
    cmListFileLexer_Token* token;
    while ((token = cmListFileLexer_Scan(lexer)) != nullptr) {
      // Access token fields to ensure they're valid
      (void)token->type;
      (void)token->text;
      (void)token->length;
      (void)token->line;
      (void)token->column;

      // Get type as string for additional coverage
      (void)cmListFileLexer_GetTypeAsString(lexer, token->type);
    }

    // Exercise position tracking
    (void)cmListFileLexer_GetCurrentLine(lexer);
    (void)cmListFileLexer_GetCurrentColumn(lexer);
  }

  cmListFileLexer_Delete(lexer);
  return 0;
}
