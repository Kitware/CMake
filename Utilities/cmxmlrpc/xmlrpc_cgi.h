/* Copyright (C) 2001 by Eric Kidd. All rights reserved.
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

#ifndef  _XMLRPC_CGI_H_
#define  _XMLRPC_CGI_H_ 1

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*=========================================================================
**  XML-RPC CGI Server
**=========================================================================
**  A simple XML-RPC server based on the Common Gateway Interface.
*/

#define XMLRPC_CGI_NO_FLAGS (0)

/* Initialize the CGI server library. */
extern void
xmlrpc_cgi_init (int flags);

/* Fetch the internal registry, if you happen to need it. */
extern xmlrpc_registry *
xmlrpc_cgi_registry (void);

/* Register a new method. */
extern void
xmlrpc_cgi_add_method (char *method_name,
                       xmlrpc_method method,
                       void *user_data);

/* As above, but provide documentation (see xmlrpc_registry_add_method_w_doc
** for more information). You should really use this one. */
extern void
xmlrpc_cgi_add_method_w_doc (char *method_name,
                             xmlrpc_method method,
                             void *user_data,
                             char *signature,
                             char *help);

/* Parse the XML-RPC call, invoke the appropriate method, and send the
** response over the network. In future releases, we reserve the right to
** time out when reading data. For now, we rely on the webserver to blow us
** away. */
extern void
xmlrpc_cgi_process_call (void);

/* Clean up any internal data structures before exiting. */
extern void
xmlrpc_cgi_cleanup (void);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _XMLRPC_CGI_H_ */
