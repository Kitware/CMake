/* Copyright (C) 2001 by Eric Kidd. All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission. 
**  
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
** OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
** SUCH DAMAGE. */


/*=========================================================================
**  XML-RPC UTF-8 Utilities
**=========================================================================
**  Routines for validating, encoding and decoding UTF-8 data.  We try to
**  be very, very strict about invalid UTF-8 data.
**
**  All of the code in this file assumes that your machine represents
**  wchar_t as a 16-bit (or wider) character containing UCS-2 data.  If this
**  assumption is incorrect, you may need to replace this file.
**
**  For lots of information on Unicode and UTF-8 decoding, see:
**    http://www.cl.cam.ac.uk/~mgk25/unicode.html
*/

#include "xmlrpc_config.h"

#include "xmlrpc.h"

#ifdef HAVE_UNICODE_WCHAR

/*=========================================================================
**  Tables and Constants
**=========================================================================
**  We use a variety of tables and constants to help decode and validate
**  UTF-8 data.
*/

/* The number of bytes in a UTF-8 sequence starting with the character used
** as the array index.  A zero entry indicates an illegal initial byte.
** This table was generated using a Perl script and information from the
** UTF-8 standard.
**
** Fredrik Lundh's UTF-8 decoder Python 2.0 uses a similar table.  But
** since Python 2.0 has the icky CNRI license, I regenerated this
** table from scratch and wrote my own decoder. */
static unsigned char utf8_seq_length[256] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 0, 0
};

/* The minimum legal character value for a UTF-8 sequence of the given
** length.  We have to check this to avoid accepting "overlong" UTF-8
** sequences, which use more bytes than necessary to encode a given
** character.  Such sequences are commonly used by evil people to bypass
** filters and security checks.  This table is based on the UTF-8-test.txt
** file by Markus Kuhn <mkuhn@acm.org>. */
static wchar_t utf8_min_char_for_length[4] = {
    0,          /* Length 0: Not used (meaningless) */
    0x0000,     /* Length 1: Not used (special-cased) */
    0x0080,     /* Length 2 */
    0x0800      /* Length 3 */

#if 0
    /* These are only useful on systems where wchar_t is 32-bits wide
    ** and supports full UCS-4. */
    0x00010000, /* Length 4 */
    0x00200000, /* Length 5 */
    0x04000000  /* Length 6 */
#endif
};

/* This is the maximum legal 16-byte (UCS-2) character.  Again, this
** information is based on UTF-8-test.txt. */
#define UCS2_MAX_LEGAL_CHARACTER (0xFFFD)

/* First and last UTF-16 surrogate characters.  These are *not* legal UCS-2
** characters--they're used to code for UCS-4 characters when using
** UTF-16.  They should never appear in decoded UTF-8 data!  Again, these
** could hypothetically be used to bypass security measures on some machines.
** Based on UTF-8-test.txt. */
#define UTF16_FIRST_SURROGATE (0xD800)
#define UTF16_LAST_SURROGATE  (0xDFFF)

/* Is the character 'c' a UTF-8 continuation character? */
#define IS_CONTINUATION(c) (((c) & 0xC0) == 0x80)

/* Maximum number of bytes needed to encode a supported character. */
#define MAX_ENCODED_BYTES (3)


/*=========================================================================
**  decode_utf8
**=========================================================================
**  Internal routine which decodes (or validates) a UTF-8 string.
**  To validate, set io_buff and out_buff_len to NULL.  To decode, allocate
**  a sufficiently large buffer, pass it as io_buff, and pass a pointer as
**  as out_buff_len.  The data will be written to the buffer, and the
**  length to out_buff_len.
**
**  We assume that wchar_t holds a single UCS-2 character in native-endian
**  byte ordering.
*/

static void 
decode_utf8(xmlrpc_env * const env,
            const char * const utf8_data,
            size_t       const utf8_len,
            wchar_t *    const io_buff,
            size_t *     const out_buff_len) {

    size_t i, length, out_pos;
    char init, con1, con2;
    wchar_t wc;

    XMLRPC_ASSERT_ENV_OK(env);
    XMLRPC_ASSERT_PTR_OK(utf8_data);
    XMLRPC_ASSERT((!io_buff && !out_buff_len) ||
                  (io_buff && out_buff_len));

    /* Suppress GCC warning about possibly undefined variable. */
    wc = 0;

    i = 0;
    out_pos = 0;
    while (i < utf8_len) {
        init = utf8_data[i];
        if ((init & 0x80) == 0x00) {
            /* Convert ASCII character to wide character. */
            wc = init;
            i++;
        } else {
            /* Look up the length of this UTF-8 sequence. */
            length = utf8_seq_length[(unsigned char) init];
            
            /* Check to make sure we have enough bytes to convert. */
            if (i + length > utf8_len)
                XMLRPC_FAIL(env, XMLRPC_INVALID_UTF8_ERROR,
                            "Truncated UTF-8 sequence");
            
            /* Decode a multibyte UTF-8 sequence. */
            switch (length) {
            case 0:
                XMLRPC_FAIL(env, XMLRPC_INVALID_UTF8_ERROR,
                            "Invalid UTF-8 initial byte");
                
            case 2:
                /* 110xxxxx 10xxxxxx */
                con1 = utf8_data[i+1];
                if (!IS_CONTINUATION(con1))
                    XMLRPC_FAIL(env, XMLRPC_INVALID_UTF8_ERROR,
                                "UTF-8 sequence too short");
                wc = ((((wchar_t) (init & 0x1F)) <<  6) |
                      (((wchar_t) (con1 & 0x3F))));
                break;
                
            case 3:
                /* 1110xxxx 10xxxxxx 10xxxxxx */
                con1 = utf8_data[i+1];
                con2 = utf8_data[i+2];
                if (!IS_CONTINUATION(con1) || !IS_CONTINUATION(con2))
                    XMLRPC_FAIL(env, XMLRPC_INVALID_UTF8_ERROR,
                                "UTF-8 sequence too short");
                wc = ((((wchar_t) (init & 0x0F)) << 12) |
                      (((wchar_t) (con1 & 0x3F)) <<  6) |
                      (((wchar_t) (con2 & 0x3F))));
                break;
                
            case 4:
                /* 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
            case 5:
                /* 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx */
            case 6:
                /* 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx */
                XMLRPC_FAIL(env, XMLRPC_INVALID_UTF8_ERROR,
                            "UCS-4 characters not supported");
                
            default:
                XMLRPC_ASSERT("Error in UTF-8 decoder tables");
            }
                    
            /* Advance to the end of the sequence. */
            i += length;
            
            /* Check for illegal UCS-2 characters. */
            if (wc > UCS2_MAX_LEGAL_CHARACTER)
                XMLRPC_FAIL(env, XMLRPC_INVALID_UTF8_ERROR,
                            "UCS-2 characters > U+FFFD are illegal");
            
            /* Check for UTF-16 surrogates. */
            if (UTF16_FIRST_SURROGATE <= wc && wc <= UTF16_LAST_SURROGATE)
                XMLRPC_FAIL(env, XMLRPC_INVALID_UTF8_ERROR,
                            "UTF-16 surrogates may not appear in UTF-8 data");
            
            /* Check for overlong sequences. */
            if (wc < utf8_min_char_for_length[length])
                XMLRPC_FAIL(env, XMLRPC_INVALID_UTF8_ERROR,
                            "Overlong UTF-8 sequence not allowed");
        }
        
        /* If we have a buffer, write our character to it. */
        if (io_buff) {
            io_buff[out_pos++] = wc;
        }
    }
    
    /* Record the number of characters we found. */
    if (out_buff_len)
        *out_buff_len = out_pos;
    
 cleanup:
    if (env->fault_occurred) {
        if (out_buff_len)
            *out_buff_len = 0;
    }
}



/*=========================================================================
**  xmlrpc_validate_utf8
**=========================================================================
**  Make sure that a UTF-8 string is valid.
*/

void 
xmlrpc_validate_utf8 (xmlrpc_env * const env,
                      const char * const utf8_data,
                      size_t       const utf8_len) {

    decode_utf8(env, utf8_data, utf8_len, NULL, NULL);
}


/*=========================================================================
**  xmlrpc_utf8_to_wcs
**=========================================================================
**  Decode UTF-8 string to a "wide character string".  This function
**  returns an xmlrpc_mem_block with an element type of wchar_t.  Don't
**  try to intepret the block in a bytewise fashion--it won't work in
**  any useful or portable fashion.
*/

xmlrpc_mem_block *xmlrpc_utf8_to_wcs (xmlrpc_env *env,
                                      char *utf8_data,
                                      size_t utf8_len)
{
    xmlrpc_mem_block *output;
    size_t wcs_length;

    /* Allocate a memory block large enough to hold any possible output.
    ** We assume that each byte of the input may decode to a whcar_t. */
    output = XMLRPC_TYPED_MEM_BLOCK_NEW(wchar_t, env, utf8_len);
    XMLRPC_FAIL_IF_FAULT(env);

    /* Decode the UTF-8 data. */
    decode_utf8(env, utf8_data, utf8_len,
                XMLRPC_TYPED_MEM_BLOCK_CONTENTS(wchar_t, output),
                &wcs_length);
    XMLRPC_FAIL_IF_FAULT(env);

    /* Make sure we didn't overrun our buffer. */
    XMLRPC_ASSERT(wcs_length <= utf8_len);

    /* Correct the length of the memory block. */
    XMLRPC_TYPED_MEM_BLOCK_RESIZE(wchar_t, env, output, wcs_length);
    XMLRPC_FAIL_IF_FAULT(env);

 cleanup:
    if (env->fault_occurred) {
        if (output)
            xmlrpc_mem_block_free(output);
        return NULL;
    }
    return output;
}


/*=========================================================================
**  xmlrpc_utf8_to_wcs
**=========================================================================
**  Encode a "wide character string" as UTF-8.
*/

xmlrpc_mem_block *xmlrpc_wcs_to_utf8 (xmlrpc_env *env,
                                      wchar_t *wcs_data,
                                      size_t wcs_len)
{
    size_t estimate, bytes_used, i;
    xmlrpc_mem_block *output;
    unsigned char *buffer;
    wchar_t wc;
    int cwc;

    XMLRPC_ASSERT_ENV_OK(env);
    XMLRPC_ASSERT_PTR_OK(wcs_data);

    /* Allocate a memory block large enough to hold any possible output.
    ** We assume that every wchar might encode to the maximum length. */
    estimate = wcs_len * MAX_ENCODED_BYTES;
    output = XMLRPC_TYPED_MEM_BLOCK_NEW(char, env, estimate);
    XMLRPC_FAIL_IF_FAULT(env);

    /* Output our characters. */
    buffer = (unsigned char*) XMLRPC_TYPED_MEM_BLOCK_CONTENTS(char, output);
    bytes_used = 0;
    for (i = 0; i < wcs_len; i++) {
        wc = wcs_data[i];
        cwc = wc;
        if (cwc <= 0x007F) {
            buffer[bytes_used++] = wc & 0x7F;
        } else if (cwc <= 0x07FF) {
            /* 110xxxxx 10xxxxxx */
            buffer[bytes_used++] = 0xC0 | (wc >> 6);
            buffer[bytes_used++] = 0x80 | (wc & 0x3F);
        } else if (cwc <= 0xFFFF) {
            /* 1110xxxx 10xxxxxx 10xxxxxx */
            buffer[bytes_used++] = 0xE0 | (wc >> 12);
            buffer[bytes_used++] = 0x80 | ((wc >> 6) & 0x3F);
            buffer[bytes_used++] = 0x80 | (wc & 0x3F);
        } else {
            XMLRPC_FAIL(env, XMLRPC_INTERNAL_ERROR,
                        "Don't know how to encode UCS-4 characters yet");
        }
    }

    /* Make sure we didn't overrun our buffer. */
    XMLRPC_ASSERT(bytes_used <= estimate);

    /* Correct the length of the memory block. */
    XMLRPC_TYPED_MEM_BLOCK_RESIZE(char, env, output, bytes_used);
    XMLRPC_FAIL_IF_FAULT(env);

 cleanup:
    if (env->fault_occurred) {
        if (output)
            xmlrpc_mem_block_free(output);
        return NULL;
    }
    return output;
}

#endif /* HAVE_UNICODE_WCHAR */
