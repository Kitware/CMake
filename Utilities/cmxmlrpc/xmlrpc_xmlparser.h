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


/*=========================================================================
**  Abstract XML Parser Interface
**=========================================================================
**  This file provides an abstract interface to the XML parser. For now,
**  this interface is implemented by expat, but feel free to change it
**  if necessary.
*/


/*=========================================================================
**  xml_element
**=========================================================================
**  This data structure represents an XML element. We provide no more API
**  than we'll need in xmlrpc_parse.c.
**
**  The pointers returned by the various accessor methods belong to the
**  xml_element structure. Do not free them, and do not use them after
**  the xml_element has been destroyed.
*/

/* You'll need to finish defining struct _xml_element elsewhere. */
typedef struct _xml_element xml_element;

/* Destroy an xml_element. */
void xml_element_free (xml_element *elem);

/* Return a pointer to the element's name. Do not free this pointer!
** This pointer should point to standard ASCII or UTF-8 data. */
char *xml_element_name (xml_element *elem);

/* Return the xml_element's CDATA. Do not free this pointer!
** This pointer should point to standard ASCII or UTF-8 data.
** The implementation is allowed to concatenate all the CDATA in the
** element regardless of child elements. Alternatively, if there are
** any child elements, the implementation is allowed to dispose
** of whitespace characters.
** The value returned by xml_element_cdata should be '\0'-terminated
** (although it may contain other '\0' characters internally).
** xml_element_cdata_size should not include the final '\0'. */
size_t xml_element_cdata_size (xml_element *elem);
char *xml_element_cdata (xml_element *elem);

/* Return the xml_element's child elements. Do not free this pointer! */
size_t xml_element_children_size (xml_element *elem);
xml_element **xml_element_children (xml_element *elem);


/*=========================================================================
**  xml_parse
**=========================================================================
**  Parse a chunk of XML data and return the top-level element. If this
**  routine fails, it will return NULL and set up the env appropriately.
**  You are responsible for calling xml_element_free on the returned pointer.
*/

xml_element *xml_parse (xmlrpc_env *env, const char *xml_data, int xml_len);
