/* Copyright information is at the end of the file */
#ifndef  _XMLRPC_TRANSPORT_H_
#define  _XMLRPC_TRANSPORT_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#if defined(HAVE_PTHREADS)
#  include "xmlrpc_pthreads.h" /* For threading helpers. */
#endif

struct call_info;

struct clientTransport;

/*=========================================================================
**  Transport function type declarations.
**=========================================================================
*/
typedef void (*transport_create)(
    xmlrpc_env *              const envP,
    int                       const flags,
    const char *              const appname,
    const char *              const appversion,
    struct clientTransport ** const handlePP);
    
typedef void (*transport_destroy)(
    struct clientTransport * const clientTransportP);

typedef void (*transport_asynch_complete)(
    struct call_info * const callInfoP,
    xmlrpc_mem_block * const responseXmlP,
    xmlrpc_env         const env);

typedef void (*transport_send_request)(
    xmlrpc_env *             const envP, 
    struct clientTransport * const clientTransportP,
    xmlrpc_server_info *     const serverP,
    xmlrpc_mem_block *       const xmlP,
    transport_asynch_complete      complete,
    struct call_info *       const callInfoP);

typedef void (*transport_call)(
    xmlrpc_env *             const envP,
    struct clientTransport * const clientTransportP,
    xmlrpc_server_info *     const serverP,
    xmlrpc_mem_block *       const xmlP,
    struct call_info *       const callInfoP,
    xmlrpc_mem_block **      const responsePP);

enum timeoutType {timeout_no, timeout_yes};
typedef void (*transport_finish_asynch)(
    struct clientTransport * const clientTransportP,
    enum timeoutType         const timeoutType,
    timeout_t                const timeout);


struct clientTransportOps {

    transport_create        create;
    transport_destroy       destroy;
    transport_send_request  send_request;
    transport_call          call;
    transport_finish_asynch finish_asynch;
};

/*=========================================================================
**  Transport Helper Functions and declarations.
**=========================================================================
*/
#if defined(HAVE_PTHREADS)
typedef struct _running_thread_info
{
        struct _running_thread_info * Next;
        struct _running_thread_info * Last;

        pthread_t _thread;
} running_thread_info;


/* list of running Async callback functions. */
typedef struct _running_thread_list
{
        running_thread_info * AsyncThreadHead;
        running_thread_info * AsyncThreadTail;
} running_thread_list;

/* MRB-WARNING: Only call when you have successfully
**     acquired the Lock/Unlock mutex! */
void register_asynch_thread (running_thread_list *list, pthread_t *thread);

/* MRB-WARNING: Only call when you have successfully
**     acquired the Lock/Unlock mutex! */
void unregister_asynch_thread (running_thread_list *list, pthread_t *thread);
#endif


#ifdef __cplusplus
}
#endif

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


#endif
