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


#ifndef  _XMLRPC_SERVER_H_
#define  _XMLRPC_SERVER_H_ 1

#include <cmxmlrpc/xmlrpc.h>

#ifdef __cplusplus
extern "C" {
#endif

/*=========================================================================
**  XML-RPC Server Method Registry
**=========================================================================
**  A method registry maintains a list of functions, and handles
**  dispatching. To build an XML-RPC server, just add an XML-RPC protocol
**  driver.
**
**  Methods are C functions which take some combination of the following
**  parameters. All pointers except user_data belong to the library, and
**  must not be freed by the callback or used after the callback returns.
**
**  env:          An XML-RPC error-handling environment. No faults will be
**                set when the function is called. If an error occurs,
**                set an appropriate fault and return NULL. (If a fault is
**                set, the NULL return value will be enforced!)
**  host:         The 'Host:' header passed by the XML-RPC client, or NULL,
**                if no 'Host:' header has been provided.
**  method_name:  The name used to call this method.
**  user_data:    The user_data used to register this method.
**  param_array:  The parameters passed to this function, stored in an
**                XML-RPC array. You are *not* responsible for calling
**                xmlrpc_DECREF on this array.
**
**  Return value: If no fault has been set, the function must return a
**                valid xmlrpc_value. This will be serialized, returned
**                to the caller, and xmlrpc_DECREF'd.
*/

/* A function to call before invoking a method for doing things like access
** control or sanity checks.  If a fault is set from this function, the
** method will not be called and the fault will be returned. */
typedef void
(*xmlrpc_preinvoke_method)(xmlrpc_env *   env,
                           const char *   method_name,
                           xmlrpc_value * param_array,
                           void *         user_data);

/* An ordinary method. */
typedef xmlrpc_value *
(*xmlrpc_method)(xmlrpc_env *   env,
                 xmlrpc_value * param_array,
                 void *         user_data);

/* A default method to call if no method can be found. */
typedef xmlrpc_value *
(*xmlrpc_default_method)(xmlrpc_env *   env,
                         const char *   host,
                         const char *   method_name,
                         xmlrpc_value * param_array,
                         void *         user_data);

/* Our registry structure. This has no public members. */
typedef struct _xmlrpc_registry xmlrpc_registry;

/* Create a new method registry. */
xmlrpc_registry *
xmlrpc_registry_new(xmlrpc_env * env);

/* Delete a method registry. */
void
xmlrpc_registry_free(xmlrpc_registry * registry);

/* Disable introspection.  The xmlrpc_registry has introspection
** capability built-in.  If you want to make nosy people work harder,
** you can turn this off. */
void
xmlrpc_registry_disable_introspection(xmlrpc_registry * registry);

/* Register a method. The host parameter must be NULL (for now). You
** are responsible for owning and managing user_data. The registry
** will make internal copies of any other pointers it needs to
** keep around. */
void
xmlrpc_registry_add_method(xmlrpc_env *      env,
                           xmlrpc_registry * registry,
                           const char *      host,
                           const char *      method_name,
                           xmlrpc_method     method,
                           void *            user_data);

/* As above, but allow the user to supply introspection information. 
**
** Signatures use their own little description language. It consists
** of one-letter type code (similar to the ones used in xmlrpc_parse_value)
** for the result, a colon, and zero or more one-letter type codes for
** the parameters. For example:
**   i:ibdsAS86
** If a function has more than one possible prototype, separate them with
** commas:
**   i:,i:s,i:ii
** If the function signature can't be represented using this language,
** pass a single question mark:
**   ?
** Help strings are ASCII text, and may contain HTML markup. */
void
xmlrpc_registry_add_method_w_doc(xmlrpc_env *      env,
                                 xmlrpc_registry * registry,
                                 const char *      host,
                                 const char *      method_name,
                                 xmlrpc_method     method,
                                 void *            user_data,
                                 const char *      signature,
                                 const char *      help);

/* Given a registry, a host name, and XML data; parse the <methodCall>,
** find the appropriate method, call it, serialize the response, and
** return it as an xmlrpc_mem_block. Most errors will be serialized
** as <fault> responses. If a *really* bad error occurs, set a fault and
** return NULL. (Actually, we currently give up with a fatal error,
** but that should change eventually.)
** The caller is responsible for destroying the memory block. */
xmlrpc_mem_block *
xmlrpc_registry_process_call(xmlrpc_env *      const envP,
                             xmlrpc_registry * const registryP,
                             const char *      const host,
                             const char *      const xml_data,
                             size_t            const xml_len);

/* Define a default method for the specified registry.  This will be invoked
** if no other method matches.  The user_data pointer is property of the
** application, and will not be freed or manipulated by the registry. */
void
xmlrpc_registry_set_default_method(xmlrpc_env *          env,
                                   xmlrpc_registry *     registry,
                                   xmlrpc_default_method handler,
                                   void *                user_data);

/* Define a preinvoke method for the specified registry.  This function will
** be called before any method (either the default or a registered one) is
** invoked.  Applications can use this to do things like access control or
** sanity checks.  The user_data pointer is property of the application,
** and will not be freed or manipulated by the registry. */
void
xmlrpc_registry_set_preinvoke_method(xmlrpc_env *            env,
                                     xmlrpc_registry *       registry,
                                     xmlrpc_preinvoke_method method,
                                     void *                  user_data);

                    
#ifdef __cplusplus
}
#endif

#endif
