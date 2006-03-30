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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifdef WIN32
# define vsnprintf _vsnprintf
#endif

#include "xmlrpc.h"
#include "xmlrpc_int.h"

#ifdef EFENCE
                /* when looking for corruption don't allocate extra slop */
#define BLOCK_ALLOC_MIN (1)
#else
#define BLOCK_ALLOC_MIN (16)
#endif
#define BLOCK_ALLOC_MAX (128 * 1024 * 1024)

#define ERROR_BUFFER_SZ (256)


/*=========================================================================
**  Strings
**=======================================================================*/

void
xmlrpc_strfree(const char * const string) {
    free((void*)string);
}


/*=========================================================================
**  Assertions and Error Handling
**=========================================================================
**  Support code for XMLRPC_ASSERT and xmlrpc_env.
*/ 

void xmlrpc_assertion_failed (char* file, int line)
{
    fprintf(stderr, "%s:%d: assertion failed\n", file, line);
    abort();
}

static char* default_fault_string = "Not enough memory for error message";

void xmlrpc_env_init (xmlrpc_env* env)
{
    XMLRPC_ASSERT(env != NULL);

    env->fault_occurred = 0;
    env->fault_code     = 0;
    env->fault_string   = NULL;
}

void xmlrpc_env_clean (xmlrpc_env* env)
{
    XMLRPC_ASSERT(env != NULL);

    /* env->fault_string may be one of three things:
    **   1) a NULL pointer
    **   2) a pointer to the default_fault_string
    **   3) a pointer to a malloc'd fault string
    ** If we have case (3), we'll need to free it. */
    if (env->fault_string && env->fault_string != default_fault_string)
        free(env->fault_string);
    env->fault_string = XMLRPC_BAD_POINTER;
}



void 
xmlrpc_env_set_fault(xmlrpc_env * const env, 
                     int          const faultCode, 
                     const char * const faultDescription) {

    XMLRPC_ASSERT(env != NULL); 
    XMLRPC_ASSERT(faultDescription != NULL);

    /* Clean up any leftover pointers. */
    xmlrpc_env_clean(env);

    env->fault_occurred = 1;
    env->fault_code     = faultCode;

    /* Try to copy the fault string. If this fails, use a default. */
    env->fault_string = (char*) malloc(strlen(faultDescription) + 1);
    if (env->fault_string)
        strcpy(env->fault_string, faultDescription);
    else
        env->fault_string = default_fault_string;
}



void 
xmlrpc_env_set_fault_formatted (xmlrpc_env * const envP, 
                                int          const code,
                                const char * const format, 
                                ...) {
    va_list args;
    char buffer[ERROR_BUFFER_SZ];

    XMLRPC_ASSERT(envP != NULL);
    XMLRPC_ASSERT(format != NULL);

    /* Print our error message to the buffer. */
    va_start(args, format);
    vsnprintf(buffer, ERROR_BUFFER_SZ, format, args);
    va_end(args);

    /* vsnprintf is guaranteed to terminate the buffer, but we're paranoid. */
    buffer[ERROR_BUFFER_SZ - 1] = '\0';

    /* Set the fault. */
    xmlrpc_env_set_fault(envP, code, buffer);
}



void xmlrpc_fatal_error (char* file, int line, char* msg)
{
    fprintf(stderr, "%s:%d: %s\n", file, line, msg);
    exit(1);
}


/*=========================================================================
**  Resource Limits
**=========================================================================
*/ 

static size_t limits[XMLRPC_LAST_LIMIT_ID + 1] = {
    XMLRPC_NESTING_LIMIT_DEFAULT,
    XMLRPC_XML_SIZE_LIMIT_DEFAULT
};

void xmlrpc_limit_set (int limit_id, size_t value)
{
    XMLRPC_ASSERT(0 <= limit_id && limit_id <= XMLRPC_LAST_LIMIT_ID);
    limits[limit_id] = value;
}

size_t xmlrpc_limit_get (int limit_id)
{
    XMLRPC_ASSERT(0 <= limit_id && limit_id <= XMLRPC_LAST_LIMIT_ID);
    return limits[limit_id];
}


/*=========================================================================
**  xmlrpc_mem_block
**=========================================================================
**  We support resizable blocks of memory. We need these just about
**  everywhere.
*/

xmlrpc_mem_block * 
xmlrpc_mem_block_new(xmlrpc_env * const env, 
                     size_t       const size) {
    xmlrpc_mem_block* block;

    XMLRPC_ASSERT_ENV_OK(env);

    block = (xmlrpc_mem_block*) malloc(sizeof(xmlrpc_mem_block));
    XMLRPC_FAIL_IF_NULL(block, env, XMLRPC_INTERNAL_ERROR,
                        "Can't allocate memory block");

    xmlrpc_mem_block_init(env, block, size);
    XMLRPC_FAIL_IF_FAULT(env);

                     cleanup:
    if (env->fault_occurred) {
        if (block)
            free(block);
        return NULL;
    } else {
        return block;
    }
}

/* Destroy an existing xmlrpc_mem_block, and everything it contains. */
void xmlrpc_mem_block_free (xmlrpc_mem_block* block)
{
    XMLRPC_ASSERT(block != NULL);
    XMLRPC_ASSERT(block->_block != NULL);

    xmlrpc_mem_block_clean(block);
    free(block);
}

/* Initialize the contents of the provided xmlrpc_mem_block. */
void xmlrpc_mem_block_init (xmlrpc_env* env,
                            xmlrpc_mem_block* block,
                            size_t size)
{
    XMLRPC_ASSERT_ENV_OK(env);
    XMLRPC_ASSERT(block != NULL);

    block->_size = size;
    if (size < BLOCK_ALLOC_MIN)
        block->_allocated = BLOCK_ALLOC_MIN;
    else
        block->_allocated = size;

    block->_block = (void*) malloc(block->_allocated);
    if (!block->_block)
        xmlrpc_env_set_fault_formatted(
            env, XMLRPC_INTERNAL_ERROR,
            "Can't allocate %u-byte memory block",
            block->_allocated);
}

/* Deallocate the contents of the provided xmlrpc_mem_block, but not the
** block itself. */
void xmlrpc_mem_block_clean (xmlrpc_mem_block* block)
{
    XMLRPC_ASSERT(block != NULL);
    XMLRPC_ASSERT(block->_block != NULL);

    free(block->_block);
    block->_block = XMLRPC_BAD_POINTER;
}



/* Get the size of the xmlrpc_mem_block. */
size_t 
xmlrpc_mem_block_size(const xmlrpc_mem_block * const block) {

    XMLRPC_ASSERT(block != NULL);
    return block->_size;
}



/* Get the contents of the xmlrpc_mem_block. */
void * 
xmlrpc_mem_block_contents(const xmlrpc_mem_block * const block) {

    XMLRPC_ASSERT(block != NULL);
    return block->_block;
}



/* Resize an xmlrpc_mem_block, preserving as much of the contents as
** possible. */
void 
xmlrpc_mem_block_resize (xmlrpc_env *       const env,
                         xmlrpc_mem_block * const block,
                         size_t             const size) {

    size_t proposed_alloc;
    void* new_block;

    XMLRPC_ASSERT_ENV_OK(env);
    XMLRPC_ASSERT(block != NULL);

    /* Check to see if we already have enough space. Maybe we'll get lucky. */
    if (size <= block->_allocated) {
        block->_size = size;
        return;
    }

    /* Calculate a new allocation size. */
#ifdef EFENCE
    proposed_alloc = size;
#else
    proposed_alloc = block->_allocated;
    while (proposed_alloc < size && proposed_alloc <= BLOCK_ALLOC_MAX)
        proposed_alloc *= 2;
#endif /* DEBUG_MEM_ERRORS */

    if (proposed_alloc > BLOCK_ALLOC_MAX)
        XMLRPC_FAIL(env, XMLRPC_INTERNAL_ERROR, "Memory block too large");

    /* Allocate our new memory block. */
    new_block = (void*) malloc(proposed_alloc);
    XMLRPC_FAIL_IF_NULL(new_block, env, XMLRPC_INTERNAL_ERROR,
                        "Can't resize memory block");

    /* Copy over our data and update the xmlrpc_mem_block struct. */
    memcpy(new_block, block->_block, block->_size);
    free(block->_block);
    block->_block     = new_block;
    block->_size      = size;
    block->_allocated = proposed_alloc;

 cleanup:
    return;
}

/* Append data to an existing xmlrpc_mem_block. */
void 
xmlrpc_mem_block_append(xmlrpc_env *       const env,
                        xmlrpc_mem_block * const block,
                        void *             const data, 
                        size_t             const len)
{
    size_t size;

    XMLRPC_ASSERT_ENV_OK(env);
    XMLRPC_ASSERT(block != NULL);

    size = block->_size;
    xmlrpc_mem_block_resize(env, block, size + len);
    XMLRPC_FAIL_IF_FAULT(env);

    memcpy(((unsigned char*) block->_block) + size, data, len);

 cleanup:
    return;
}



void
xmlrpc_traceXml(const char * const label, 
                const char * const xml,
                unsigned int const xmlLength) {

    if (getenv("XMLRPC_TRACE_XML")) {
        unsigned int nonPrintableCount;
        unsigned int i;

        nonPrintableCount = 0;  /* Initial value */

        for (i = 0; i < xmlLength; ++i) {
            if (!isprint((int)(xml[i])) && xml[i] != '\n' && xml[i] != '\r')
                ++nonPrintableCount;
        }
        if (nonPrintableCount > 0)
            fprintf(stderr, "%s contains %u nonprintable characters.\n", 
                    label, nonPrintableCount);

        fprintf(stderr, "%s:\n %.*s\n", label, (int)xmlLength, xml);
    }
}
