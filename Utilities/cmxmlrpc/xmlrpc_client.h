/*============================================================================
                         xmlrpc_client.h
==============================================================================
  This header file defines the interface between xmlrpc.c and its users,
  related to clients.

  Copyright information is at the end of the file.
============================================================================*/

#ifndef  _XMLRPC_CLIENT_H_
#define  _XMLRPC_CLIENT_H_ 1

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*=========================================================================
**  Initialization and Shutdown
**=========================================================================
**  These routines initialize and terminate the XML-RPC client. If you're
**  already using libwww on your own, you can pass
**  XMLRPC_CLIENT_SKIP_LIBWWW_INIT to avoid initializing it twice.
*/

#define XMLRPC_CLIENT_NO_FLAGS         (0)
#define XMLRPC_CLIENT_SKIP_LIBWWW_INIT (1)

extern void
xmlrpc_client_init(int          const flags,
                   const char * const appname,
                   const char * const appversion);

struct xmlrpc_clientparms {
    const char * transport;
};

#define XMLRPC_CP_MEMBER_OFFSET(mbrname) \
  ((unsigned long)(char*)&((struct xmlrpc_clientparms *)0)->mbrname)
#define XMLRPC_CP_MEMBER_SIZE(mbrname) \
  sizeof(((struct xmlrpc_clientparms *)0)->mbrname)
#define XMLRPC_CPSIZE(mbrname) \
  (XMLRPC_CP_MEMBER_OFFSET(mbrname) + XMLRPC_CP_MEMBER_SIZE(mbrname))

/* XMLRPC_CPSIZE(xyz) is the minimum size a struct xmlrpc_clientparms
   must be to include the 'xyz' member.  This is essential to forward and
   backward compatbility, as new members will be added to the end of the
   struct in future releases.  This is how the callee knows whether or
   not the caller is new enough to have supplied a certain parameter.
*/

void 
xmlrpc_client_init2(xmlrpc_env *                const env,
                    int                         const flags,
                    const char *                const appname,
                    const char *                const appversion,
                    struct xmlrpc_clientparms * const clientparms,
                    unsigned int                const parm_size);

extern void
xmlrpc_client_cleanup(void);

const char * 
xmlrpc_client_get_default_transport(xmlrpc_env * const env);

/*=========================================================================
** Required for both internal and external development.
**=========================================================================
*/
/* A callback function to handle the response to an asynchronous call.
** If 'fault->fault_occurred' is true, then response will be NULL. All
** arguments except 'user_data' will be deallocated internally; please do
** not free any of them yourself.
** WARNING: param_array may (or may not) be NULL if fault->fault_occurred
** is true, and you set up the call using xmlrpc_client_call_asynch.
** WARNING: If asynchronous calls are still pending when the library is
** shut down, your handler may (or may not) be called with a fault. */
typedef void (*xmlrpc_response_handler) (const char *server_url,
                                         const char *method_name,
                                         xmlrpc_value *param_array,
                                         void *user_data,
                                         xmlrpc_env *fault,
                                         xmlrpc_value *result);


/*=========================================================================
**  xmlrpc_server_info
**=========================================================================
**  We normally refer to servers by URL. But sometimes we need to do extra
**  setup for particular servers. In that case, we can create an
**  xmlrpc_server_info object, configure it in various ways, and call the
**  remote server.
**
**  (This interface is also designed to discourage further multiplication
**  of xmlrpc_client_call APIs. We have enough of those already. Please
**  add future options and flags using xmlrpc_server_info.)
*/

typedef struct _xmlrpc_server_info xmlrpc_server_info;

/* Create a new server info record, pointing to the specified server. */
xmlrpc_server_info *
xmlrpc_server_info_new(xmlrpc_env * const env,
                       const char * const server_url);

/* Create a new server info record, with a copy of the old server. */
extern xmlrpc_server_info * 
xmlrpc_server_info_copy(xmlrpc_env *env, xmlrpc_server_info *src_server);

/* Delete a server info record. */
extern void
xmlrpc_server_info_free (xmlrpc_server_info *server);

/* We support rudimentary basic authentication. This lets us talk to Zope
** servers and similar critters. When called, this routine makes a copy
** of all the authentication information and passes it to future requests.
** Only the most-recently-set authentication information is used.
** (In general, you shouldn't write XML-RPC servers which require this
** kind of authentication--it confuses many client implementations.)
** If we fail, leave the xmlrpc_server_info record unchanged. */
void 
xmlrpc_server_info_set_basic_auth(xmlrpc_env *         const envP,
                                  xmlrpc_server_info * const serverP,
                                  const char *         const username,
                                  const char *         const password);


/*=========================================================================
**  xmlrpc_client_call
**=========================================================================
**  A synchronous XML-RPC client. Do not attempt to call any of these
**  functions from inside an asynchronous callback!
*/

xmlrpc_value * 
xmlrpc_client_call(xmlrpc_env * const envP,
                   const char * const server_url,
                   const char * const method_name,
                   const char * const format,
                   ...);

xmlrpc_value * 
xmlrpc_client_call_params (xmlrpc_env *   const env,
                           const char *   const server_url,
                           const char *   const method_name,
                           xmlrpc_value * const param_array);

xmlrpc_value * 
xmlrpc_client_call_server(xmlrpc_env *         const envP,
                          xmlrpc_server_info * const server,
                          const char *         const method_name,
                          const char *         const format, 
                          ...);

extern xmlrpc_value *
xmlrpc_client_call_server_params (xmlrpc_env *env,
                                  xmlrpc_server_info *server,
                                  char *method_name,
                                  xmlrpc_value *param_array);


/*=========================================================================
**  xmlrpc_client_call_asynch
**=========================================================================
**  An asynchronous XML-RPC client.
*/

/* Make an asynchronous XML-RPC call. We make internal copies of all
** arguments except user_data, so you can deallocate them safely as soon
** as you return. Errors will be passed to the callback. You will need
** to run the event loop somehow; see below.
** WARNING: If an error occurs while building the argument, the
** response handler will be called with a NULL param_array. */
void 
xmlrpc_client_call_asynch(const char * const server_url,
                          const char * const method_name,
                          xmlrpc_response_handler callback,
                          void *       const user_data,
                          const char * const format,
                          ...);

/* As above, but use an xmlrpc_server_info object. The server object can be
** safely destroyed as soon as this function returns. */
void 
xmlrpc_client_call_server_asynch(xmlrpc_server_info * const server,
                                 const char *         const method_name,
                                 xmlrpc_response_handler callback,
                                 void *               const user_data,
                                 const char *         const format,
                                 ...);

/* As above, but the parameter list is supplied as an xmlrpc_value
** containing an array.
*/
void
xmlrpc_client_call_asynch_params(const char *   const server_url,
                                 const char *   const method_name,
                                 xmlrpc_response_handler callback,
                                 void *         const user_data,
                                 xmlrpc_value * const paramArrayP);
    
/* As above, but use an xmlrpc_server_info object. The server object can be
** safely destroyed as soon as this function returns. */
void 
xmlrpc_client_call_server_asynch_params(
    xmlrpc_server_info * const server,
    const char *         const method_name,
    xmlrpc_response_handler callback,
    void *               const user_data,
    xmlrpc_value *       const paramArrayP);
    
/*=========================================================================
**  Event Loop Interface
**=========================================================================
**  These functions can be used to run the XML-RPC event loop. If you
**  don't like these, you can also run the libwww event loop directly.
*/

/* Finish all outstanding asynchronous calls. Alternatively, the loop
** will exit if someone calls xmlrpc_client_event_loop_end. */
extern void
xmlrpc_client_event_loop_finish_asynch(void);


/* Finish all outstanding asynchronous calls. */
extern void
xmlrpc_client_event_loop_finish_asynch_timeout(timeout_t const milliseconds);



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

#endif /* _XMLRPC_CLIENT_H_ */
