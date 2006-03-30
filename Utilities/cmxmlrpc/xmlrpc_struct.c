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
#include <stdlib.h>
#include <string.h>

#include "xmlrpc.h"
#include "xmlrpc_int.h"

#define KEY_ERROR_BUFFER_SZ (32)


void
xmlrpc_destroyStruct(xmlrpc_value * const structP) {

    _struct_member * const members = 
        XMLRPC_MEMBLOCK_CONTENTS(_struct_member, &structP->_block);
    size_t const size = 
        XMLRPC_MEMBLOCK_SIZE(_struct_member, &structP->_block);

    unsigned int i;

    for (i = 0; i < size; ++i) {
        xmlrpc_DECREF(members[i].key);
        xmlrpc_DECREF(members[i].value);
    }
    XMLRPC_MEMBLOCK_CLEAN(_struct_member, &structP->_block);
}



/*=========================================================================
**  xmlrpc_struct_new
**=========================================================================
**  Create a new <struct> value. The corresponding destructor code
**  currently lives in xmlrpc_DECREF.
**
**  We store the individual members in an array of _struct_member. This
**  contains a key, a hash code, and a value. We look up keys by doing
**  a linear search of the hash codes.
*/

xmlrpc_value *
xmlrpc_struct_new(xmlrpc_env* env)
{
    xmlrpc_value *strct;
    int strct_valid;

    XMLRPC_ASSERT_ENV_OK(env);

    /* Set up error handling preconditions. */
    strct_valid = 0;

    /* Allocate and fill out an empty structure. */
    strct = (xmlrpc_value*) malloc(sizeof(xmlrpc_value));
    XMLRPC_FAIL_IF_NULL(strct, env, XMLRPC_INTERNAL_ERROR,
                        "Could not allocate memory for struct");
    strct->_refcount = 1;
    strct->_type = XMLRPC_TYPE_STRUCT;
    XMLRPC_MEMBLOCK_INIT(_struct_member, env, &strct->_block, 0);
    XMLRPC_FAIL_IF_FAULT(env);
    strct_valid = 1;

 cleanup:
    if (env->fault_occurred) {
        if (strct) {
            if (strct_valid)
                xmlrpc_DECREF(strct);
            else
                free(strct);
        }
        return NULL;
    }
    return strct;
}



/*=========================================================================
**  xmlrpc_struct_size
**=========================================================================
**  Return the number of key-value pairs contained in the struct. If the
**  value is not a struct, return -1 and set a fault.
*/

int 
xmlrpc_struct_size(xmlrpc_env* env, xmlrpc_value* strct)
{
    int retval;

    /* Suppress a compiler warning about uninitialized variables. */
    retval = 0;

    XMLRPC_ASSERT_ENV_OK(env);
    XMLRPC_ASSERT_VALUE_OK(strct);

    XMLRPC_TYPE_CHECK(env, strct, XMLRPC_TYPE_STRUCT);
    retval = (int)XMLRPC_MEMBLOCK_SIZE(_struct_member, &strct->_block);

 cleanup:
    if (env->fault_occurred)
        return -1;
    return retval;
}



/*=========================================================================
**  get_hash
**=========================================================================
**  A mindlessly simple hash function. Please feel free to write something
**  more clever if this produces bad results.
*/

static unsigned char 
get_hash(const char * const key, 
         size_t       const key_len) {

    unsigned char retval;
    size_t i;

    XMLRPC_ASSERT(key != NULL);
    
    retval = 0;
    for (i = 0; i < key_len; i++)
        retval += key[i];
    return retval;
}



/*=========================================================================
**  find_member
**=========================================================================
**  Get the index of the member with the specified key, or -1 if no such
**  member exists.
*/

static int 
find_member(xmlrpc_value * const strctP, 
            const char *   const key, 
            size_t         const key_len) {

    size_t size, i;
    unsigned char hash;
    _struct_member *contents;
    xmlrpc_value *keyval;
    char *keystr;
    size_t keystr_size;

    XMLRPC_ASSERT_VALUE_OK(strctP);
    XMLRPC_ASSERT(key != NULL);

    /* Look for our key. */
    hash = get_hash(key, key_len);
    size = XMLRPC_MEMBLOCK_SIZE(_struct_member, &strctP->_block);
    contents = XMLRPC_MEMBLOCK_CONTENTS(_struct_member, &strctP->_block);
    for (i = 0; i < size; i++) {
        if (contents[i].key_hash == hash) {
            keyval = contents[i].key;
            keystr = XMLRPC_MEMBLOCK_CONTENTS(char, &keyval->_block);
            keystr_size = XMLRPC_MEMBLOCK_SIZE(char, &keyval->_block)-1;
            if (key_len == keystr_size && memcmp(key, keystr, key_len) == 0)
                return (int)i;
        }   
    }
    return -1;
}



/*=========================================================================
**  xmlrpc_struct_has_key
**=========================================================================
*/

int 
xmlrpc_struct_has_key(xmlrpc_env *   const envP,
                      xmlrpc_value * const strctP,
                      const char *   const key) {

    XMLRPC_ASSERT(key != NULL);
    return xmlrpc_struct_has_key_n(envP, strctP, key, strlen(key));
}



int 
xmlrpc_struct_has_key_n(xmlrpc_env   * const envP,
                        xmlrpc_value * const strctP,
                        const char *   const key, 
                        size_t         const key_len) {
    int xmIndex;

    /* Suppress a compiler warning about uninitialized variables. */
    xmIndex = 0;

    XMLRPC_ASSERT_ENV_OK(envP);
    XMLRPC_ASSERT_VALUE_OK(strctP);
    XMLRPC_ASSERT(key != NULL);
    
    XMLRPC_TYPE_CHECK(envP, strctP, XMLRPC_TYPE_STRUCT);
    xmIndex = find_member(strctP, key, key_len);

 cleanup:
    if (envP->fault_occurred)
        return 0;
    return (xmIndex >= 0);
}



/*=========================================================================
**  xmlrpc_struct_find_value...
**=========================================================================
**  These functions look up a specified key value in a specified struct.
**  If it exists, they return the value of the struct member.  If not,
**  they return a NULL to indicate such.
*/

/* It would be a nice extension to be able to look up a key that is
   not a text string.
*/

void
xmlrpc_struct_find_value(xmlrpc_env *    const envP,
                         xmlrpc_value *  const structP,
                         const char *    const key,
                         xmlrpc_value ** const valuePP) {
/*----------------------------------------------------------------------------
  Given a key, retrieve a value from the struct.  If the key is not
  present, return NULL as *valuePP.
-----------------------------------------------------------------------------*/
    XMLRPC_ASSERT_ENV_OK(envP);
    XMLRPC_ASSERT_VALUE_OK(structP);
    XMLRPC_ASSERT_PTR_OK(key);
    
    if (structP->_type != XMLRPC_TYPE_STRUCT)
        xmlrpc_env_set_fault_formatted(
            envP, XMLRPC_TYPE_ERROR, "Value is not a struct.  It is type #%d",
            structP->_type);
    else {
        int xmIndex;

        /* Get our member index. */
        xmIndex = find_member(structP, key, strlen(key));
        if (xmIndex < 0)
            *valuePP = NULL;
        else {
            _struct_member * const members =
                XMLRPC_MEMBLOCK_CONTENTS(_struct_member, &structP->_block);
            *valuePP = members[xmIndex].value;
            
            XMLRPC_ASSERT_VALUE_OK(*valuePP);
            
            xmlrpc_INCREF(*valuePP);
        }
    }
}



void
xmlrpc_struct_find_value_v(xmlrpc_env *    const envP,
                           xmlrpc_value *  const structP,
                           xmlrpc_value *  const keyP,
                           xmlrpc_value ** const valuePP) {
/*----------------------------------------------------------------------------
  Given a key, retrieve a value from the struct.  If the key is not
  present, return NULL as *valuePP.
-----------------------------------------------------------------------------*/
    XMLRPC_ASSERT_ENV_OK(envP);
    XMLRPC_ASSERT_VALUE_OK(structP);
    XMLRPC_ASSERT_VALUE_OK(keyP);
    
    if (structP->_type != XMLRPC_TYPE_STRUCT)
        xmlrpc_env_set_fault_formatted(
            envP, XMLRPC_TYPE_ERROR, "Value is not a struct.  It is type #%d",
            structP->_type);
    else {
        if (keyP->_type != XMLRPC_TYPE_STRING)
            xmlrpc_env_set_fault_formatted(
                envP, XMLRPC_TYPE_ERROR, "Key value is not a string.  "
                "It is type #%d",
                keyP->_type);
        else {
            int xmIndex;

            /* Get our member index. */
            xmIndex = find_member(structP, 
                                XMLRPC_MEMBLOCK_CONTENTS(char, &keyP->_block),
                                XMLRPC_MEMBLOCK_SIZE(char, &keyP->_block)-1);
            if (xmIndex < 0)
                *valuePP = NULL;
            else {
                _struct_member * const members =
                    XMLRPC_MEMBLOCK_CONTENTS(_struct_member, &structP->_block);
                *valuePP = members[xmIndex].value;
                
                XMLRPC_ASSERT_VALUE_OK(*valuePP);
                
                xmlrpc_INCREF(*valuePP);
            }
        }
    }
}



/*=========================================================================
**  xmlrpc_struct_read_value...
**=========================================================================
**  These fail if no member with the specified key exists.
**  Otherwise, they are the same as xmlrpc_struct_find_value...
*/

void
xmlrpc_struct_read_value_v(xmlrpc_env *    const envP,
                           xmlrpc_value *  const structP,
                           xmlrpc_value *  const keyP,
                           xmlrpc_value ** const valuePP) {

    xmlrpc_struct_find_value_v(envP, structP, keyP, valuePP);

    if (!envP->fault_occurred) {
        if (*valuePP == NULL) {
            xmlrpc_env_set_fault_formatted(
                envP, XMLRPC_INDEX_ERROR, "No member of struct has key '%.*s'",
                XMLRPC_MEMBLOCK_SIZE(char, &keyP->_block),
                XMLRPC_MEMBLOCK_CONTENTS(char, &keyP->_block));
        }
    }
}



void
xmlrpc_struct_read_value(xmlrpc_env *    const envP,
                         xmlrpc_value *  const structP,
                         const char *    const key,
                         xmlrpc_value ** const valuePP) {

    xmlrpc_struct_find_value(envP, structP, key, valuePP);
    
    if (!envP->fault_occurred) {
        if (*valuePP == NULL) {
            xmlrpc_env_set_fault_formatted(
                envP, XMLRPC_INDEX_ERROR, "No member of struct has key '%s'",
                key);
            /* We should fix the error message to format the key for display */
        }
    }
}



/*=========================================================================
**  xmlrpc_struct_get_value...
**=========================================================================
**  These are for backward compatibility.  They used to be the only ones.
**  They're deprecated because they don't acquire a reference to the
**  value they return.
*/

xmlrpc_value * 
xmlrpc_struct_get_value_n(xmlrpc_env *   const envP,
                          xmlrpc_value * const structP,
                          const char *   const key, 
                          size_t         const keyLen) {

    xmlrpc_value * retval;
    xmlrpc_value * keyP;
    
    keyP = xmlrpc_build_value(envP, "s#", key, keyLen);
    if (!envP->fault_occurred) {
        xmlrpc_struct_find_value_v(envP, structP, keyP, &retval);

        if (!envP->fault_occurred) {
            if (retval == NULL) {
                xmlrpc_env_set_fault_formatted(
                    envP, XMLRPC_INDEX_ERROR, 
                    "No member of struct has key '%.*s'",
                    keyLen, key);
                /* We should fix the error message to format the key
                   for display */
            } else
                /* For backward compatibility.  */
                xmlrpc_DECREF(retval);
        }
        xmlrpc_DECREF(keyP);
    }
    return retval;
}



xmlrpc_value * 
xmlrpc_struct_get_value(xmlrpc_env *   const envP,
                        xmlrpc_value * const strctP,
                        const char *   const key) {

    XMLRPC_ASSERT(key != NULL);
    return xmlrpc_struct_get_value_n(envP, strctP, key, strlen(key));
}



/*=========================================================================
**  xmlrpc_struct_set_value
**=========================================================================
*/

void 
xmlrpc_struct_set_value(xmlrpc_env *   const envP,
                        xmlrpc_value * const strctP,
                        const char *   const key,
                        xmlrpc_value * const valueP) {

    XMLRPC_ASSERT(key != NULL);
    xmlrpc_struct_set_value_n(envP, strctP, key, strlen(key), valueP);
}



void 
xmlrpc_struct_set_value_n(xmlrpc_env *    const envP,
                          xmlrpc_value *  const strctP,
                          const char *    const key, 
                          size_t          const key_len,
                          xmlrpc_value *  const valueP) {

    xmlrpc_value *keyval;

    XMLRPC_ASSERT_ENV_OK(envP);
    XMLRPC_ASSERT(key != NULL);

    /* Set up error handling preconditions. */
    keyval = NULL;

    XMLRPC_TYPE_CHECK(envP, strctP, XMLRPC_TYPE_STRUCT);

    /* Build an xmlrpc_value from our string. */
    keyval = xmlrpc_build_value(envP, "s#", key, key_len);
    XMLRPC_FAIL_IF_FAULT(envP);

    /* Do the actual work. */
    xmlrpc_struct_set_value_v(envP, strctP, keyval, valueP);

 cleanup:
    if (keyval)
        xmlrpc_DECREF(keyval);
}



void 
xmlrpc_struct_set_value_v(xmlrpc_env *   const envP,
                          xmlrpc_value * const strctP,
                          xmlrpc_value * const keyvalP,
                          xmlrpc_value * const valueP) {

    char *key;
    size_t key_len;
    int xmIndex;
    _struct_member *members, *member, new_member;
    xmlrpc_value *old_value;

    XMLRPC_ASSERT_ENV_OK(envP);
    XMLRPC_ASSERT_VALUE_OK(strctP);
    XMLRPC_ASSERT_VALUE_OK(keyvalP);
    XMLRPC_ASSERT_VALUE_OK(valueP);

    XMLRPC_TYPE_CHECK(envP, strctP, XMLRPC_TYPE_STRUCT);
    XMLRPC_TYPE_CHECK(envP, keyvalP, XMLRPC_TYPE_STRING);

    key = XMLRPC_MEMBLOCK_CONTENTS(char, &keyvalP->_block);
    key_len = XMLRPC_MEMBLOCK_SIZE(char, &keyvalP->_block) - 1;
    xmIndex = find_member(strctP, key, key_len);

    if (xmIndex >= 0) {
        /* Change the value of an existing member. (But be careful--the
        ** original and new values might be the same object, so watch
        ** the order of INCREF and DECREF calls!) */
        members = XMLRPC_MEMBLOCK_CONTENTS(_struct_member, &strctP->_block);
        member = &members[xmIndex];

        /* Juggle our references. */
        old_value = member->value;
        member->value = valueP;
        xmlrpc_INCREF(member->value);
        xmlrpc_DECREF(old_value);
    } else {
        /* Add a new member. */
        new_member.key_hash = get_hash(key, key_len);
        new_member.key      = keyvalP;
        new_member.value    = valueP;
        XMLRPC_MEMBLOCK_APPEND(_struct_member, envP, &strctP->_block,
                               &new_member, 1);
        XMLRPC_FAIL_IF_FAULT(envP);
        xmlrpc_INCREF(keyvalP);
        xmlrpc_INCREF(valueP);
    }

cleanup:
    return;
}



/* Note that the order of keys and values is undefined, and may change
   when you modify the struct.
*/

void 
xmlrpc_struct_read_member(xmlrpc_env *    const envP,
                          xmlrpc_value *  const structP,
                          unsigned int    const xmIndex,
                          xmlrpc_value ** const keyvalP,
                          xmlrpc_value ** const valueP) {

    XMLRPC_ASSERT_ENV_OK(envP);
    XMLRPC_ASSERT_VALUE_OK(structP);
    XMLRPC_ASSERT_PTR_OK(keyvalP);
    XMLRPC_ASSERT_PTR_OK(valueP);

    if (structP->_type != XMLRPC_TYPE_STRUCT)
        xmlrpc_env_set_fault_formatted(
            envP, XMLRPC_TYPE_ERROR, "Attempt to read a struct member "
            "of something that is not a struct");
    else {
        _struct_member * const members =
            XMLRPC_MEMBLOCK_CONTENTS(_struct_member, &structP->_block);
        size_t const size = 
            XMLRPC_MEMBLOCK_SIZE(_struct_member, &structP->_block);

        if (xmIndex >= size)
            xmlrpc_env_set_fault_formatted(
                envP, XMLRPC_INDEX_ERROR, "Index %u is beyond the end of "
                "the %u-member structure", xmIndex, (unsigned int)size);
        else {
            _struct_member * const memberP = &members[xmIndex];
            *keyvalP = memberP->key;
            xmlrpc_INCREF(memberP->key);
            *valueP = memberP->value;
            xmlrpc_INCREF(memberP->value);
        }
    }
}



void 
xmlrpc_struct_get_key_and_value(xmlrpc_env *    const envP,
                                xmlrpc_value *  const structP,
                                int             const xmIndex,
                                xmlrpc_value ** const keyvalP,
                                xmlrpc_value ** const valueP) {
/*----------------------------------------------------------------------------
   Same as xmlrpc_struct_read_member(), except doesn't take a reference
   to the returned value.

   This is obsolete.
-----------------------------------------------------------------------------*/
    XMLRPC_ASSERT_ENV_OK(envP);
    XMLRPC_ASSERT_VALUE_OK(structP);
    XMLRPC_ASSERT_PTR_OK(keyvalP);
    XMLRPC_ASSERT_PTR_OK(valueP);

    if (xmIndex < 0)
        xmlrpc_env_set_fault_formatted(
            envP, XMLRPC_INDEX_ERROR, "Index %d is negative.");
    else {
        xmlrpc_struct_read_member(envP, structP, xmIndex, keyvalP, valueP);
        if (!envP->fault_occurred) {
            xmlrpc_DECREF(*keyvalP);
            xmlrpc_DECREF(*valueP);
        }
    }
    if (envP->fault_occurred) {
        *keyvalP = NULL;
        *valueP = NULL;
    }
}
