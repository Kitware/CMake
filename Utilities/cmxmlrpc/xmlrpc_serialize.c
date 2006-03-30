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
** SUCH DAMAGE. */

#include "xmlrpc_config.h"

#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "xmlrpc.h"
#include "xmlrpc_int.h"

#define CRLF "\015\012"
#define SMALL_BUFFER_SZ (128)
#define XML_PROLOGUE "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"CRLF


/*=========================================================================
**  format_out
**=========================================================================
**  A lightweight print routine for use with various serialization
**  functions. Only use this routine for printing small objects--it uses
**  a fixed-size internal buffer and returns an error on overflow.
**  In particular, do NOT use this routine to print XML-RPC string values!
*/

static void 
format_out(xmlrpc_env *env,
           xmlrpc_mem_block *output,
           char *format_string,
           ...) {

    va_list args;
    char buffer[SMALL_BUFFER_SZ];
    int count;

    XMLRPC_ASSERT_ENV_OK(env);

    va_start(args, format_string);

    /* We assume that this function is present and works correctly. Right. */
    count = vsnprintf(buffer, SMALL_BUFFER_SZ, format_string, args);

    /* Old C libraries return -1 if vsnprintf overflows its buffer.
    ** New C libraries return the number of characters which *would* have
    ** been printed if the error did not occur. This is impressively vile.
    ** Thank the C99 committee for this bright idea. But wait! We also
    ** need to keep track of the trailing NULL. */
    if (count < 0 || count >= (SMALL_BUFFER_SZ - 1))
    XMLRPC_FAIL(env, XMLRPC_INTERNAL_ERROR,
                "format_out overflowed internal buffer");

    /* Append our new data to our output. */
    XMLRPC_TYPED_MEM_BLOCK_APPEND(char, env, output, buffer, count);
    XMLRPC_FAIL_IF_FAULT(env);

cleanup:
    va_end(args);
}


/*=========================================================================
**  Warnings About Invalid UTF-8
**=========================================================================
**  We claim to send UTF-8 data to the network.  But we rely on application
**  programs to pass us correctly-formed UTF-8 data, which is very naive
**  and optimistic.
**
**  In debudding mode, we call this routine to issue dire-sounding
**  warnings.  For the sake of safety, this routine never exits the
**  program or does anything else drastic.
**
**  This routine almost certainly slows down our output.
*/

#if !defined NDEBUG && defined HAVE_UNICODE_WCHAR

static void 
sanity_check_utf8(const char * const str,
                  size_t       const len) {

    xmlrpc_env env;

    xmlrpc_env_init(&env);
    xmlrpc_validate_utf8(&env, str, len);
    if (env.fault_occurred)
        fprintf(stderr, "*** xmlrpc-c WARNING ***: %s (%s)\n",
                "Application sending corrupted UTF-8 data to network",
                env.fault_string);
    xmlrpc_env_clean(&env);
}
#endif



/*=========================================================================
**  Escaping Strings
**=========================================================================
*/

static xmlrpc_mem_block * 
escape_string(xmlrpc_env * const env, 
              const char * const str,
              size_t       const len) {

    xmlrpc_mem_block *retval;
    size_t i, needed;
    char *out;

    XMLRPC_ASSERT_ENV_OK(env);
    XMLRPC_ASSERT(str != NULL);

    /* Sanity-check this string before we print it. */
#if !defined NDEBUG && defined HAVE_UNICODE_WCHAR
    sanity_check_utf8(str, len);
#endif    

    /* Calculate the amount of space we'll need. */
    needed = 0;
    for (i = 0; i < len; i++) {
        if (str[i] == '<')
            needed += 4; /* &lt; */
        else if (str[i] == '>')
            needed += 4; /* &gt; */
        else if (str[i] == '&')
            needed += 5; /* &amp; */
        else
            needed++;
    }

    /* Allocate our memory block. */
    retval = XMLRPC_TYPED_MEM_BLOCK_NEW(char, env, needed);
    XMLRPC_FAIL_IF_FAULT(env);

    /* Copy over the newly-allocated data. */
    out = XMLRPC_TYPED_MEM_BLOCK_CONTENTS(char, retval);
    for (i = 0; i < len; i++) {
        if (str[i] == '<') {
            *out++ = '&';
            *out++ = 'l';
            *out++ = 't';
            *out++ = ';';
        } else if (str[i] == '>') {
            *out++ = '&';
            *out++ = 'g';
            *out++ = 't';
            *out++ = ';';
        } else if (str[i] == '&') {
            *out++ = '&';
            *out++ = 'a';
            *out++ = 'm';
            *out++ = 'p';
            *out++ = ';';
        } else {
            *out++ = str[i];
        }
    }

 cleanup:
    if (env->fault_occurred) {
        if (retval)
            XMLRPC_TYPED_MEM_BLOCK_FREE(char, retval);
        retval = NULL;
    }
    return retval;
}



static xmlrpc_mem_block* 
escape_block (xmlrpc_env *env,
              xmlrpc_mem_block *block) {

    XMLRPC_ASSERT_ENV_OK(env);
    XMLRPC_ASSERT(block != NULL);

    return escape_string(env,
                         XMLRPC_TYPED_MEM_BLOCK_CONTENTS(char, block),
                         XMLRPC_TYPED_MEM_BLOCK_SIZE(char, block));
}



/*=========================================================================
**  xmlrpc_serialize_string_data
**=========================================================================
**  Escape and print the contents of a string.
*/                

static void 
xmlrpc_serialize_string_data(xmlrpc_env *env,
                             xmlrpc_mem_block *output,
                             xmlrpc_value *string) {

    xmlrpc_mem_block *escaped;
    char *contents;
    size_t size;

    /* Since this routine can only be called internally, we only need
    ** an assertion here, not a runtime type check.
    ** XXX - Temporarily disabled because we're using this code to
    ** print <dateTime.iso8601> values as well. */
    /* XMLRPC_ASSERT(string->_type == XMLRPC_TYPE_STRING); */

    /* Escape any '&' and '<' characters in the string. */
    escaped = escape_block(env, &string->_block);
    XMLRPC_FAIL_IF_FAULT(env);
    contents = XMLRPC_TYPED_MEM_BLOCK_CONTENTS(char, escaped);
    size = XMLRPC_TYPED_MEM_BLOCK_SIZE(char, escaped) - 1;
    
    /* Print the string. */
    XMLRPC_TYPED_MEM_BLOCK_APPEND(char, env, output, contents, size);
    XMLRPC_FAIL_IF_FAULT(env);
    
 cleanup:
    if (escaped)
        XMLRPC_TYPED_MEM_BLOCK_FREE(char, escaped);
}



/*=========================================================================
**  xmlrpc_serialize_base64_data
**=========================================================================
**  Print the contents of a memory block as well-formed Base64 data.
*/                

static void 
xmlrpc_serialize_base64_data (xmlrpc_env *env,
                              xmlrpc_mem_block *output,
                              unsigned char* data, size_t len) {

    xmlrpc_mem_block *encoded;
    unsigned char *contents;
    size_t size;

    /* Encode the data. */
    encoded = xmlrpc_base64_encode_without_newlines(env, data, len);
    XMLRPC_FAIL_IF_FAULT(env);
    contents = XMLRPC_TYPED_MEM_BLOCK_CONTENTS(unsigned char, encoded);
    size = XMLRPC_TYPED_MEM_BLOCK_SIZE(unsigned char, encoded);

    /* Print the data. */
    XMLRPC_TYPED_MEM_BLOCK_APPEND(char, env, output, contents, size);
    XMLRPC_FAIL_IF_FAULT(env);

 cleanup:
    if (encoded)
        XMLRPC_TYPED_MEM_BLOCK_FREE(char, encoded);
}



/*=========================================================================
**  xmlrpc_serialize_struct
**=========================================================================
**  Dump the contents of a struct.
*/                

static void 
xmlrpc_serialize_struct(xmlrpc_env *env,
                        xmlrpc_mem_block *output,
                        xmlrpc_value *strct) {

    size_t size;
    size_t i;
    xmlrpc_value *key, *value;

    format_out(env, output, "<struct>"CRLF);
    XMLRPC_FAIL_IF_FAULT(env);

    size = xmlrpc_struct_size(env, strct);
    XMLRPC_FAIL_IF_FAULT(env);
    for (i = 0; i < size; i++) {
        xmlrpc_struct_get_key_and_value(env, strct, (int)i, &key, &value);
        XMLRPC_FAIL_IF_FAULT(env);
        format_out(env, output, "<member><name>");
        XMLRPC_FAIL_IF_FAULT(env);
        xmlrpc_serialize_string_data(env, output, key);
        XMLRPC_FAIL_IF_FAULT(env);
        format_out(env, output, "</name>"CRLF);
        XMLRPC_FAIL_IF_FAULT(env);
        xmlrpc_serialize_value(env, output, value);
        XMLRPC_FAIL_IF_FAULT(env);
        format_out(env, output, "</member>"CRLF);
        XMLRPC_FAIL_IF_FAULT(env);
    }

    format_out(env, output, "</struct>");
    XMLRPC_FAIL_IF_FAULT(env);

cleanup:
    return;
}



/*=========================================================================
**  xmlrpc_serialize_value
**=========================================================================
**  Dump a value in the appropriate fashion.
*/                

void 
xmlrpc_serialize_value(xmlrpc_env *env,
                       xmlrpc_mem_block *output,
                       xmlrpc_value *value) {

    xmlrpc_value *item;
    size_t size;
    unsigned char* contents;
    size_t i;

    XMLRPC_ASSERT_ENV_OK(env);
    XMLRPC_ASSERT(output != NULL);
    XMLRPC_ASSERT_VALUE_OK(value);

    /* Print our ubiquitous header. */
    format_out(env, output, "<value>");
    XMLRPC_FAIL_IF_FAULT(env);

    switch (value->_type) {

    case XMLRPC_TYPE_INT:
        /* XXX - We assume that '%i' is the appropriate format specifier
        ** for an xmlrpc_int32 value. We should add some test cases to
        ** make sure this works. */
        format_out(env, output, "<i4>%i</i4>", value->_value.i);
        break;

    case XMLRPC_TYPE_BOOL:
        /* XXX - We assume that '%i' is the appropriate format specifier
        ** for an xmlrpc_bool value. */
        format_out(env, output, "<boolean>%i</boolean>",
                   (value->_value.b) ? 1 : 0);
        break;

    case XMLRPC_TYPE_DOUBLE:
        /* We must output a number of the form [+-]?\d*.\d*. */
        format_out(env, output, "<double>%.17g</double>", value->_value.d);
        break;

    case XMLRPC_TYPE_STRING:
        format_out(env, output, "<string>");
        XMLRPC_FAIL_IF_FAULT(env);
        xmlrpc_serialize_string_data(env, output, value);
        XMLRPC_FAIL_IF_FAULT(env);
        format_out(env, output, "</string>");
        break;

    case XMLRPC_TYPE_ARRAY:
        format_out(env, output, "<array><data>"CRLF);
        XMLRPC_FAIL_IF_FAULT(env);

        /* Serialize each item. */
        size = xmlrpc_array_size(env, value);
        XMLRPC_FAIL_IF_FAULT(env);
        for (i = 0; i < size; i++) {
            item = xmlrpc_array_get_item(env, value, (int)i);
            XMLRPC_FAIL_IF_FAULT(env);
            xmlrpc_serialize_value(env, output, item);
            XMLRPC_FAIL_IF_FAULT(env);
            format_out(env, output, CRLF);
            XMLRPC_FAIL_IF_FAULT(env);
        }

        format_out(env, output, "</data></array>");
        break;

    case XMLRPC_TYPE_STRUCT:
        xmlrpc_serialize_struct(env, output, value);
        break;

    case XMLRPC_TYPE_BASE64:
        format_out(env, output, "<base64>"CRLF);
        XMLRPC_FAIL_IF_FAULT(env);
        contents = XMLRPC_TYPED_MEM_BLOCK_CONTENTS(unsigned char,
                                                   &value->_block);
        size = XMLRPC_TYPED_MEM_BLOCK_SIZE(unsigned char, &value->_block);
        xmlrpc_serialize_base64_data(env, output, contents, size);
        XMLRPC_FAIL_IF_FAULT(env);
        format_out(env, output, "</base64>");
        break;      

    case XMLRPC_TYPE_DATETIME:
        format_out(env, output, "<dateTime.iso8601>");
        XMLRPC_FAIL_IF_FAULT(env);
        xmlrpc_serialize_string_data(env, output, value);
        XMLRPC_FAIL_IF_FAULT(env);
        format_out(env, output, "</dateTime.iso8601>");
        break;

    case XMLRPC_TYPE_C_PTR:
        xmlrpc_env_set_fault_formatted(
            env, XMLRPC_INTERNAL_ERROR,
            "Tried to serialize a C pointer value.");
        break;

    case XMLRPC_TYPE_DEAD:
        xmlrpc_env_set_fault_formatted(
            env, XMLRPC_INTERNAL_ERROR,
            "Tried to serialize a deaad value.");
        break;

    default:
        xmlrpc_env_set_fault_formatted(
            env, XMLRPC_INTERNAL_ERROR,
            "Invalid xmlrpc_value type: %d", value->_type);
    }
    XMLRPC_FAIL_IF_FAULT(env);

    /* Print our ubiquitous footer. */
    format_out(env, output, "</value>");
    XMLRPC_FAIL_IF_FAULT(env);

 cleanup:
    return;
}



/*=========================================================================
**  xmlrpc_serialize_params
**=========================================================================
**  Serialize a list as a set of parameters.
*/                

void 
xmlrpc_serialize_params(xmlrpc_env *env,
                        xmlrpc_mem_block *output,
                        xmlrpc_value *param_array) {

    size_t size, i;
    xmlrpc_value *item;

    XMLRPC_ASSERT_ENV_OK(env);
    XMLRPC_ASSERT(output != NULL);
    XMLRPC_ASSERT_VALUE_OK(param_array);

    format_out(env, output, "<params>"CRLF);
    XMLRPC_FAIL_IF_FAULT(env);

    /* Dump each parameter. */
    size = xmlrpc_array_size(env, param_array);
    XMLRPC_FAIL_IF_FAULT(env);
    for (i = 0; i < size; i++) {
        format_out(env, output, "<param>");
        XMLRPC_FAIL_IF_FAULT(env);
        item = xmlrpc_array_get_item(env, param_array, (int)i);
        XMLRPC_FAIL_IF_FAULT(env);
        xmlrpc_serialize_value(env, output, item);
        XMLRPC_FAIL_IF_FAULT(env);
        format_out(env, output, "</param>"CRLF);
        XMLRPC_FAIL_IF_FAULT(env);
    }

    format_out(env, output, "</params>"CRLF);
    XMLRPC_FAIL_IF_FAULT(env);

 cleanup:    
    return;
}



/*=========================================================================
**  xmlrpc_serialize_call
**=========================================================================
**  Serialize an XML-RPC call.
*/                

void 
xmlrpc_serialize_call(xmlrpc_env *       const env,
                      xmlrpc_mem_block * const output,
                      const char *       const method_name,
                      xmlrpc_value *     const param_array) {

    xmlrpc_mem_block *escaped;
    char *contents;
    size_t size;

    XMLRPC_ASSERT_ENV_OK(env);
    XMLRPC_ASSERT(output != NULL);
    XMLRPC_ASSERT(method_name != NULL);
    XMLRPC_ASSERT_VALUE_OK(param_array);
    
    /* Set up our error-handling preconditions. */
    escaped = NULL;

    /* Dump our header. */
    format_out(env, output, XML_PROLOGUE);
    XMLRPC_FAIL_IF_FAULT(env);
    format_out(env, output, "<methodCall>"CRLF"<methodName>");
    XMLRPC_FAIL_IF_FAULT(env);

    /* Dump the method name. */
    escaped = escape_string(env, method_name, strlen(method_name));
    XMLRPC_FAIL_IF_FAULT(env);
    contents = XMLRPC_TYPED_MEM_BLOCK_CONTENTS(char, escaped);
    size = XMLRPC_TYPED_MEM_BLOCK_SIZE(char, escaped);
    XMLRPC_TYPED_MEM_BLOCK_APPEND(char, env, output, contents, size);
    XMLRPC_FAIL_IF_FAULT(env);    

    /* Dump our parameters and footer. */
    format_out(env, output, "</methodName>"CRLF);
    XMLRPC_FAIL_IF_FAULT(env);
    xmlrpc_serialize_params(env, output, param_array);
    XMLRPC_FAIL_IF_FAULT(env);
    format_out(env, output, "</methodCall>"CRLF);
    XMLRPC_FAIL_IF_FAULT(env);

 cleanup:
    if (escaped)
        xmlrpc_mem_block_free(escaped);
}



/*=========================================================================
**  xmlrpc_serialize_response
**=========================================================================
**  Serialize the (non-fault) response to an XML-RPC call.
*/                

void 
xmlrpc_serialize_response (xmlrpc_env *env,
                           xmlrpc_mem_block *output,
                           xmlrpc_value *value) {

    XMLRPC_ASSERT_ENV_OK(env);
    XMLRPC_ASSERT(output != NULL);
    XMLRPC_ASSERT_VALUE_OK(value);

    format_out(env, output, XML_PROLOGUE);
    XMLRPC_FAIL_IF_FAULT(env);
    format_out(env, output, "<methodResponse>"CRLF"<params>"CRLF"<param>");
    XMLRPC_FAIL_IF_FAULT(env);

    xmlrpc_serialize_value(env, output, value);
    XMLRPC_FAIL_IF_FAULT(env);

    format_out(env, output,
               "</param>"CRLF"</params>"CRLF"</methodResponse>"CRLF);
    XMLRPC_FAIL_IF_FAULT(env);
    
 cleanup:
    return;
}



/*=========================================================================
**  xmlrpc_serialize_fault
**=========================================================================
**  Serialize an XML-RPC fault.
**
**  If this function fails, it will set up the first env argument. You'll
**  need to take some other drastic action to produce a serialized fault
**  of your own. (This function should only fail in an out-of-memory
**  situation, AFAIK.)
*/                

void 
xmlrpc_serialize_fault(xmlrpc_env *env,
                       xmlrpc_mem_block *output,
                       xmlrpc_env *fault) {

    xmlrpc_value *strct;

    XMLRPC_ASSERT_ENV_OK(env);
    XMLRPC_ASSERT(output != NULL);
    XMLRPC_ASSERT(fault != NULL && fault->fault_occurred);

    /* Build a fault structure. */
    strct = xmlrpc_build_value(env, "{s:i,s:s}",
                   "faultCode", (xmlrpc_int32) fault->fault_code,
                   "faultString", fault->fault_string);
    XMLRPC_FAIL_IF_FAULT(env);

    /* Output our header. */
    format_out(env, output, XML_PROLOGUE);
    XMLRPC_FAIL_IF_FAULT(env);
    format_out(env, output, "<methodResponse>"CRLF"<fault>"CRLF);
    XMLRPC_FAIL_IF_FAULT(env);

    /* Serialize our fault structure. */
    xmlrpc_serialize_value(env, output, strct);
    XMLRPC_FAIL_IF_FAULT(env);

    /* Output our footer. */
    format_out(env, output, CRLF"</fault>"CRLF"</methodResponse>"CRLF);
    XMLRPC_FAIL_IF_FAULT(env);

 cleanup:
    if (strct)
        xmlrpc_DECREF(strct);
}
