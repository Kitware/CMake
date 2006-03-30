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

#undef PACKAGE
#undef VERSION

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

#include "bool.h"
#include "mallocvar.h"
#include "xmlrpc.h"
#include "xmlrpc_int.h"
#include "xmlrpc_client.h"
#include "xmlrpc_client_int.h"
/* transport_config.h defines XMLRPC_DEFAULT_TRANSPORT,
    MUST_BUILD_WININET_CLIENT, MUST_BUILD_CURL_CLIENT,
    MUST_BUILD_LIBWWW_CLIENT 
*/
#include "transport_config.h"

#if MUST_BUILD_WININET_CLIENT
#include "xmlrpc_wininet_transport.h"
#endif
#if MUST_BUILD_CURL_CLIENT
#include "xmlrpc_curl_transport.h"
#endif
#if MUST_BUILD_LIBWWW_CLIENT
#include "xmlrpc_libwww_transport.h"
#endif

struct xmlrpc_client {
/*----------------------------------------------------------------------------
   This represents a client object.
-----------------------------------------------------------------------------*/
    struct clientTransport * transportP;
};



typedef struct call_info
{
    /* These fields are used when performing asynchronous calls.
    ** The _asynch_data_holder contains server_url, method_name and
    ** param_array, so it's the only thing we need to free. */
    xmlrpc_value *_asynch_data_holder;
    char *server_url;
    char *method_name;
    xmlrpc_value *param_array;
    xmlrpc_response_handler callback;
    void *user_data;

    /* The serialized XML data passed to this call. We keep this around
    ** for use by our source_anchor field. */
    xmlrpc_mem_block *serialized_xml;
} call_info;

static bool clientInitialized = FALSE;

/*=========================================================================
**  Initialization and Shutdown
**=========================================================================
*/

static struct clientTransportOps clientTransportOps;

static struct xmlrpc_client client;
    /* Some day, we need to make this dynamically allocated, so there can
       be more than one client per program and just generally to provide
       a cleaner interface.
    */

extern void
xmlrpc_client_init(int          const flags,
                   const char * const appname,
                   const char * const appversion) {

    struct xmlrpc_clientparms clientparms;

    /* As our interface does not allow for failure, we just fail silently ! */
    
    xmlrpc_env env;
    xmlrpc_env_init(&env);

    clientparms.transport = XMLRPC_DEFAULT_TRANSPORT;

    xmlrpc_client_init2(&env, flags,
                        appname, appversion,
                        &clientparms, XMLRPC_CPSIZE(transport));

    xmlrpc_env_clean(&env);
}



const char * 
xmlrpc_client_get_default_transport(xmlrpc_env * const env ATTR_UNUSED) {

    return XMLRPC_DEFAULT_TRANSPORT;
}



static void
setupTransport(xmlrpc_env * const envP,
               const char * const transportName) {

    if (FALSE) {
    }
#if MUST_BUILD_WININET_CLIENT
    else if (strcmp(transportName, "wininet") == 0)
        clientTransportOps = xmlrpc_wininet_transport_ops;
#endif
#if MUST_BUILD_CURL_CLIENT
    else if (strcmp(transportName, "curl") == 0)
        clientTransportOps = xmlrpc_curl_transport_ops;
    else if (strcmp(transportName, "libcurl") == 0)
        clientTransportOps = xmlrpc_curl_transport_ops;
#endif
#if MUST_BUILD_LIBWWW_CLIENT
    else if (strcmp(transportName, "libwww") == 0)
        clientTransportOps = xmlrpc_libwww_transport_ops;
#endif
    else
        xmlrpc_env_set_fault_formatted(
            envP, XMLRPC_INTERNAL_ERROR, 
            "Unrecognized XML transport name '%s'", transportName);
}



void 
xmlrpc_client_init2(xmlrpc_env *                const envP,
                    int                         const flags,
                    const char *                const appname,
                    const char *                const appversion,
                    struct xmlrpc_clientparms * const clientparmsP,
                    unsigned int                const parm_size) {

    if (clientInitialized)
        xmlrpc_env_set_fault_formatted(
            envP, XMLRPC_INTERNAL_ERROR, 
            "Xmlrpc-c client instance has already been initialized "
            "(need to call xmlrpc_client_cleanup() before you can "
            "reinitialize).");
    else {
        const char * transportName;

        if (parm_size < XMLRPC_CPSIZE(transport) ||
            clientparmsP->transport == NULL) {
            /* He didn't specify a transport.  Use the default */
            transportName = xmlrpc_client_get_default_transport(envP);
        } else
            transportName = clientparmsP->transport;

        if (!envP->fault_occurred) {
            setupTransport(envP, transportName);
            if (!envP->fault_occurred) {
                clientTransportOps.create(envP, flags, appname, appversion,
                                          &client.transportP);
                if (!envP->fault_occurred)
                    clientInitialized = TRUE;
            }
        }
    }
}



void 
xmlrpc_client_cleanup() {

    XMLRPC_ASSERT(clientInitialized);

    clientTransportOps.destroy(client.transportP);
    
    clientInitialized = FALSE;
}



static void 
call_info_free(call_info * const callInfoP) {

    /* Assume the worst.. That only parts of the call_info are valid. */

    XMLRPC_ASSERT_PTR_OK(callInfoP);

    /* If this has been allocated, we're responsible for destroying it. */
    if (callInfoP->_asynch_data_holder)
        xmlrpc_DECREF(callInfoP->_asynch_data_holder);

    /* Now we can blow away the XML data. */
    if (callInfoP->serialized_xml)
         xmlrpc_mem_block_free(callInfoP->serialized_xml);

    free(callInfoP);
}



static void
call_info_new(xmlrpc_env *         const envP,
              xmlrpc_server_info * const server,
              const char *         const method_name,
              xmlrpc_value *       const argP,
              call_info **         const callInfoPP) {
/*----------------------------------------------------------------------------
   Create a call_info object.  A call_info object represents an XML-RPC
   call.
-----------------------------------------------------------------------------*/
    call_info * callInfoP;

    XMLRPC_ASSERT_PTR_OK(argP);
    XMLRPC_ASSERT_PTR_OK(callInfoPP);

    if (method_name == NULL)
        xmlrpc_env_set_fault_formatted(
            envP, XMLRPC_INTERNAL_ERROR,
            "method name argument is NULL pointer");
    else if (server == NULL)
        xmlrpc_env_set_fault_formatted(
            envP, XMLRPC_INTERNAL_ERROR,
            "server info argument is NULL pointer");
    else {
        MALLOCVAR(callInfoP);
        if (callInfoP == NULL)
            xmlrpc_env_set_fault_formatted(
                envP, XMLRPC_INTERNAL_ERROR,
                "Couldn't allocate memory for xmlrpc_call_info");
        else {
            xmlrpc_mem_block * callXmlP;

        /* Clear contents. */
            memset(callInfoP, 0, sizeof(*callInfoP));
        
            /* Make the XML for our call */
            callXmlP = XMLRPC_MEMBLOCK_NEW(char, envP, 0);
            if (!envP->fault_occurred) {
                xmlrpc_serialize_call(envP, callXmlP, method_name, argP);
                if (!envP->fault_occurred) {
                    xmlrpc_traceXml("XML-RPC CALL", 
                                    XMLRPC_MEMBLOCK_CONTENTS(char, callXmlP),
                                    (unsigned int)XMLRPC_MEMBLOCK_SIZE(char, callXmlP));
        
                    callInfoP->serialized_xml = callXmlP;

                    *callInfoPP = callInfoP;
                }
                if (envP->fault_occurred)
                    XMLRPC_MEMBLOCK_FREE(char, callXmlP);
            }
            if (envP->fault_occurred)
                free(callInfoP);
        }
    }
}



static void
clientCallServerParams(xmlrpc_env *             const envP,
                       struct clientTransport * const transportP,
                       xmlrpc_server_info *     const serverP,
                       const char *             const methodName,
                       xmlrpc_value *           const paramArrayP,
                       xmlrpc_value **          const resultPP) {

    call_info * callInfoP;
    
    if (!clientInitialized)
        xmlrpc_env_set_fault_formatted(
            envP, XMLRPC_INTERNAL_ERROR, 
            "Xmlrpc-c client instance has not been initialized "
            "(need to call xmlrpc_client_init2()).");
    else {
        call_info_new(envP, serverP, methodName, paramArrayP, &callInfoP);
        if (!envP->fault_occurred) {
            xmlrpc_mem_block * respXmlP;
        
            clientTransportOps.call(envP, transportP, serverP, 
                                    callInfoP->serialized_xml, callInfoP, 
                                    &respXmlP);
            if (!envP->fault_occurred) {
                xmlrpc_traceXml("XML-RPC RESPONSE", 
                                XMLRPC_MEMBLOCK_CONTENTS(char, respXmlP),
                                (unsigned int)XMLRPC_MEMBLOCK_SIZE(char, respXmlP));
            
                *resultPP = xmlrpc_parse_response(
                    envP,
                    XMLRPC_MEMBLOCK_CONTENTS(char, respXmlP),
                    XMLRPC_MEMBLOCK_SIZE(char, respXmlP));
                XMLRPC_MEMBLOCK_FREE(char, respXmlP);
            }                    
            call_info_free(callInfoP);
        }
    }
}



xmlrpc_value * 
xmlrpc_client_call_params(xmlrpc_env *   const envP,
                          const char *   const serverUrl,
                          const char *   const methodName,
                          xmlrpc_value * const paramArrayP) {

    xmlrpc_value *retval;

    XMLRPC_ASSERT_ENV_OK(envP);
    XMLRPC_ASSERT_PTR_OK(serverUrl);

    if (!clientInitialized)
        xmlrpc_env_set_fault_formatted(
            envP, XMLRPC_INTERNAL_ERROR, 
            "Xmlrpc-c client instance has not been initialized "
            "(need to call xmlrpc_client_init2()).");
    else {
        xmlrpc_server_info * serverP;
        
        /* Build a server info object and make our call. */
        serverP = xmlrpc_server_info_new(envP, serverUrl);
        if (!envP->fault_occurred) {
            clientCallServerParams(envP, client.transportP, serverP, 
                                   methodName, paramArrayP,
                                   &retval);

            xmlrpc_server_info_free(serverP);
        }
    }
        
    if (!envP->fault_occurred)
        XMLRPC_ASSERT_VALUE_OK(retval);

    return retval;
}



static xmlrpc_value * 
xmlrpc_client_call_va(xmlrpc_env * const envP,
                      const char * const server_url,
                      const char * const method_name,
                      const char * const format,
                      va_list            args) {

    xmlrpc_value * argP;
    xmlrpc_value * retval = 0;
    xmlrpc_env argenv;
    const char * suffix;

    XMLRPC_ASSERT_ENV_OK(envP);
    XMLRPC_ASSERT_PTR_OK(format);

    /* Build our argument value. */
    xmlrpc_env_init(&argenv);
    xmlrpc_build_value_va(&argenv, format, args, &argP, &suffix);
    if (argenv.fault_occurred) {
        xmlrpc_env_set_fault_formatted(
            envP, argenv.fault_code, "Invalid RPC arguments.  "
            "The format argument must indicate a single array, and the "
            "following arguments must correspond to that format argument.  "
            "The failure is: %s",
            argenv.fault_string);
        xmlrpc_env_clean(&argenv);
    } else {
        XMLRPC_ASSERT_VALUE_OK(argP);
        
        if (*suffix != '\0')
            xmlrpc_env_set_fault_formatted(
                envP, XMLRPC_INTERNAL_ERROR, "Junk after the argument "
                "specifier: '%s'.  There must be exactly one arument.",
                suffix);
        else {
            /* Perform the actual XML-RPC call. */
            retval = xmlrpc_client_call_params(
                envP, server_url, method_name, argP);
            if (!envP->fault_occurred)
                XMLRPC_ASSERT_VALUE_OK(retval);
        }
        xmlrpc_DECREF(argP);
    }
    return retval;
}



xmlrpc_value * 
xmlrpc_client_call(xmlrpc_env * const envP,
                   const char *       const server_url,
                   const char *       const method_name,
                   const char *       const format,
                   ...) {

    xmlrpc_value * result;
    va_list args;

    va_start(args, format);
    result = xmlrpc_client_call_va(envP, server_url,
                                   method_name, format, args);
    va_end(args);

    return result;
}



xmlrpc_value * 
xmlrpc_client_call_server(xmlrpc_env *         const envP,
                          xmlrpc_server_info * const serverP,
                          const char *         const methodName,
                          const char *         const format, 
                          ...) {

    va_list args;
    xmlrpc_value * paramArrayP;
    xmlrpc_value * retval;
    const char * suffix;

    XMLRPC_ASSERT_ENV_OK(envP);
    XMLRPC_ASSERT_PTR_OK(format);

    /* Build our argument */
    va_start(args, format);
    xmlrpc_build_value_va(envP, format, args, &paramArrayP, &suffix);
    va_end(args);

    if (!envP->fault_occurred) {
        if (*suffix != '\0')
            xmlrpc_env_set_fault_formatted(
                envP, XMLRPC_INTERNAL_ERROR, "Junk after the argument "
                "specifier: '%s'.  There must be exactly one arument.",
                suffix);
        else
            clientCallServerParams(envP, client.transportP, serverP, 
                                   methodName, paramArrayP, 
                                   &retval);

        xmlrpc_DECREF(paramArrayP);
    }
    return retval;
}


void 
xmlrpc_client_event_loop_finish_asynch(void) {
    XMLRPC_ASSERT(clientInitialized);
    clientTransportOps.finish_asynch(client.transportP, timeout_no, 0);
}



void 
xmlrpc_client_event_loop_finish_asynch_timeout(timeout_t const timeout) {
    XMLRPC_ASSERT(clientInitialized);
    clientTransportOps.finish_asynch(client.transportP, timeout_yes, timeout);
}



static void 
call_info_set_asynch_data(xmlrpc_env *   const env,
                          call_info *    const info,
                          const char *   const server_url,
                          const char *   const method_name,
                          xmlrpc_value * const argP,
                          xmlrpc_response_handler callback,
                          void *         const user_data) {

    xmlrpc_value *holder;

    XMLRPC_ASSERT_ENV_OK(env);
    XMLRPC_ASSERT_PTR_OK(info);
    XMLRPC_ASSERT(info->_asynch_data_holder == NULL);
    XMLRPC_ASSERT_PTR_OK(server_url);
    XMLRPC_ASSERT_PTR_OK(method_name);
    XMLRPC_ASSERT_VALUE_OK(argP);

    /* Install our callback and user_data.
    ** (We're not responsible for destroying the user_data.) */
    info->callback  = callback;
    info->user_data = user_data;

    /* Build an XML-RPC data structure to hold our other data. This makes
    ** copies of server_url and method_name, and increments the reference
    ** to the argument *argP. */
    holder = xmlrpc_build_value(env, "(ssV)",
                                server_url, method_name, argP);
    XMLRPC_FAIL_IF_FAULT(env);

    /* Parse the newly-allocated structure into our public member variables.
    ** This doesn't make any new references, so we can dispose of the whole
    ** thing by DECREF'ing the one master reference. Nifty, huh? */
    xmlrpc_parse_value(env, holder, "(ssV)",
                       &info->server_url,
                       &info->method_name,
                       &info->param_array);
    XMLRPC_FAIL_IF_FAULT(env);

    /* Hand over ownership of the holder to the call_info struct. */
    info->_asynch_data_holder = holder;
    holder = NULL;

 cleanup:
    if (env->fault_occurred) {
        if (holder)
            xmlrpc_DECREF(holder);
    }
}

/*=========================================================================
**  xmlrpc_server_info
**=========================================================================
*/

xmlrpc_server_info *
xmlrpc_server_info_new (xmlrpc_env * const env,
                        const char * const server_url) {

    xmlrpc_server_info *server;
    char *url_copy;

    /* Error-handling preconditions. */
    url_copy = NULL;

    XMLRPC_ASSERT_ENV_OK(env);
    XMLRPC_ASSERT_PTR_OK(server_url);

    /* Allocate our memory blocks. */
    server = (xmlrpc_server_info*) malloc(sizeof(xmlrpc_server_info));
    XMLRPC_FAIL_IF_NULL(server, env, XMLRPC_INTERNAL_ERROR,
                        "Couldn't allocate memory for xmlrpc_server_info");
    memset(server, 0, sizeof(xmlrpc_server_info));
    url_copy = (char*) malloc(strlen(server_url) + 1);
    XMLRPC_FAIL_IF_NULL(url_copy, env, XMLRPC_INTERNAL_ERROR,
                        "Couldn't allocate memory for server URL");

    /* Build our object. */
    strcpy(url_copy, server_url);
    server->_server_url = url_copy;
    server->_http_basic_auth = NULL;

 cleanup:
    if (env->fault_occurred) {
        if (url_copy)
            free(url_copy);
        if (server)
            free(server);
        return NULL;
    }
    return server;
}

xmlrpc_server_info * xmlrpc_server_info_copy(xmlrpc_env *env,
                                             xmlrpc_server_info *aserver)
{
    xmlrpc_server_info *server;
    char *url_copy, *auth_copy;

    XMLRPC_ASSERT_ENV_OK(env);
    XMLRPC_ASSERT_PTR_OK(aserver);

    /* Error-handling preconditions. */
    url_copy = NULL;
    auth_copy = NULL;

    /* Allocate our memory blocks. */
    server = (xmlrpc_server_info*) malloc(sizeof(xmlrpc_server_info));
    XMLRPC_FAIL_IF_NULL(server, env, XMLRPC_INTERNAL_ERROR,
                        "Couldn't allocate memory for xmlrpc_server_info");
    url_copy = (char*) malloc(strlen(aserver->_server_url) + 1);
    XMLRPC_FAIL_IF_NULL(url_copy, env, XMLRPC_INTERNAL_ERROR,
                        "Couldn't allocate memory for server URL");
    auth_copy = (char*) malloc(strlen(aserver->_http_basic_auth) + 1);
    XMLRPC_FAIL_IF_NULL(auth_copy, env, XMLRPC_INTERNAL_ERROR,
                        "Couldn't allocate memory for authentication info");

    /* Build our object. */
    strcpy(url_copy, aserver->_server_url);
    server->_server_url = url_copy;
    strcpy(auth_copy, aserver->_http_basic_auth);
    server->_http_basic_auth = auth_copy;

    cleanup:
    if (env->fault_occurred) {
        if (url_copy)
            free(url_copy);
        if (auth_copy)
            free(auth_copy);
        if (server)
            free(server);
        return NULL;
    }
    return server;

}

void xmlrpc_server_info_free (xmlrpc_server_info *server)
{
    XMLRPC_ASSERT_PTR_OK(server);
    XMLRPC_ASSERT(server->_server_url != XMLRPC_BAD_POINTER);

    if (server->_http_basic_auth)
        free(server->_http_basic_auth);
    free(server->_server_url);
    server->_server_url = XMLRPC_BAD_POINTER;
    free(server);
}

/*=========================================================================
**  xmlrpc_client_call_asynch
**=========================================================================
*/

void 
xmlrpc_client_call_asynch(const char * const serverUrl,
                          const char * const methodName,
                          xmlrpc_response_handler callback,
                          void *       const userData,
                          const char * const format,
                          ...) {

    xmlrpc_env env;
    va_list args;
    xmlrpc_value * paramArrayP;
    const char * suffix;

    xmlrpc_env_init(&env);

    XMLRPC_ASSERT_PTR_OK(serverUrl);
    XMLRPC_ASSERT_PTR_OK(format);

    /* Build our argument array. */
    va_start(args, format);
    xmlrpc_build_value_va(&env, format, args, &paramArrayP, &suffix);
    va_end(args);
    if (env.fault_occurred) {
        /* Unfortunately, we have no way to return an error and the
           regular callback for a failed RPC is designed to have the
           parameter array passed to it.  This was probably an oversight
           of the original asynch design, but now we have to be as
           backward compatible as possible, so we do this:
        */
        (*callback)(serverUrl, methodName, NULL, userData, &env, NULL);
    } else {
        if (*suffix != '\0')
            xmlrpc_env_set_fault_formatted(
                &env, XMLRPC_INTERNAL_ERROR, "Junk after the argument "
                "specifier: '%s'.  There must be exactly one arument.",
                suffix);
        else {
            xmlrpc_server_info * serverP;
            serverP = xmlrpc_server_info_new(&env, serverUrl);
            if (!env.fault_occurred) {
                xmlrpc_client_call_server_asynch_params(
                    serverP, methodName, callback, userData, 
                    paramArrayP);
            }
            xmlrpc_server_info_free(serverP);
        }
        if (env.fault_occurred)
            (*callback)(serverUrl, methodName, paramArrayP, userData,
                        &env, NULL);
        xmlrpc_DECREF(paramArrayP);
    }

    xmlrpc_env_clean(&env);
}



void
xmlrpc_client_call_asynch_params(const char *   const serverUrl,
                                 const char *   const methodName,
                                 xmlrpc_response_handler callback,
                                 void *         const userData,
                                 xmlrpc_value * const paramArrayP) {

    xmlrpc_env env;
    xmlrpc_server_info *serverP;

    xmlrpc_env_init(&env);

    XMLRPC_ASSERT_PTR_OK(serverUrl);

    serverP = xmlrpc_server_info_new(&env, serverUrl);
    if (!env.fault_occurred) {
        xmlrpc_client_call_server_asynch_params(
            serverP, methodName, callback, userData, paramArrayP);

        xmlrpc_server_info_free(serverP);
    }

    if (env.fault_occurred)
        /* We have no way to return failure; we report the failure
           as it happened after we successfully started the RPC.
        */
        (*callback)(serverUrl, methodName, paramArrayP, userData,
                    &env, NULL);

    xmlrpc_env_clean(&env);
}



void 
xmlrpc_client_call_server_asynch(xmlrpc_server_info * const serverP,
                                 const char *         const methodName,
                                 xmlrpc_response_handler callback,
                                 void *               const userData,
                                 const char *         const format,
                                 ...) {

    xmlrpc_env env;
    va_list args;
    xmlrpc_value * paramArrayP;
    const char * suffix;

    xmlrpc_env_init(&env);

    XMLRPC_ASSERT_PTR_OK(format);

    /* Build our parameter array. */
    va_start(args, format);
    xmlrpc_build_value_va(&env, format, args, &paramArrayP, &suffix);
    va_end(args);
    if (env.fault_occurred) {
        /* Unfortunately, we have no way to return an error and the
           regular callback for a failed RPC is designed to have the
           parameter array passed to it.  This was probably an oversight
           of the original asynch design, but now we have to be as
           backward compatible as possible, so we do this:
        */
        (*callback)(serverP->_server_url, methodName, NULL, userData, 
                    &env, NULL);
    } else {
        if (*suffix != '\0')
            xmlrpc_env_set_fault_formatted(
                &env, XMLRPC_INTERNAL_ERROR, "Junk after the argument "
                "specifier: '%s'.  There must be exactly one arument.",
                suffix);
        else {
            xmlrpc_client_call_server_asynch_params(
                serverP, methodName, callback, userData, paramArrayP);
        }
        xmlrpc_DECREF(paramArrayP);
    }

    if (env.fault_occurred)
        (*callback)(serverP->_server_url, methodName, paramArrayP, userData,
                    &env, NULL);

    xmlrpc_env_clean(&env);
}



static void
asynchComplete(call_info *        const callInfoP,
               xmlrpc_mem_block * const responseXmlP,
               xmlrpc_env         const transportEnv) {
/*----------------------------------------------------------------------------
   Complete an asynchronous XML-RPC call request.

   This includes calling the user's RPC completion routine.

   'transportEnv' describes the an error that the transport
   encountered in processing the call.  If the transport successfully
   sent the call to the server and processed the response but the
   server failed the call, 'transportEnv' indicates no error, and the
   response in *callInfoP might very well indicate that the server
   failed the request.
-----------------------------------------------------------------------------*/
    xmlrpc_env env;
    xmlrpc_value * responseP = 0;

    xmlrpc_env_init(&env);

    if (transportEnv.fault_occurred)
        xmlrpc_env_set_fault_formatted(
            &env, transportEnv.fault_code,
            "Client transport failed to execute the RPC.  %s",
            transportEnv.fault_string);

    if (!env.fault_occurred)
        responseP = xmlrpc_parse_response(
            &env,
            XMLRPC_MEMBLOCK_CONTENTS(char, responseXmlP),
            XMLRPC_MEMBLOCK_SIZE(char, responseXmlP));

    /* Call the user's callback function with the result */
    (*callInfoP->callback)(callInfoP->server_url, 
                           callInfoP->method_name, 
                           callInfoP->param_array,
                           callInfoP->user_data, &env, responseP);

    if (!env.fault_occurred)
        xmlrpc_DECREF(responseP);

    call_info_free(callInfoP);

    xmlrpc_env_clean(&env);
}



static void
sendRequest(xmlrpc_env *             const envP,
            struct clientTransport * const transportP,
            xmlrpc_server_info *     const serverP,
            const char *             const methodName,
            xmlrpc_response_handler        responseHandler,
            void *                   const userData,
            xmlrpc_value *           const argP) {

    call_info * callInfoP;

    call_info_new(envP, serverP, methodName, argP, &callInfoP);
    if (!envP->fault_occurred) {
        call_info_set_asynch_data(envP, callInfoP, 
                                  serverP->_server_url, methodName,
                                  argP, responseHandler, userData);
        if (!envP->fault_occurred)
            clientTransportOps.send_request(
                envP, transportP, serverP, callInfoP->serialized_xml,
                &asynchComplete, callInfoP);

        if (envP->fault_occurred)
            call_info_free(callInfoP);
        else {
            /* asynchComplete() will free *callInfoP */
        }
    }
    if (envP->fault_occurred) {
        /* Transport did not start the call.  Report the call complete
           (with error) now.
        */
        (*responseHandler)(serverP->_server_url, methodName, argP, userData,
                           envP, NULL);
    } else {
        /* The transport will call *responseHandler() when it has completed
           the call
        */
    }
}



void 
xmlrpc_client_call_server_asynch_params(
    xmlrpc_server_info * const serverP,
    const char *         const methodName,
    xmlrpc_response_handler    responseHandler,
    void *               const userData,
    xmlrpc_value *       const argP) {
    xmlrpc_env env;

    xmlrpc_env_init(&env);

    XMLRPC_ASSERT_PTR_OK(serverP);
    XMLRPC_ASSERT_PTR_OK(methodName);
    XMLRPC_ASSERT_PTR_OK(responseHandler);
    XMLRPC_ASSERT_VALUE_OK(argP);

    if (!clientInitialized)
        xmlrpc_env_set_fault_formatted(
            &env, XMLRPC_INTERNAL_ERROR, 
            "Xmlrpc-c client instance has not been initialized "
            "(need to call xmlrpc_client_init2()).");
    else
        sendRequest(&env, client.transportP, serverP, 
                    methodName, responseHandler, userData, 
                    argP);

    xmlrpc_env_clean(&env);
}



void 
xmlrpc_server_info_set_basic_auth(xmlrpc_env *         const envP,
                                  xmlrpc_server_info * const serverP,
                                  const char *         const username,
                                  const char *         const password) {

    size_t username_len, password_len, raw_token_len;
    char *raw_token;
    xmlrpc_mem_block *token;
    char *token_data, *auth_type, *auth_header;
    size_t token_len, auth_type_len, auth_header_len;

    /* Error-handling preconditions. */
    token = NULL;
    auth_header = NULL;

    XMLRPC_ASSERT_ENV_OK(envP);
    XMLRPC_ASSERT_PTR_OK(serverP);
    XMLRPC_ASSERT_PTR_OK(username);
    XMLRPC_ASSERT_PTR_OK(password);

    /* Calculate some lengths. */
    username_len = strlen(username);
    password_len = strlen(password);
    raw_token_len = username_len + password_len + 1;

    /* Build a raw token of the form 'username:password'. */
    raw_token = (char*) malloc(raw_token_len + 1);
    XMLRPC_FAIL_IF_NULL(raw_token, envP, XMLRPC_INTERNAL_ERROR,
                        "Couldn't allocate memory for auth token");
    strcpy(raw_token, username);
    raw_token[username_len] = ':';
    strcpy(&raw_token[username_len + 1], password);

    /* Encode our raw token using Base64. */
    token = xmlrpc_base64_encode_without_newlines(envP, 
                                                  (unsigned char*) raw_token,
                                                  raw_token_len);
    XMLRPC_FAIL_IF_FAULT(envP);
    token_data = XMLRPC_TYPED_MEM_BLOCK_CONTENTS(char, token);
    token_len = XMLRPC_TYPED_MEM_BLOCK_SIZE(char, token);

    /* Build our actual header value. (I hate string processing in C.) */
    auth_type = "Basic ";
    auth_type_len = strlen(auth_type);
    auth_header_len = auth_type_len + token_len;
    auth_header = (char*) malloc(auth_header_len + 1);
    XMLRPC_FAIL_IF_NULL(auth_header, envP, XMLRPC_INTERNAL_ERROR,
                        "Couldn't allocate memory for auth header");
    memcpy(auth_header, auth_type, auth_type_len);
    memcpy(&auth_header[auth_type_len], token_data, token_len);
    auth_header[auth_header_len] = '\0';

    /* Clean up any pre-existing authentication information, and install
    ** the new value. */
    if (serverP->_http_basic_auth)
        free(serverP->_http_basic_auth);
    serverP->_http_basic_auth = auth_header;

 cleanup:
    if (raw_token)
        free(raw_token);
    if (token)
        xmlrpc_mem_block_free(token);
    if (envP->fault_occurred) {
        if (auth_header)
            free(auth_header);
    }
}

