/*============================================================================
                         xmlrpc_client_int.h
==============================================================================
  This header file defines the interface between modules inside
  xmlrpc-c.

  Use this in addition to xmlrpc.h, which defines the external
  interface.

  Copyright information is at the end of the file.
============================================================================*/


#ifndef  _XMLRPC_INT_H_
#define  _XMLRPC_INT_H_ 1

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


struct _xmlrpc_value {
    xmlrpc_type _type;
    int _refcount;

    /* Certain data types store their data directly in the xmlrpc_value. */
    union {
        xmlrpc_int32 i;
        xmlrpc_bool b;
        double d;
        /* time_t t */
        void *c_ptr;
    } _value;
    
    /* Other data types use a memory block. */
    xmlrpc_mem_block _block;

#ifdef HAVE_UNICODE_WCHAR
    /* We may need to convert our string data to a wchar_t string. */
    xmlrpc_mem_block *_wcs_block;
#endif
};

typedef struct {
    unsigned char key_hash;
    xmlrpc_value *key;
    xmlrpc_value *value;
} _struct_member;


struct _xmlrpc_registry {
    int _introspection_enabled;
    xmlrpc_value *_methods;
    xmlrpc_value *_default_method;
    xmlrpc_value *_preinvoke_method;
};


/* When we deallocate a pointer in a struct, we often replace it with
** this and throw in a few assertions here and there. */
#define XMLRPC_BAD_POINTER ((void*) 0xDEADBEEF)


void
xmlrpc_traceXml(const char * const label, 
                const char * const xml,
                unsigned int const xmlLength);

void
xmlrpc_destroyStruct(xmlrpc_value * const structP);

void
xmlrpc_destroyArrayContents(xmlrpc_value * const arrayP);

const char * 
xmlrpc_makePrintable(const char * const input);

const char *
xmlrpc_makePrintableChar(char const input);



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

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
