/*============================================================================
                         xmlrpc_server_abyss_int.h
==============================================================================
  This header file defines the interface between client modules inside
  xmlrpc-c.

  Use this in addition to xmlrpc_server_abyss.h, which defines the external
  interface.

  Copyright information is at the end of the file.
============================================================================*/

#ifndef  _XMLRPC_SERVER_ABYSS_INT_H_
#define  _XMLRPC_SERVER_ABYSS_INT_H_ 1

#include "abyss.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*=========================================================================
**  Abyss Content Handlers
**=========================================================================
**  These are Abyss handlers.  You install them into an Abyss server.
*/

/* Handler for XML-RPC requests.  Install this using ServerAddHandler
   as the handler for all requests to /RPC2. This handler assumes that
   it can read from the method registry without running into race
   conditions or anything nasty like that.  
*/
extern xmlrpc_bool
xmlrpc_server_abyss_rpc2_handler (TSession *r);

/* A default handler.  Install this as the default handler with
   ServerDefaultHandler if you don't want to serve any HTML or
   GIFs from your htdocs directory. 
 
   This handler always returns a "404 Not Found". 
*/
extern xmlrpc_bool
xmlrpc_server_abyss_default_handler (TSession *r);




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

