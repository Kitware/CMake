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
#include <cm_expat.h>

#include "xmlrpc.h"
#include "xmlrpc_int.h"
#include "xmlrpc_xmlparser.h"

/* Define the contents of our internal structure. */
struct _xml_element {
    struct _xml_element *_parent;
    char *_name;
    xmlrpc_mem_block _cdata;    /* char */
    xmlrpc_mem_block _children; /* xml_element* */
};

/* Check that we're using expat in UTF-8 mode, not wchar_t mode.
** If you need to use expat in wchar_t mode, write a subroutine to
** copy a wchar_t string to a char string & return an error for
** any non-ASCII characters. Then call this subroutine on all
** XML_Char strings passed to our event handlers before using the
** data. */
/* #if sizeof(char) != sizeof(XML_Char)
** #error expat must define XML_Char to be a regular char. 
** #endif
*/

#define XMLRPC_ASSERT_ELEM_OK(elem) \
    XMLRPC_ASSERT((elem) != NULL && (elem)->_name != XMLRPC_BAD_POINTER)


/*=========================================================================
**  xml_element_new
**=========================================================================
**  Create a new xml_element. This routine isn't exported, because the
**  arguments are implementation-dependent.
*/

static xml_element *xml_element_new (xmlrpc_env *env, char *name)
{
    xml_element *retval;
    int name_valid, cdata_valid, children_valid;

    XMLRPC_ASSERT_ENV_OK(env);
    XMLRPC_ASSERT(name != NULL);

    /* Set up our error-handling preconditions. */
    name_valid = cdata_valid = children_valid = 0;

    /* Allocate our xml_element structure. */
    retval = (xml_element*) malloc(sizeof(xml_element));
    XMLRPC_FAIL_IF_NULL(retval, env, XMLRPC_INTERNAL_ERROR,
                        "Couldn't allocate memory for XML element");

    /* Set our parent field to NULL. */
    retval->_parent = NULL;
    
    /* Copy over the element name. */
    retval->_name = (char*) malloc(strlen(name) + 1);
    XMLRPC_FAIL_IF_NULL(retval->_name, env, XMLRPC_INTERNAL_ERROR,
                        "Couldn't allocate memory for XML element");
    name_valid = 1;
    strcpy(retval->_name, name);

    /* Initialize a block to hold our CDATA. */
    XMLRPC_TYPED_MEM_BLOCK_INIT(char, env, &retval->_cdata, 0);
    XMLRPC_FAIL_IF_FAULT(env);
    cdata_valid = 1;

    /* Initialize a block to hold our child elements. */
    XMLRPC_TYPED_MEM_BLOCK_INIT(xml_element*, env, &retval->_children, 0);
    XMLRPC_FAIL_IF_FAULT(env);
    children_valid = 1;

 cleanup:
    if (env->fault_occurred) {
        if (retval) {
            if (name_valid)
                free(retval->_name);
            if (cdata_valid)
                xmlrpc_mem_block_clean(&retval->_cdata);
            if (children_valid)
                xmlrpc_mem_block_clean(&retval->_children);
            free(retval);
        }
        return NULL;
    } else {
        return retval;
    }
}


/*=========================================================================
**  xml_element_free
**=========================================================================
**  Blow away an existing element & all of its child elements.
*/

void xml_element_free (xml_element *elem)
{
    xmlrpc_mem_block *children;
    int size, i;
    xml_element **contents;

    XMLRPC_ASSERT_ELEM_OK(elem);

    free(elem->_name);
    elem->_name = XMLRPC_BAD_POINTER;
    xmlrpc_mem_block_clean(&elem->_cdata);

    /* Deallocate all of our children recursively. */
    children = &elem->_children;
    contents = XMLRPC_TYPED_MEM_BLOCK_CONTENTS(xml_element*, children);
    size = (int)XMLRPC_TYPED_MEM_BLOCK_SIZE(xml_element*, children);
    for (i = 0; i < size; i++)
        xml_element_free(contents[i]);

    xmlrpc_mem_block_clean(&elem->_children);
    free(elem);
}


/*=========================================================================
**  Miscellaneous Accessors
**=========================================================================
**  Return the fields of the xml_element. See the header for more
**  documentation on each function works.
*/

char *xml_element_name (xml_element *elem)
{
    XMLRPC_ASSERT_ELEM_OK(elem);
    return elem->_name;
}

/* The result of this function is NOT VALID until the end_element handler
** has been called! */
size_t xml_element_cdata_size (xml_element *elem)
{
    XMLRPC_ASSERT_ELEM_OK(elem);
    return XMLRPC_TYPED_MEM_BLOCK_SIZE(char, &elem->_cdata) - 1;
}

char *xml_element_cdata (xml_element *elem)
{
    XMLRPC_ASSERT_ELEM_OK(elem);
    return XMLRPC_TYPED_MEM_BLOCK_CONTENTS(char, &elem->_cdata);
}

size_t xml_element_children_size (xml_element *elem)
{
    XMLRPC_ASSERT_ELEM_OK(elem);
    return XMLRPC_TYPED_MEM_BLOCK_SIZE(xml_element*, &elem->_children);
}

xml_element **xml_element_children (xml_element *elem)
{
    XMLRPC_ASSERT_ELEM_OK(elem);
    return XMLRPC_TYPED_MEM_BLOCK_CONTENTS(xml_element*, &elem->_children);
}


/*=========================================================================
**  Internal xml_element Utility Functions
**=========================================================================
*/

static void xml_element_append_cdata (xmlrpc_env *env,
                                      xml_element *elem,
                                      char *cdata,
                                      size_t size)
{
    XMLRPC_ASSERT_ENV_OK(env);
    XMLRPC_ASSERT_ELEM_OK(elem);    

    XMLRPC_TYPED_MEM_BLOCK_APPEND(char, env, &elem->_cdata, cdata, size);
}

/* Whether or not this function succeeds, it takes ownership of the 'child'
** argument.
** WARNING - This is the exact opposite of the usual memory ownership
** rules for xmlrpc_value! So please pay attention. */
static void xml_element_append_child (xmlrpc_env *env,
                                      xml_element *elem,
                                      xml_element *child)
{
    XMLRPC_ASSERT_ENV_OK(env);
    XMLRPC_ASSERT_ELEM_OK(elem);
    XMLRPC_ASSERT_ELEM_OK(child);
    XMLRPC_ASSERT(child->_parent == NULL);

    XMLRPC_TYPED_MEM_BLOCK_APPEND(xml_element*, env, &elem->_children,
                                  &child, 1);
    if (!env->fault_occurred)
        child->_parent = elem;
    else
        xml_element_free(child);
}


/*=========================================================================
**  Our parse context. We pass this around as expat user data.
**=========================================================================
*/

typedef struct {
    xmlrpc_env *env;
    xml_element *root;
    xml_element *current;
} parse_context;


/*=========================================================================
**  Expat Event Handler Functions
**=========================================================================
*/

static void
start_element (void *user_data, XML_Char *name, XML_Char **atts ATTR_UNUSED)
{
    parse_context *context;
    xml_element *elem, *new_current;

    XMLRPC_ASSERT(user_data != NULL && name != NULL);

    /* Get our context and see if an error has already occured. */
    context = (parse_context*) user_data;
    if (!context->env->fault_occurred) {

        /* Build a new element. */
        elem = xml_element_new(context->env, name);
        XMLRPC_FAIL_IF_FAULT(context->env);

        /* Insert it in the appropriate place. */
        if (!context->root) {
            context->root = elem;
            context->current = elem;
            elem = NULL;
        } else {
            XMLRPC_ASSERT(context->current != NULL);

            /* (We need to watch our error handling invariants very carefully
            ** here. Read the docs for xml_element_append_child. */
            new_current = elem;
            xml_element_append_child(context->env, context->current, elem);
            elem = NULL;
            XMLRPC_FAIL_IF_FAULT(context->env);
            context->current = new_current;
        }

 cleanup:
        if (elem)
            xml_element_free(elem);
    }
}

static void end_element (void *user_data, XML_Char *name)
{
    parse_context *context;

    XMLRPC_ASSERT(user_data != NULL && name != NULL);

    /* Get our context and see if an error has already occured. */
    context = (parse_context*) user_data;
    if (!context->env->fault_occurred) {

        /* XXX - I think expat enforces these facts, but I want to be sure.
        ** If one of these assertion ever fails, it should be replaced by a
        ** non-assertion runtime error check. */
        XMLRPC_ASSERT(strcmp(name, context->current->_name) == 0);
        XMLRPC_ASSERT(context->current->_parent != NULL ||
                      context->current == context->root);

        /* Add a trailing '\0' to our cdata. */
        xml_element_append_cdata(context->env, context->current, "\0", 1);
        XMLRPC_FAIL_IF_FAULT(context->env);     

        /* Pop our "stack" of elements. */
        context->current = context->current->_parent;

 cleanup:
        return;
    }
}

static void character_data (void *user_data, XML_Char *s, int len)
{
    parse_context *context;

    XMLRPC_ASSERT(user_data != NULL && s != NULL && len >= 0);

    /* Get our context and see if an error has already occured. */
    context = (parse_context*) user_data;
    if (!context->env->fault_occurred) {

        XMLRPC_ASSERT(context->current != NULL);
        
        xml_element_append_cdata(context->env, context->current, s, len);
        XMLRPC_FAIL_IF_FAULT(context->env);

 cleanup:
        return;
    }
}


/*=========================================================================
**  Expat Driver
**=========================================================================
**  XXX - We should allow the user to specify the encoding of our xml_data.
*/

xml_element *xml_parse (xmlrpc_env *env, const char *xml_data, int xml_len)
{
    parse_context context;
    XML_Parser parser;
    int ok;

    XMLRPC_ASSERT_ENV_OK(env);
    XMLRPC_ASSERT(xml_data != NULL && xml_len >= 0);

    /* Set up our error-handling preconditions. */
    context.root = NULL;
    
    /* Set up the rest of our parse context. */
    context.env     = env;
    context.current = NULL;

    /* Set up our XML parser. */
    parser = XML_ParserCreate(NULL);
    XMLRPC_FAIL_IF_NULL(parser, env, XMLRPC_INTERNAL_ERROR,
                        "Could not create expat parser");
    XML_SetUserData(parser, &context);
    XML_SetElementHandler(parser,
                          (XML_StartElementHandler) start_element,
                          (XML_EndElementHandler) end_element);
    XML_SetCharacterDataHandler(parser,
                                (XML_CharacterDataHandler) character_data);

    /* Parse our data. */
    ok = XML_Parse(parser, xml_data, xml_len, 1);
    if (!ok)
        XMLRPC_FAIL(env, XMLRPC_PARSE_ERROR,
                    (char*) XML_ErrorString(XML_GetErrorCode(parser)));
    XMLRPC_FAIL_IF_FAULT(env);

    /* Perform some sanity checks. */
    XMLRPC_ASSERT(context.root != NULL);
    XMLRPC_ASSERT(context.current == NULL);

 cleanup:
    if (parser)
        XML_ParserFree(parser);

    if (env->fault_occurred) {
        if (context.root)
            xml_element_free(context.root);
        return NULL;
    } else {
        return context.root;
    }
}
