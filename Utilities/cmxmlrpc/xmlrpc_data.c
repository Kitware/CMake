/* Copyright information is at end of file */

#include "xmlrpc_config.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "bool.h"
#include "xmlrpc.h"
#include "xmlrpc_int.h"

/* Borrowed from Python 1.5.2.
** MPW pushes 'extended' for float and double types with varargs */
#ifdef MPW
typedef extended va_double;
#else
typedef double va_double;
#endif

/* Borrowed from Python 1.5.2.
** Python copies its va_list objects before using them in certain
** tricky fashions. We don't why Python does this, but since we're
** abusing our va_list objects in a similar fashion, we'll copy them
** too. */
#if VA_LIST_IS_ARRAY
#define VA_LIST_COPY(dest,src) memcpy((dest), (src), sizeof(va_list))
#else
#define VA_LIST_COPY(dest,src) ((dest) = (src))
#endif


static void
destroyValue(xmlrpc_value * const valueP) {

    /* First, we need to destroy this value's contents, if any. */
    switch (valueP->_type) {
    case XMLRPC_TYPE_INT:
    case XMLRPC_TYPE_BOOL:
    case XMLRPC_TYPE_DOUBLE:
        break;
        
    case XMLRPC_TYPE_ARRAY:
        xmlrpc_destroyArrayContents(valueP);
        break;
        
    case XMLRPC_TYPE_STRING:
#ifdef HAVE_UNICODE_WCHAR
        if (valueP->_wcs_block)
            xmlrpc_mem_block_free(valueP->_wcs_block);
#endif /* HAVE_UNICODE_WCHAR */
        /* Fall through. */

    case XMLRPC_TYPE_DATETIME:
    case XMLRPC_TYPE_BASE64:
        xmlrpc_mem_block_clean(&valueP->_block);
        break;

    case XMLRPC_TYPE_STRUCT:
        xmlrpc_destroyStruct(valueP);
        break;

    case XMLRPC_TYPE_C_PTR:
        break;

    case XMLRPC_TYPE_DEAD:
        XMLRPC_ASSERT(FALSE); /* Can't happen, per entry conditions */

    default:
        XMLRPC_ASSERT(FALSE); /* There are no other possible values */
    }

    /* Next, we mark this value as invalid, to help catch refcount
        ** errors. */
    valueP->_type = XMLRPC_TYPE_DEAD;

    /* Finally, we destroy the value itself. */
    free(valueP);
}



/*=========================================================================
**  Reference Counting
**=========================================================================
**  Some simple reference-counting code. The xmlrpc_DECREF routine is in
**  charge of destroying values when their reference count equals zero.
*/

void 
xmlrpc_INCREF (xmlrpc_value * const valueP) {

    XMLRPC_ASSERT_VALUE_OK(valueP);
    XMLRPC_ASSERT(valueP->_refcount > 0);

    valueP->_refcount++;
}



void 
xmlrpc_DECREF (xmlrpc_value * const valueP) {

    XMLRPC_ASSERT_VALUE_OK(valueP);
    XMLRPC_ASSERT(valueP->_refcount > 0);
    XMLRPC_ASSERT(valueP->_type != XMLRPC_TYPE_DEAD);

    valueP->_refcount--;

    /* If we have no more refs, we need to deallocate this value. */
    if (valueP->_refcount == 0)
        destroyValue(valueP);
}



/*=========================================================================
    Utiltiies
=========================================================================*/

static const char *
typeName(xmlrpc_type const type) {

    switch(type) {

    case XMLRPC_TYPE_INT: return "INT";
    case XMLRPC_TYPE_BOOL: return "BOOL";
    case XMLRPC_TYPE_DOUBLE: return "DOUBLE";
    case XMLRPC_TYPE_DATETIME: return "DATETIME";
    case XMLRPC_TYPE_STRING: return "STRING";
    case XMLRPC_TYPE_BASE64: return "BASE64";
    case XMLRPC_TYPE_ARRAY: return "ARRAY";
    case XMLRPC_TYPE_STRUCT: return "STRUCT";
    case XMLRPC_TYPE_C_PTR: return "C_PTR";
    case XMLRPC_TYPE_DEAD: return "DEAD";
    default: return "???";
    }
}



static void
verifyNoNulls(xmlrpc_env * const envP,
              const char * const contents,
              unsigned int const len) {
/*----------------------------------------------------------------------------
   Verify that the character array 'contents', which is 'len' bytes long,
   does not contain any NUL characters, which means it can be made into
   a passable ASCIIZ string just by adding a terminating NUL.

   Fail if the array contains a NUL.
-----------------------------------------------------------------------------*/
    unsigned int i;

    for (i = 0; i < len && !envP->fault_occurred; i++)
        if (contents[i] == '\0')
            xmlrpc_env_set_fault_formatted(
                envP, XMLRPC_TYPE_ERROR, 
                "String must not contain NUL characters");
}



static void
verifyNoNullsW(xmlrpc_env *    const envP,
               const wchar_t * const contents,
               unsigned int    const len) {
/*----------------------------------------------------------------------------
   Same as verifyNoNulls(), but for wide characters.
-----------------------------------------------------------------------------*/
    unsigned int i;

    for (i = 0; i < len && !envP->fault_occurred; i++)
        if (contents[i] == '\0')
            xmlrpc_env_set_fault_formatted(
                envP, XMLRPC_TYPE_ERROR, 
                "String must not contain NUL characters");
}



static void
validateType(xmlrpc_env *         const envP,
             const xmlrpc_value * const valueP,
             xmlrpc_type          const expectedType) {

    if (valueP->_type != expectedType) {
        xmlrpc_env_set_fault_formatted(
            envP, XMLRPC_TYPE_ERROR, "Value of type %s supplied where "
            "type %s was expected.", 
            typeName(valueP->_type), typeName(expectedType));
    }
}



/*=========================================================================
    Extracting XML-RPC value
===========================================================================
  These routines extract XML-RPC values into ordinary C data types.

  For array and struct values, see the separates files xmlrpc_array.c
  and xmlrpc_struct.c.
=========================================================================*/

void 
xmlrpc_read_int(xmlrpc_env *         const envP,
                const xmlrpc_value * const valueP,
                xmlrpc_int32 *       const intValueP) {

    validateType(envP, valueP, XMLRPC_TYPE_INT);
    if (!envP->fault_occurred)
        *intValueP = valueP->_value.i;
}



void
xmlrpc_read_double(xmlrpc_env *         const envP,
                   const xmlrpc_value * const valueP,
                   xmlrpc_double *      const doubleValueP) {
    
    validateType(envP, valueP, XMLRPC_TYPE_DOUBLE);
    if (!envP->fault_occurred)
        *doubleValueP = valueP->_value.d;

}



void
xmlrpc_read_bool(xmlrpc_env *         const envP,
                 const xmlrpc_value * const valueP,
                 xmlrpc_bool *        const boolValueP) {

    validateType(envP, valueP, XMLRPC_TYPE_BOOL);
    if (!envP->fault_occurred)
        *boolValueP = valueP->_value.b;
}



void
xmlrpc_read_string(xmlrpc_env *         const envP,
                   const xmlrpc_value * const valueP,
                   const char **        const stringValueP) {
/*----------------------------------------------------------------------------
   Read the value of an XML-RPC string an ASCIIZ string.

   Fail if the string contains null characters (which means it wasn't
   really a string, but XML-RPC doesn't seem to understand what a string
   is, and such values are possible).
-----------------------------------------------------------------------------*/
    validateType(envP, valueP, XMLRPC_TYPE_STRING);
    if (!envP->fault_occurred) {
        size_t const size = 
            XMLRPC_MEMBLOCK_SIZE(char, &valueP->_block);
        const char * const contents = 
            XMLRPC_MEMBLOCK_CONTENTS(char, &valueP->_block);

        verifyNoNulls(envP, contents, (unsigned int)size);
        if (!envP->fault_occurred) {
            char * stringValue;
            
            stringValue = malloc(size+1);
            if (stringValue == NULL)
                xmlrpc_env_set_fault_formatted(
                    envP, XMLRPC_INTERNAL_ERROR, "Unable to allocate space "
                    "for %u-character string", size);
            else {
                memcpy(stringValue,
                       XMLRPC_MEMBLOCK_CONTENTS(char, &valueP->_block), size);
                stringValue[size] = '\0';

                *stringValueP = stringValue;
            }
        }
    }
}



void
xmlrpc_read_string_lp(xmlrpc_env *         const envP,
                      const xmlrpc_value * const valueP,
                      unsigned int *       const lengthP,
                      const char **        const stringValueP) {

    validateType(envP, valueP, XMLRPC_TYPE_STRING);
    if (!envP->fault_occurred) {
        size_t const size = 
            XMLRPC_MEMBLOCK_SIZE(char, &valueP->_block);
        const char * const contents = 
            XMLRPC_MEMBLOCK_CONTENTS(char, &valueP->_block);

        char * stringValue;

        stringValue = malloc(size);
        if (stringValue == NULL)
            xmlrpc_env_set_fault_formatted(
                envP, XMLRPC_INTERNAL_ERROR, "Unable to allocate %u bytes "
                "for string.", size);
        else {
            memcpy(stringValue, contents, size);
            *stringValueP = stringValue;
            *lengthP = (unsigned int)size;
        }
    }
}



/*=========================================================================
**  Building XML-RPC values.
**=========================================================================
**  Build new XML-RPC values from a format string. This code is heavily
**  inspired by Py_BuildValue from Python 1.5.2. In particular, our
**  particular abuse of the va_list data type is copied from the equivalent
**  Python code in modsupport.c. Since Python is portable, our code should
**  (in theory) also be portable.
*/


xmlrpc_type xmlrpc_value_type (xmlrpc_value* value)
{
    XMLRPC_ASSERT_VALUE_OK(value);
    return value->_type;
}



static void
createXmlrpcValue(xmlrpc_env *    const envP,
                  xmlrpc_value ** const valPP) {
/*----------------------------------------------------------------------------
   Create a blank xmlrpc_value to be filled in.

   Set the reference count to 1.
-----------------------------------------------------------------------------*/
    xmlrpc_value * valP;

    valP = (xmlrpc_value*) malloc(sizeof(xmlrpc_value));
    if (!valP)
        xmlrpc_env_set_fault(envP, XMLRPC_INTERNAL_ERROR,
                             "Could not allocate memory for xmlrpc_value");
    else
        valP->_refcount = 1;

    *valPP = valP;
}



static void
mkInt(xmlrpc_env *    const envP, 
      xmlrpc_int32    const value,
      xmlrpc_value ** const valPP) {

    xmlrpc_value * valP;

    createXmlrpcValue(envP, &valP);

    if (!envP->fault_occurred) {
        valP->_type    = XMLRPC_TYPE_INT;
        valP->_value.i = value;
    }
    *valPP = valP;
}



static void
mkBool(xmlrpc_env *    const envP, 
       xmlrpc_bool     const value,
       xmlrpc_value ** const valPP) {

    xmlrpc_value * valP;

    createXmlrpcValue(envP, &valP);

    if (!envP->fault_occurred) {
        valP->_type = XMLRPC_TYPE_BOOL;
        valP->_value.b = value;
    }
    *valPP = valP;
}



static void
mkDouble(xmlrpc_env *    const envP, 
         double          const value,
         xmlrpc_value ** const valPP) {

    xmlrpc_value * valP;

    createXmlrpcValue(envP, &valP);

    if (!envP->fault_occurred) {
        valP->_type = XMLRPC_TYPE_DOUBLE;
        valP->_value.d = value;
    }
    *valPP = valP;
}



#ifdef HAVE_UNICODE_WCHAR
#define MAKE_WCS_BLOCK_NULL(val) ((val)->_wcs_block = NULL)
#else
#define MAKE_WCS_BLOCK_NULL(val) do {} while(0)
#endif



static void
mkString(xmlrpc_env *    const envP, 
         const char *    const value,
         unsigned int    const length,
         xmlrpc_value ** const valPP) {

    xmlrpc_value * valP;

    createXmlrpcValue(envP, &valP);

    if (!envP->fault_occurred) {
        valP->_type = XMLRPC_TYPE_STRING;
        MAKE_WCS_BLOCK_NULL(valP);
        XMLRPC_MEMBLOCK_INIT(char, envP, &valP->_block, length + 1);
        if (!envP->fault_occurred) {
            char * const contents =
                XMLRPC_MEMBLOCK_CONTENTS(char, &valP->_block);
            memcpy(contents, value, length);
            contents[length] = '\0';
        }
        if (envP->fault_occurred)
            free(valP);
    }
    *valPP = valP;
}



static void
getString(xmlrpc_env *    const envP,
          const char **   const formatP,
          va_list *       const args,
          xmlrpc_value ** const valPP) {

    const char * str;
    size_t len;
    
    str = (const char*) va_arg(*args, char*);
    if (**formatP == '#') {
        (*formatP)++;
        len = (size_t) va_arg(*args, size_t);
    } else
        len = strlen(str);

    mkString(envP, str, (unsigned int)len, valPP);
}



#ifdef HAVE_UNICODE_WCHAR
static void
mkWideString(xmlrpc_env *    const envP,
             wchar_t *       const wcs,
             size_t          const wcs_len,
             xmlrpc_value ** const valPP) {

    xmlrpc_value * valP;
    char *contents;
    wchar_t *wcs_contents;
    int block_is_inited;
    xmlrpc_mem_block *utf8_block;
    char *utf8_contents;
    size_t utf8_len;

    /* Error-handling preconditions. */
    utf8_block = NULL;
    block_is_inited = 0;

    /* Initialize our XML-RPC value. */
    valP = (xmlrpc_value*) malloc(sizeof(xmlrpc_value));
    XMLRPC_FAIL_IF_NULL(valP, envP, XMLRPC_INTERNAL_ERROR,
                        "Could not allocate memory for wide string");
    valP->_refcount = 1;
    valP->_type = XMLRPC_TYPE_STRING;

    /* More error-handling preconditions. */
    valP->_wcs_block = NULL;

    /* Build our wchar_t block first. */
    valP->_wcs_block =
        XMLRPC_TYPED_MEM_BLOCK_NEW(wchar_t, envP, wcs_len + 1);
    XMLRPC_FAIL_IF_FAULT(envP);
    wcs_contents =
        XMLRPC_TYPED_MEM_BLOCK_CONTENTS(wchar_t, valP->_wcs_block);
    memcpy(wcs_contents, wcs, wcs_len * sizeof(wchar_t));
    wcs_contents[wcs_len] = '\0';
    
    /* Convert the wcs block to UTF-8. */
    utf8_block = xmlrpc_wcs_to_utf8(envP, wcs_contents, wcs_len + 1);
    XMLRPC_FAIL_IF_FAULT(envP);
    utf8_contents = XMLRPC_TYPED_MEM_BLOCK_CONTENTS(char, utf8_block);
    utf8_len = XMLRPC_TYPED_MEM_BLOCK_SIZE(char, utf8_block);

    /* XXX - We need an extra memcopy to initialize _block. */
    XMLRPC_TYPED_MEM_BLOCK_INIT(char, envP, &valP->_block, utf8_len);
    XMLRPC_FAIL_IF_FAULT(envP);
    block_is_inited = 1;
    contents = XMLRPC_TYPED_MEM_BLOCK_CONTENTS(char, &valP->_block);
    memcpy(contents, utf8_contents, utf8_len);

 cleanup:
    if (utf8_block)
        xmlrpc_mem_block_free(utf8_block);
    if (envP->fault_occurred) {
        if (valP) {
            if (valP->_wcs_block)
                xmlrpc_mem_block_free(valP->_wcs_block);
            if (block_is_inited)
                xmlrpc_mem_block_clean(&valP->_block);
            free(valP);
        }
    }
    *valPP = valP;
}
#endif /* HAVE_UNICODE_WCHAR */



static void
getWideString(xmlrpc_env *    const envP,
              const char **   const formatP,
              va_list *       const args,
              xmlrpc_value ** const valPP) {
#ifdef HAVE_UNICODE_WCHAR

    wchar_t *wcs;
    size_t len;
    
    wcs = (wchar_t*) va_arg(*args, wchar_t*);
    if (**formatP == '#') {
        (*formatP)++;
        len = (size_t) va_arg(*args, size_t);
    } else
        len = wcslen(wcs);

    mkWideString(envP, wcs, len, valPP);

#endif /* HAVE_UNICODE_WCHAR */
}



static void
mkDatetime(xmlrpc_env *    const envP, 
           const char *    const value,
           xmlrpc_value ** const valPP) {

    xmlrpc_value * valP;

    createXmlrpcValue(envP, &valP);

    if (!envP->fault_occurred) {
        valP->_type = XMLRPC_TYPE_DATETIME;

        XMLRPC_TYPED_MEM_BLOCK_INIT(
            char, envP, &valP->_block, strlen(value) + 1);
        if (!envP->fault_occurred) {
            char * const contents =
                XMLRPC_TYPED_MEM_BLOCK_CONTENTS(char, &valP->_block);
            strcpy(contents, value);
        }
        if (envP->fault_occurred)
            free(valP);
    }
    *valPP = valP;
}



static void
mkBase64(xmlrpc_env *          const envP, 
         const unsigned char * const value,
         size_t                const length,
         xmlrpc_value **       const valPP) {

    xmlrpc_value * valP;

    createXmlrpcValue(envP, &valP);

    if (!envP->fault_occurred) {
        valP->_type = XMLRPC_TYPE_BASE64;

        xmlrpc_mem_block_init(envP, &valP->_block, length);
        if (!envP->fault_occurred) {
            char * const contents = 
                xmlrpc_mem_block_contents(&valP->_block);
            memcpy(contents, value, length);
        }
        if (envP->fault_occurred)
            free(valP);
    }
    *valPP = valP;
}



static void
getBase64(xmlrpc_env *    const envP,
          va_list *       const args,
          xmlrpc_value ** const valPP) {

    unsigned char * value;
    size_t          length;
    
    value  = (unsigned char*) va_arg(*args, unsigned char*);
    length = (size_t)         va_arg(*args, size_t);

    mkBase64(envP, value, length, valPP);
}



static void
mkCPtr(xmlrpc_env *    const envP, 
       void *          const value,
       xmlrpc_value ** const valPP) {

    xmlrpc_value * valP;

    createXmlrpcValue(envP, &valP);

    if (!envP->fault_occurred) {
        valP->_type = XMLRPC_TYPE_C_PTR;
        valP->_value.c_ptr = value;
    }
    *valPP = valP;
}



static void
mkArrayFromVal(xmlrpc_env *    const envP, 
               xmlrpc_value *  const value,
               xmlrpc_value ** const valPP) {

    if (xmlrpc_value_type(value) != XMLRPC_TYPE_ARRAY)
        xmlrpc_env_set_fault(envP, XMLRPC_INTERNAL_ERROR,
                             "Array format ('A'), non-array xmlrpc_value");
    else
        xmlrpc_INCREF(value);

    *valPP = value;
}



static void
mkStructFromVal(xmlrpc_env *    const envP, 
                xmlrpc_value *  const value,
                xmlrpc_value ** const valPP) {

    if (xmlrpc_value_type(value) != XMLRPC_TYPE_STRUCT)
        xmlrpc_env_set_fault(envP, XMLRPC_INTERNAL_ERROR,
                             "Struct format ('S'), non-struct xmlrpc_value");
    else
        xmlrpc_INCREF(value);

    *valPP = value;
}



static void
getValue(xmlrpc_env *    const envP, 
         const char**    const format, 
         va_list *       const args,
         xmlrpc_value ** const valPP);



static void
createXmlrpcArray(xmlrpc_env *    const envP,
                  xmlrpc_value ** const arrayPP) {
/*----------------------------------------------------------------------------
   Create an empty array xmlrpc_value.
-----------------------------------------------------------------------------*/
    xmlrpc_value * arrayP;

    createXmlrpcValue(envP, &arrayP);
    if (!envP->fault_occurred) {
        arrayP->_type = XMLRPC_TYPE_ARRAY;
        XMLRPC_TYPED_MEM_BLOCK_INIT(xmlrpc_value*, envP, &arrayP->_block, 0);
        if (envP->fault_occurred)
            free(arrayP);
    }
    *arrayPP = arrayP;
}



static void
getArray(xmlrpc_env *    const envP,
         const char **   const formatP,
         char            const delimiter,
         va_list *       const args,
         xmlrpc_value ** const arrayPP) {

    xmlrpc_value * arrayP;

    createXmlrpcArray(envP, &arrayP);

    /* Add items to the array until we hit our delimiter. */
    
    while (**formatP != delimiter && !envP->fault_occurred) {
        
        xmlrpc_value * itemP;
        
        if (**formatP == '\0')
            xmlrpc_env_set_fault(
                envP, XMLRPC_INTERNAL_ERROR,
                "format string ended before closing ')'.");
        else {
            getValue(envP, formatP, args, &itemP);
            if (!envP->fault_occurred) {
                xmlrpc_array_append_item(envP, arrayP, itemP);
                xmlrpc_DECREF(itemP);
            }
        }
    }
    if (envP->fault_occurred)
        xmlrpc_DECREF(arrayP);

    *arrayPP = arrayP;
}



static void
getStructMember(xmlrpc_env *    const envP,
                const char **   const formatP,
                va_list *       const args,
                xmlrpc_value ** const keyPP,
                xmlrpc_value ** const valuePP) {


    /* Get the key */
    getValue(envP, formatP, args, keyPP);
    if (!envP->fault_occurred) {
        if (**formatP != ':')
            xmlrpc_env_set_fault(
                envP, XMLRPC_INTERNAL_ERROR,
                "format string does not have ':' after a "
                "structure member key.");
        else {
            /* Skip over colon that separates key from value */
            (*formatP)++;
            
            /* Get the value */
            getValue(envP, formatP, args, valuePP);
        }
        if (envP->fault_occurred)
            xmlrpc_DECREF(*keyPP);
    }
}
            
            

static void
getStruct(xmlrpc_env *    const envP,
          const char **   const formatP,
          char            const delimiter,
          va_list *       const args,
          xmlrpc_value ** const structPP) {

    xmlrpc_value * structP;

    structP = xmlrpc_struct_new(envP);
    if (!envP->fault_occurred) {
        while (**formatP != delimiter && !envP->fault_occurred) {
            xmlrpc_value * keyP;
            xmlrpc_value * valueP;
            
            getStructMember(envP, formatP, args, &keyP, &valueP);
            
            if (!envP->fault_occurred) {
                if (**formatP == ',')
                    (*formatP)++;  /* Skip over the comma */
                else if (**formatP == delimiter) {
                    /* End of the line */
                } else 
                    xmlrpc_env_set_fault(
                        envP, XMLRPC_INTERNAL_ERROR,
                        "format string does not have ',' or ')' after "
                        "a structure member");
                
                if (!envP->fault_occurred)
                    /* Add the new member to the struct. */
                    xmlrpc_struct_set_value_v(envP, structP, keyP, valueP);
                
                xmlrpc_DECREF(valueP);
                xmlrpc_DECREF(keyP);
            }
        }
        if (envP->fault_occurred)
            xmlrpc_DECREF(structP);
    }
    *structPP = structP;
}



static void
getValue(xmlrpc_env *    const envP, 
         const char**    const formatP,
         va_list *       const args,
         xmlrpc_value ** const valPP) {
/*----------------------------------------------------------------------------
   Get the next value from the list.  *formatP points to the specifier
   for the next value in the format string (i.e. to the type code
   character) and we move *formatP past the whole specifier for the
   next value.  We read the required arguments from 'args'.  We return
   the value as *valPP with a reference to it.

   For example, if *formatP points to the "i" in the string "sis",
   we read one argument from 'args' and return as *valP an integer whose
   value is the argument we read.  We advance *formatP to point to the
   last 's' and advance 'args' to point to the argument that belongs to
   that 's'.
-----------------------------------------------------------------------------*/
    char const formatChar = *(*formatP)++;

    switch (formatChar) {
    case 'i':
        mkInt(envP, (xmlrpc_int32) va_arg(*args, xmlrpc_int32), valPP);
        break;

    case 'b':
        mkBool(envP, (xmlrpc_bool) va_arg(*args, xmlrpc_bool), valPP);
        break;

    case 'd':
        mkDouble(envP, (double) va_arg(*args, va_double), valPP);
        break;

    case 's':
        getString(envP, formatP, args, valPP);
        break;

    case 'w':
        getWideString(envP, formatP, args, valPP);
        break;

    /* The code 't' is reserved for a better, time_t based
       implementation of dateTime conversion.  
    */
    case '8':
        mkDatetime(envP, (char*) va_arg(*args, char*), valPP);
        break;

    case '6':
        getBase64(envP, args, valPP);
        break;

    case 'p':
        /* We might someday want to use the code 'p!' to read in a
           cleanup function for this pointer. 
        */
        mkCPtr(envP, (void*) va_arg(*args, void*), valPP);
        break;      

    case 'A':
        mkArrayFromVal(envP, (xmlrpc_value*) va_arg(*args, xmlrpc_value*),
                       valPP);
        break;

    case 'S':
        mkStructFromVal(envP, (xmlrpc_value*) va_arg(*args, xmlrpc_value*),
                        valPP);
        break;

    case 'V':
        *valPP = (xmlrpc_value*) va_arg(*args, xmlrpc_value*);
        xmlrpc_INCREF(*valPP);
        break;

    case '(':
        getArray(envP, formatP, ')', args, valPP);
        if (!envP->fault_occurred) {
            XMLRPC_ASSERT(**formatP == ')');
            (*formatP)++;  /* Skip over closing parenthesis */
        }
        break;

    case '{': 
        getStruct(envP, formatP, '}', args, valPP);
        if (!envP->fault_occurred) {
            XMLRPC_ASSERT(**formatP == '}');
            (*formatP)++;  /* Skip over closing brace */
        }
        break;

    default: {
        const char * const badCharacter = xmlrpc_makePrintableChar(formatChar);
        xmlrpc_env_set_fault_formatted(
            envP, XMLRPC_INTERNAL_ERROR,
            "Unexpected character '%s' in format string", badCharacter);
        xmlrpc_strfree(badCharacter);
        }
    }
}



void
xmlrpc_build_value_va(xmlrpc_env *    const envP,
                      const char *    const format,
                      va_list               args,
                      xmlrpc_value ** const valPP,
                      const char **   const tailP) {

    const char * formatCursor;
    va_list args_copy;

    XMLRPC_ASSERT_ENV_OK(envP);
    XMLRPC_ASSERT(format != NULL);

    if (strlen(format) == 0)
        xmlrpc_env_set_fault_formatted(
            envP, XMLRPC_INTERNAL_ERROR, "Format string is empty.");
    else {
        formatCursor = &format[0];
        VA_LIST_COPY(args_copy, args);
        getValue(envP, &formatCursor, &args_copy, valPP);
        
        if (!envP->fault_occurred)
            XMLRPC_ASSERT_VALUE_OK(*valPP);
        
        *tailP = formatCursor;
    }
}



xmlrpc_value * 
xmlrpc_build_value(xmlrpc_env * const envP,
                   const char * const format, 
                   ...) {

    va_list args;
    xmlrpc_value* retval;
    const char * suffix;

    va_start(args, format);
    xmlrpc_build_value_va(envP, format, args, &retval, &suffix);
    va_end(args);

    if (!envP->fault_occurred) {
        if (*suffix != '\0')
            xmlrpc_env_set_fault_formatted(
                envP, XMLRPC_INTERNAL_ERROR, "Junk after the argument "
                "specifier: '%s'.  There must be exactly one arument.",
                suffix);
    
        if (envP->fault_occurred)
            xmlrpc_DECREF(retval);
    }
    return retval;
}


/*=========================================================================
**  Parsing XML-RPC values.
**=========================================================================
**  Parse an XML-RPC value based on a format string. This code is heavily
**  inspired by Py_BuildValue from Python 1.5.2.
*/

/* Prototype for recursive invocation: */

static void 
parsevalue(xmlrpc_env *   const env,
           xmlrpc_value * const val,
           const char **  const format,
           va_list *            args);

static void 
parsearray(xmlrpc_env *         const env,
           const xmlrpc_value * const array,
           const char **        const format,
           char                 const delimiter,
           va_list *                  args) {

    int size, i;
    xmlrpc_value *item;

    /* Fetch the array size. */
    size = xmlrpc_array_size(env, array);
    XMLRPC_FAIL_IF_FAULT(env);

    /* Loop over the items in the array. */
    for (i = 0; i < size; i++) {
        /* Bail out if the caller didn't care about the rest of the items. */
        if (**format == '*')
            break;

        item = xmlrpc_array_get_item(env, array, i);
        XMLRPC_FAIL_IF_FAULT(env);

        XMLRPC_ASSERT(**format != '\0');
        if (**format == delimiter)
            XMLRPC_FAIL(env, XMLRPC_INDEX_ERROR, "Too many items in array");
        parsevalue(env, item, format, args);
        XMLRPC_FAIL_IF_FAULT(env);
    }
    if (**format == '*')
        (*format)++;
    if (**format != delimiter)
        XMLRPC_FAIL(env, XMLRPC_INDEX_ERROR, "Not enough items in array");

           cleanup:
    return;
}



static void 
parsestruct(xmlrpc_env *   const env,
            xmlrpc_value * const strct,
            const char **  const format,
            char           const delimiter,
            va_list *            args) {

    xmlrpc_value *key, *value;
    char *keystr;
    size_t keylen;

    /* Set up error handling preconditions. */
    key = NULL;

    /* Build the members of our struct. */
    while (**format != '*' && **format != delimiter && **format != '\0') {

        /* Get our key, and skip over the ':' character. Notice the
        ** sudden call to getValue--we're going in the opposite direction. */
        getValue(env, format, args, &key);
        XMLRPC_FAIL_IF_FAULT(env);
        XMLRPC_ASSERT(**format == ':');
        (*format)++;

        /* Look up the value for our key. */
        xmlrpc_parse_value(env, key, "s#", &keystr, &keylen);
        XMLRPC_FAIL_IF_FAULT(env);
        value = xmlrpc_struct_get_value_n(env, strct, keystr, keylen);
        XMLRPC_FAIL_IF_FAULT(env);

        /* Get our value, and skip over the ',' character (if present). */
        parsevalue(env, value, format, args);
        XMLRPC_FAIL_IF_FAULT(env);
        XMLRPC_ASSERT(**format == ',' || **format == delimiter);
        if (**format == ',')
            (*format)++;

        /* Release our reference, and restore our invariant. */
        xmlrpc_DECREF(key);
        key = NULL;
    }
    if (**format == '*') {
        (*format)++;
        if (**format != delimiter && **format != '\0')
            XMLRPC_FAIL(env, XMLRPC_INTERNAL_ERROR, 
                        "* can appear only at the end "
                        "of a structure format specifier");
    } else {
        /* Here we're supposed to fail if he didn't extract all the
           members.  But we don't know how to determine whether he
           specified all the members, so we always fail.
        */
        XMLRPC_FAIL(env, XMLRPC_INTERNAL_ERROR, "You must specify '*' as the "
                    "last member of a structure in a format specifier "
                    "used for parsing an xmlrpc_value"); 
    }
    XMLRPC_ASSERT(**format == delimiter || **format == '\0');

cleanup:
    if (key)
        xmlrpc_DECREF(key);
}


static void 
parsevalue(xmlrpc_env *   const envP,
           xmlrpc_value * const valueP,
           const char **  const format,
           va_list *            args) {

    char formatSpecChar;

    formatSpecChar = *(*format)++;

    switch (formatSpecChar) {
    case 'i':
        validateType(envP, valueP, XMLRPC_TYPE_INT);
        if (!envP->fault_occurred) {
            xmlrpc_int32 * const int32ptr = 
                (xmlrpc_int32*) va_arg(*args, xmlrpc_int32*);
            *int32ptr = valueP->_value.i;
        }
        break;

    case 'b':
        validateType(envP, valueP, XMLRPC_TYPE_BOOL);
        if (!envP->fault_occurred) {
            xmlrpc_bool * const boolptr =
                (xmlrpc_bool*) va_arg(*args, xmlrpc_bool*);
            *boolptr = valueP->_value.b;       
        }
        break;

    case 'd':
        validateType(envP, valueP, XMLRPC_TYPE_DOUBLE);
        if (!envP->fault_occurred) {
            double * const doubleptr = (double*) va_arg(*args, double*);
            *doubleptr = valueP->_value.d;     
        }
        break;

    case 's':
        validateType(envP, valueP, XMLRPC_TYPE_STRING);
        if (!envP->fault_occurred) {
            char * const contents =
                XMLRPC_MEMBLOCK_CONTENTS(char, &valueP->_block);
            size_t const len = XMLRPC_MEMBLOCK_SIZE(char, &valueP->_block) - 1;
            
            char ** const strptr = (char**) va_arg(*args, char**);
            if (**format == '#') {
                size_t * const sizeptr = (size_t*) va_arg(*args, size_t**);
                (*format)++;
                *sizeptr = len;
            } else
                verifyNoNulls(envP, contents, (unsigned int)len);
            *strptr = contents;
        }
        break;

#ifdef HAVE_UNICODE_WCHAR
    case 'w':
        validateType(envP, valueP, XMLRPC_TYPE_STRING);
        if (!envP->fault_occurred) {
            if (!valueP->_wcs_block) {
                /* Allocate a wchar_t string if we don't have one. */
                char * const contents = 
                    XMLRPC_MEMBLOCK_CONTENTS(char, &valueP->_block);
                size_t const len = 
                    XMLRPC_MEMBLOCK_SIZE(char, &valueP->_block) - 1;
                valueP->_wcs_block = 
                    xmlrpc_utf8_to_wcs(envP, contents, len + 1);
            }
            if (!envP->fault_occurred) {
                wchar_t * const wcontents = 
                    XMLRPC_MEMBLOCK_CONTENTS(wchar_t, valueP->_wcs_block);
                size_t const len = 
                    XMLRPC_MEMBLOCK_SIZE(wchar_t, valueP->_wcs_block) - 1;

                wchar_t ** const wcsptr = (wchar_t**) va_arg(*args, wchar_t**);
                if (**format == '#') {
                    size_t * const sizeptr = (size_t*) va_arg(*args, size_t**);
                    (*format)++;
                    *sizeptr = len;
                } else
                    verifyNoNullsW(envP, wcontents, (unsigned int)len);
                *wcsptr = wcontents;
            }
        }
        break;
#endif /* HAVE_UNICODE_WCHAR */
        
    case '8':
        /* The code 't' is reserved for a better, time_t based
        ** implementation of dateTime conversion. */
        validateType(envP, valueP, XMLRPC_TYPE_DATETIME);
        if (!envP->fault_occurred) {
            char * const contents = 
                XMLRPC_MEMBLOCK_CONTENTS(char, &valueP->_block);
            char ** const strptr = (char**) va_arg(*args, char**);
            *strptr = contents;
        }
        break;

    case '6':
        validateType(envP, valueP, XMLRPC_TYPE_BASE64);
        if (!envP->fault_occurred) {
            unsigned char * const bin_data =
                XMLRPC_MEMBLOCK_CONTENTS(unsigned char,
                                         &valueP->_block);
            size_t const len = XMLRPC_MEMBLOCK_SIZE(char, &valueP->_block);
            unsigned char ** const binptr =
                (unsigned char**) va_arg(*args, unsigned char**);
            size_t * const sizeptr = (size_t*) va_arg(*args, size_t**);        
            *binptr = bin_data;
            *sizeptr = len;
        }
        break;

    case 'p':
        validateType(envP, valueP, XMLRPC_TYPE_C_PTR);
        if (!envP->fault_occurred) {
            void ** const voidptrptr = (void**) va_arg(*args, void**);
            *voidptrptr = valueP->_value.c_ptr;
        }
        break;

    case 'V': {
        xmlrpc_value ** const valptr =
            (xmlrpc_value**) va_arg(*args, xmlrpc_value**);
        *valptr = valueP;
    }
        break;

    case 'A':
        validateType(envP, valueP, XMLRPC_TYPE_ARRAY);
        if (!envP->fault_occurred) {
            xmlrpc_value ** const valptr =
                (xmlrpc_value**) va_arg(*args, xmlrpc_value**);
            *valptr = valueP;
        }
        break;

    case 'S':
        validateType(envP, valueP, XMLRPC_TYPE_STRUCT);
        if (!envP->fault_occurred) {
            xmlrpc_value ** const valptr =
                (xmlrpc_value**) va_arg(*args, xmlrpc_value**);
            *valptr = valueP;
        }
        break;

    case '(':
        validateType(envP, valueP, XMLRPC_TYPE_ARRAY);
        if (!envP->fault_occurred) {
            parsearray(envP, valueP, format, ')', args);
            (*format)++;
        }
        break;

    case '{':
        validateType(envP, valueP, XMLRPC_TYPE_STRUCT);
        if (!envP->fault_occurred) {
            parsestruct(envP, valueP, format, '}', args);
            (*format)++;
        }
        break;

    default:
        xmlrpc_env_set_fault_formatted(
            envP, XMLRPC_INTERNAL_ERROR, "Invalid format character '%c'",
            formatSpecChar);
    }
}



void 
xmlrpc_parse_value_va(xmlrpc_env *   const envP,
                      xmlrpc_value * const value,
                      const char *   const format,
                      va_list              args) {

    const char *format_copy;
    va_list args_copy;

    XMLRPC_ASSERT_ENV_OK(envP);
    XMLRPC_ASSERT_VALUE_OK(value);
    XMLRPC_ASSERT(format != NULL);

    format_copy = format;
    VA_LIST_COPY(args_copy, args);
    parsevalue(envP, value, &format_copy, &args_copy);
    if (!envP->fault_occurred) {
        XMLRPC_ASSERT(*format_copy == '\0');
    }
}



void 
xmlrpc_parse_value(xmlrpc_env *   const envP,
                   xmlrpc_value * const value,
                   const char *   const format, 
                   ...) {

    va_list args;

    va_start(args, format);
    xmlrpc_parse_value_va(envP, value, format, args);
    va_end(args);
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
