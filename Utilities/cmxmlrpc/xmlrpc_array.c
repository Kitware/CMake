/* Copyright information is at the end of the file */

/*=========================================================================
**  XML-RPC Array Functions
**=========================================================================
*/

#include "xmlrpc_config.h"

#include <stddef.h>
#include <stdlib.h>

#include "xmlrpc.h"
#include "xmlrpc_int.h"


void
xmlrpc_abort_if_array_bad(xmlrpc_value * const arrayP) {

    if (arrayP == NULL)
        abort();
    else if (arrayP->_type != XMLRPC_TYPE_ARRAY)
        abort();
    else {
        size_t const arraySize =
            XMLRPC_MEMBLOCK_SIZE(xmlrpc_value*, &arrayP->_block);
        xmlrpc_value ** const contents = 
            XMLRPC_MEMBLOCK_CONTENTS(xmlrpc_value*, &arrayP->_block);
        
        if (contents == NULL)
            abort();
        else {
            unsigned int xmIndex;
            
            for (xmIndex = 0; xmIndex < arraySize; ++xmIndex) {
                xmlrpc_value * const itemP = contents[xmIndex];
                if (itemP == NULL)
                    abort();
                else if (itemP->_refcount < 1)
                    abort();
            }
        }
    }
}



void
xmlrpc_destroyArrayContents(xmlrpc_value * const arrayP) {
/*----------------------------------------------------------------------------
   Dispose of the contents of an array (but not the array value itself).
   The value is not valid after this.
-----------------------------------------------------------------------------*/
    size_t const arraySize =
        XMLRPC_MEMBLOCK_SIZE(xmlrpc_value*, &arrayP->_block);
    xmlrpc_value ** const contents = 
        XMLRPC_MEMBLOCK_CONTENTS(xmlrpc_value*, &arrayP->_block);

    unsigned int xmIndex;
    
    XMLRPC_ASSERT_ARRAY_OK(arrayP);

    /* Release our reference to each item in the array */
    for (xmIndex = 0; xmIndex < arraySize; ++xmIndex) {
        xmlrpc_value * const itemP = contents[xmIndex];
        xmlrpc_DECREF(itemP);
    }
    XMLRPC_MEMBLOCK_CLEAN(xmlrpc_value *, &arrayP->_block);
}



int 
xmlrpc_array_size(xmlrpc_env *         const env, 
                  const xmlrpc_value * const array) {

    int retval;

    /* Suppress a compiler warning about uninitialized variables. */
    retval = 0;

    XMLRPC_ASSERT_ENV_OK(env);
    XMLRPC_ASSERT_VALUE_OK(array);
    XMLRPC_TYPE_CHECK(env, array, XMLRPC_TYPE_ARRAY);

    retval = (int)XMLRPC_TYPED_MEM_BLOCK_SIZE(xmlrpc_value*, &array->_block);

                  cleanup:
    if (env->fault_occurred)
        return -1;
    else
        return retval;
}



void 
xmlrpc_array_append_item(xmlrpc_env *   const envP,
                         xmlrpc_value * const arrayP,
                         xmlrpc_value * const valueP) {

    XMLRPC_ASSERT_ENV_OK(envP);
    XMLRPC_ASSERT_VALUE_OK(arrayP);
    
    if (xmlrpc_value_type(arrayP) != XMLRPC_TYPE_ARRAY)
        xmlrpc_env_set_fault_formatted(
            envP, XMLRPC_TYPE_ERROR, "Value is not an array");
    else {
        size_t const size = 
            XMLRPC_MEMBLOCK_SIZE(xmlrpc_value *, &arrayP->_block);

        XMLRPC_MEMBLOCK_RESIZE(xmlrpc_value *, envP, &arrayP->_block, size+1);

        if (!envP->fault_occurred) {
            xmlrpc_value ** const contents =
                XMLRPC_MEMBLOCK_CONTENTS(xmlrpc_value*, &arrayP->_block);
            xmlrpc_INCREF(valueP);
            contents[size] = valueP;
        }
    }
}



void
xmlrpc_array_read_item(xmlrpc_env *         const envP,
                       const xmlrpc_value * const arrayP,
                       unsigned int         const xmIndex,
                       xmlrpc_value **      const valuePP) {

    XMLRPC_ASSERT_ENV_OK(envP);
    XMLRPC_ASSERT_VALUE_OK(arrayP);
    XMLRPC_ASSERT_PTR_OK(valuePP);

    if (arrayP->_type != XMLRPC_TYPE_ARRAY)
        xmlrpc_env_set_fault_formatted(
            envP, XMLRPC_TYPE_ERROR, "Attempt to read array item from "
            "a value that is not an array");
    else {
        xmlrpc_value ** const contents = 
            XMLRPC_MEMBLOCK_CONTENTS(xmlrpc_value *, &arrayP->_block);
        size_t const size = 
            XMLRPC_MEMBLOCK_SIZE(xmlrpc_value *, &arrayP->_block);

        if (xmIndex >= size)
            xmlrpc_env_set_fault_formatted(
                envP, XMLRPC_INDEX_ERROR, "Array index %u is beyond end "
                "of %u-item array", xmIndex, (unsigned int)size);
        else {
            *valuePP = contents[xmIndex];
            xmlrpc_INCREF(*valuePP);
        }
    }
}



xmlrpc_value * 
xmlrpc_array_get_item(xmlrpc_env *         const envP,
                      const xmlrpc_value * const arrayP,
                      int                  const xmIndex) {

    xmlrpc_value * valueP;

    if (xmIndex < 0)
        xmlrpc_env_set_fault_formatted(
            envP, XMLRPC_INDEX_ERROR, "Index %d is negative.");
    else {
        xmlrpc_array_read_item(envP, arrayP, xmIndex, &valueP);

        if (!envP->fault_occurred)
            xmlrpc_DECREF(valueP);
    }
    if (envP->fault_occurred)
        valueP = NULL;
        
    return valueP;
}



/* Copyright (C) 2001 by First Peer, Inc. All rights reserved.
** Copyright (C) 2001 by Eric Kidd. All rights reserved.
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
