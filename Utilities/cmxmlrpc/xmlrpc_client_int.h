/*============================================================================
                         xmlrpc_client_int.h
==============================================================================
  This header file defines the interface between client modules inside
  xmlrpc-c.

  Use this in addition to xmlrpc_client.h, which defines the external
  interface.

  Copyright information is at the end of the file.
============================================================================*/


#ifndef  _XMLRPC_CLIENT_INT_H_
#define  _XMLRPC_CLIENT_INT_H_ 1

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct _xmlrpc_server_info {
    char *_server_url;
    char *_http_basic_auth;
};

/* Create a new server info record, with a copy of the old server. */
extern xmlrpc_server_info * 
xmlrpc_server_info_copy(xmlrpc_env *env, xmlrpc_server_info *aserver);


/*=========================================================================
** Transport Implementation functions.
**=========================================================================
*/
#include "xmlrpc_transport.h"

/* The generalized event loop. This uses the above flags. For more details,
** see the wrapper functions below. If you're not using the timeout, the
** 'milliseconds' parameter will be ignored.
** Note that ANY event loop call will return immediately if there are
** no outstanding XML-RPC calls. */
extern void
xmlrpc_client_event_loop_run_general (int flags, timeout_t milliseconds);

/* Run the event loop forever. The loop will exit if someone calls
** xmlrpc_client_event_loop_end. */
extern void
xmlrpc_client_event_loop_run (void);

/* Run the event loop forever. The loop will exit if someone calls
** xmlrpc_client_event_loop_end or the timeout expires.
** (Note that ANY event loop call will return immediately if there are
** no outstanding XML-RPC calls.) */
extern void
xmlrpc_client_event_loop_run_timeout (timeout_t milliseconds);

/* End the running event loop immediately. This can also be accomplished
** by calling the corresponding function in libwww.
** (Note that ANY event loop call will return immediately if there are
** no outstanding XML-RPC calls.) */
extern void
xmlrpc_client_event_loop_end (void);


/* Return true if there are uncompleted asynchronous calls.
** The exact value of this during a response callback is undefined. */
extern int
xmlrpc_client_asynch_calls_are_unfinished (void);

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



