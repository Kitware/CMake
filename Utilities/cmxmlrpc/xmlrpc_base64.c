/* Copyright (C) 2001 by First Peer, Inc. All rights reserved.
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
** SUCH DAMAGE.
**
** There is more copyright information in the bottom half of this file. 
** Please see it for more details. */


/*=========================================================================
**  XML-RPC Base64 Utilities
**=========================================================================
**  This code was swiped from Jack Jansen's code in Python 1.5.2 and
**  modified to work with our data types.
*/

#include "xmlrpc_config.h"

#include "xmlrpc.h"

#define CRLF    "\015\012"
#define CR      '\015'
#define LF      '\012'


/***********************************************************
Copyright 1991, 1992, 1993, 1994 by Stichting Mathematisch Centrum,
Amsterdam, The Netherlands.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the names of Stichting Mathematisch
Centrum or CWI or Corporation for National Research Initiatives or
CNRI not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior
permission.

While CWI is the initial source for this software, a modified version
is made available by the Corporation for National Research Initiatives
(CNRI) at the Internet address ftp://ftp.python.org.

STICHTING MATHEMATISCH CENTRUM AND CNRI DISCLAIM ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL STICHTING MATHEMATISCH
CENTRUM OR CNRI BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.

******************************************************************/

static char table_a2b_base64[] = {
        -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
        -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
        -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,62, -1,-1,-1,63,
        52,53,54,55, 56,57,58,59, 60,61,-1,-1, -1, 0,-1,-1, /* Note PAD->0 */
        -1, 0, 1, 2,  3, 4, 5, 6,  7, 8, 9,10, 11,12,13,14,
        15,16,17,18, 19,20,21,22, 23,24,25,-1, -1,-1,-1,-1,
        -1,26,27,28, 29,30,31,32, 33,34,35,36, 37,38,39,40,
        41,42,43,44, 45,46,47,48, 49,50,51,-1, -1,-1,-1,-1
};

#define BASE64_PAD '='
#define BASE64_MAXBIN 57        /* Max binary chunk size (76 char line) */
#define BASE64_LINE_SZ 128      /* Buffer size for a single line. */    

static unsigned char table_b2a_base64[] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static xmlrpc_mem_block *
xmlrpc_base64_encode_internal (xmlrpc_env *env,
                               unsigned char *bin_data,
                               size_t bin_len,
                               int want_newlines)
{
    size_t chunk_start, chunk_left;
    unsigned char *ascii_data;
    int leftbits;
    unsigned char this_ch;
    unsigned int leftchar;
    xmlrpc_mem_block *output;
    unsigned char line_buffer[BASE64_LINE_SZ];

    /* Create a block to hold our lines when we finish them. */
    output = xmlrpc_mem_block_new(env, 0);
    XMLRPC_FAIL_IF_FAULT(env);

    /* Deal with empty data blocks gracefully. Yuck. */
    if (bin_len == 0) {
        if (want_newlines)
            XMLRPC_TYPED_MEM_BLOCK_APPEND(char, env, output, CRLF, 2);
        goto cleanup;
    }

    /* Process our binary data in line-sized chunks. */
    for (chunk_start=0; chunk_start < bin_len; chunk_start += BASE64_MAXBIN) {

        /* Set up our per-line state. */
        ascii_data = &line_buffer[0];
        chunk_left = bin_len - chunk_start;
        if (chunk_left > BASE64_MAXBIN)
            chunk_left = BASE64_MAXBIN;
        leftbits = 0;
        leftchar = 0;

        for(; chunk_left > 0; chunk_left--, bin_data++) {
            /* Shift the data into our buffer */
            leftchar = (leftchar << 8) | *bin_data;
            leftbits += 8;

            /* See if there are 6-bit groups ready */
            while (leftbits >= 6) {
                this_ch = (leftchar >> (leftbits-6)) & 0x3f;
                leftbits -= 6;
                *ascii_data++ = table_b2a_base64[this_ch];
            }
        }
        if (leftbits == 2) {
            *ascii_data++ = table_b2a_base64[(leftchar&3) << 4];
            *ascii_data++ = BASE64_PAD;
            *ascii_data++ = BASE64_PAD;
        } else if (leftbits == 4) {
            *ascii_data++ = table_b2a_base64[(leftchar&0xf) << 2];
            *ascii_data++ = BASE64_PAD;
        } 

        /* Append a courtesy CRLF. */
        if (want_newlines) {
            *ascii_data++ = CR;
            *ascii_data++ = LF;
        }
        
        /* Save our line. */
        XMLRPC_TYPED_MEM_BLOCK_APPEND(char, env, output, line_buffer,
                                      ascii_data - &line_buffer[0]);
        XMLRPC_FAIL_IF_FAULT(env);
    }

 cleanup:
    if (env->fault_occurred) {
        if (output)
            xmlrpc_mem_block_free(output);
        return NULL;
    }
    return output;
}


xmlrpc_mem_block *
xmlrpc_base64_encode (xmlrpc_env *env, unsigned char *bin_data, size_t bin_len)
{
    return xmlrpc_base64_encode_internal(env, bin_data, bin_len, 1);
}


xmlrpc_mem_block *
xmlrpc_base64_encode_without_newlines (xmlrpc_env *env,
                                       unsigned char *bin_data,
                                       size_t bin_len)
{
    return xmlrpc_base64_encode_internal(env, bin_data, bin_len, 0);
}


xmlrpc_mem_block *
xmlrpc_base64_decode (xmlrpc_env *env,
                      char *ascii_data,
                      size_t ascii_len)
{
    unsigned char *bin_data;
    int leftbits;
    unsigned char this_ch;
    unsigned int leftchar;
    size_t npad;
    size_t bin_len, buffer_size;
    xmlrpc_mem_block *output;

    /* Create a block to hold our chunks when we finish them.
    ** We overestimate the size now, and fix it later. */
    buffer_size = ((ascii_len+3)/4)*3;
    output = xmlrpc_mem_block_new(env, buffer_size);
    XMLRPC_FAIL_IF_FAULT(env);

    /* Set up our decoder state. */
    leftbits = 0;
    leftchar = 0;
    npad = 0;
    bin_data = XMLRPC_TYPED_MEM_BLOCK_CONTENTS(unsigned char, output);
    bin_len = 0;

    for( ; ascii_len > 0 ; ascii_len--, ascii_data++ ) {

        /* Skip some punctuation. */
        this_ch = (*ascii_data & 0x7f);
        if ( this_ch == '\r' || this_ch == '\n' || this_ch == ' ' )
            continue;
        if ( this_ch == BASE64_PAD )
            npad++;
        this_ch = table_a2b_base64[(*ascii_data) & 0x7f];

        /* XXX - We just throw away invalid characters. Is this right? */
        if ( this_ch == (unsigned char) -1 ) continue;

        /* Shift it in on the low end, and see if there's
        ** a byte ready for output. */
        leftchar = (leftchar << 6) | (this_ch);
        leftbits += 6;
        if ( leftbits >= 8 ) {
            leftbits -= 8;
            XMLRPC_ASSERT(bin_len < buffer_size);
            *bin_data++ = (leftchar >> leftbits) & 0xFF;
            leftchar &= ((1 << leftbits) - 1);
            bin_len++;
        }
    }

    /* Check that no bits are left. */
    if ( leftbits )
        XMLRPC_FAIL(env, XMLRPC_PARSE_ERROR, "Incorrect Base64 padding");

    /* Check to make sure we have a sane amount of padding. */
    if (npad > bin_len || npad > 2)
        XMLRPC_FAIL(env, XMLRPC_PARSE_ERROR, "Malformed Base64 data");

    /* Remove any padding and set the correct size. */
    bin_len -= npad;
    XMLRPC_TYPED_MEM_BLOCK_RESIZE(char, env, output, bin_len);
    XMLRPC_ASSERT(!env->fault_occurred);

 cleanup:
    if (env->fault_occurred) {
        if (output)
            xmlrpc_mem_block_free(output);
        return NULL;
    }
    return output;
}
