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
** SUCH DAMAGE.
**
** There is more copyright information in the bottom half of this file. 
** Please see it for more details. */

#include "xmlrpc_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "abyss.h"

#include "xmlrpc.h"
#include "xmlrpc_server.h"
#include "xmlrpc_int.h"
#include "xmlrpc_server_abyss.h"
#include "xmlrpc_server_abyss_int.h"


/*=========================================================================
**  die_if_fault_occurred
**=========================================================================
**  If certain kinds of out-of-memory errors occur during server setup,
**  we want to quit and print an error.
*/

static void die_if_fault_occurred(xmlrpc_env *env) {
    if (env->fault_occurred) {
        fprintf(stderr, "Unexpected XML-RPC fault: %s (%d)\n",
                env->fault_string, env->fault_code);
        exit(1);
    }
}



/*=========================================================================
**  send_xml_data
**=========================================================================
**  Blast some XML data back to the client.
*/

static void 
send_xml_data (TSession * const r, 
               char *     const buffer, 
               uint64     const len) {

    const char * const http_cookie = NULL;
        /* This used to set http_cookie to getenv("HTTP_COOKIE"), but
           that doesn't make any sense -- environment variables are not
           appropriate for this.  So for now, cookie code is disabled.
           - Bryan 2004.10.03.
        */

    /* fwrite(buffer, sizeof(char), len, stderr); */

    /* XXX - Is it safe to chunk our response? */
    ResponseChunked(r);

    ResponseStatus(r, 200);
    
    if (http_cookie) {
        /* There's an auth cookie, so pass it back in the response. */

        char *cookie_response;
 
        cookie_response = malloc(10+strlen(http_cookie));
        sprintf(cookie_response, "auth=%s", http_cookie);
        
        /* Return abyss response. */
        ResponseAddField(r, "Set-Cookie", cookie_response);

        free(cookie_response);
    }   
 
  
    ResponseContentType(r, "text/xml; charset=\"utf-8\"");
    ResponseContentLength(r, len);
    
    ResponseWrite(r);
    
    HTTPWrite(r, buffer, len);
    HTTPWriteEnd(r);
}



/*=========================================================================
**  send_error
**=========================================================================
**  Send an error back to the client.
*/

static void
send_error(TSession *   const abyssSessionP, 
           unsigned int const status) {

    ResponseStatus(abyssSessionP, (uint16) status);
    ResponseError(abyssSessionP);
}



/*=========================================================================
**  get_buffer_data
**=========================================================================
**  Extract some data from the TConn's underlying input buffer. Do not
**  extract more than 'max'.
*/

static void
get_buffer_data(TSession * const r, 
                int        const max, 
                char **    const out_start, 
                int *      const out_len) {

    /* Point to the start of our data. */
    *out_start = &r->conn->buffer[r->conn->bufferpos];

    /* Decide how much data to retrieve. */
    *out_len = r->conn->buffersize - r->conn->bufferpos;
    if (*out_len > max)
        *out_len = max;

    /* Update our buffer position. */
    r->conn->bufferpos += *out_len;
}



/*=========================================================================
**  get_body
**=========================================================================
**  Slurp the body of the request into an xmlrpc_mem_block.
*/

static void
getBody(xmlrpc_env *        const envP,
        TSession *          const abyssSessionP,
        unsigned int        const contentSize,
        xmlrpc_mem_block ** const bodyP) {
/*----------------------------------------------------------------------------
   Get the entire body from the Abyss session and return it as the new
   memblock *bodyP.

   The first chunk of the body may already be in Abyss's buffer.  We
   retrieve that before reading more.
-----------------------------------------------------------------------------*/
    xmlrpc_mem_block * body;

    body = xmlrpc_mem_block_new(envP, 0);
    if (!envP->fault_occurred) {
        unsigned int bytesRead;
        char * chunkPtr;
        int chunkLen;

        bytesRead = 0;

        while (!envP->fault_occurred && bytesRead < contentSize) {
            get_buffer_data(abyssSessionP, contentSize - bytesRead, 
                            &chunkPtr, &chunkLen);
            bytesRead += chunkLen;

            XMLRPC_TYPED_MEM_BLOCK_APPEND(char, envP, body, 
                                          chunkPtr, chunkLen);
            
            if (bytesRead < contentSize) {
                /* Get the next chunk of data from the connection into the
                   buffer 
                */
                abyss_bool succeeded;
            
                /* Reset our read buffer & flush data from previous reads. */
                ConnReadInit(abyssSessionP->conn);
            
                /* Read more network data into our buffer. If we encounter
                   a timeout, exit immediately. We're very forgiving about
                   the timeout here. We allow a full timeout per network
                   read, which would allow somebody to keep a connection
                   alive nearly indefinitely. But it's hard to do anything
                   intelligent here without very complicated code. 
                */
                succeeded = ConnRead(abyssSessionP->conn,
                                     abyssSessionP->server->timeout);
                if (!succeeded)
                    xmlrpc_env_set_fault_formatted(
                        envP, XMLRPC_TIMEOUT_ERROR, "Timed out waiting for "
                        "client to send its POST data");
            }
        }
        if (envP->fault_occurred)
            xmlrpc_mem_block_free(body);
        else
            *bodyP = body;
    }
}



static void
storeCookies(TSession *     const httpRequestP,
             unsigned int * const httpErrorP) {
/*----------------------------------------------------------------------------
   Get the cookie settings from the HTTP headers and remember them for
   use in responses.
-----------------------------------------------------------------------------*/
    const char * const cookie = RequestHeaderValue(httpRequestP, "cookie");
    if (cookie) {
        /* 
           Setting the value in an environment variable doesn't make
           any sense.  So for now, cookie code is disabled.
           -Bryan 04.10.03.

        setenv("HTTP_COOKIE", cookie, 1);
        */
    }
    /* TODO: parse HTTP_COOKIE to find auth pair, if there is one */

    *httpErrorP = 0;
}




static void
validateContentType(TSession *     const httpRequestP,
                    unsigned int * const httpErrorP) {
/*----------------------------------------------------------------------------
   If the client didn't specify a content-type of "text/xml", return      
   "400 Bad Request".  We can't allow the client to default this header,
   because some firewall software may rely on all XML-RPC requests
   using the POST method and a content-type of "text/xml". 
-----------------------------------------------------------------------------*/
    const char * const content_type =
        RequestHeaderValue(httpRequestP, "content-type");
    if (content_type == NULL || strcmp(content_type, "text/xml") != 0)
        *httpErrorP = 400;
    else
        *httpErrorP = 0;
}



static void
processContentLength(TSession *     const httpRequestP,
                     unsigned int * const inputLenP,
                     unsigned int * const httpErrorP) {
/*----------------------------------------------------------------------------
  Make sure the content length is present and non-zero.  This is
  technically required by XML-RPC, but we only enforce it because we
  don't want to figure out how to safely handle HTTP < 1.1 requests
  without it.  If the length is missing, return "411 Length Required". 
-----------------------------------------------------------------------------*/
    const char * const content_length = 
        RequestHeaderValue(httpRequestP, "content-length");
    if (content_length == NULL)
        *httpErrorP = 411;
    else {
        int const contentLengthValue = atoi(content_length);
        if (contentLengthValue <= 0)
            *httpErrorP = 400;
        else {
            *httpErrorP = 0;
            *inputLenP = (unsigned int)contentLengthValue;
        }
    }
}


/****************************************************************************
    Abyss handlers (to be registered with and called by Abyss)
****************************************************************************/

/* XXX - This variable is *not* currently threadsafe. Once the server has
** been started, it must be treated as read-only. */
static xmlrpc_registry *global_registryP;

static const char * trace_abyss;

static void
processCall(TSession * const abyssSessionP,
            int        const inputLen) {
/*----------------------------------------------------------------------------
   Handle an RPC request.  This is an HTTP request that has the proper form
   to be one of our RPCs.
-----------------------------------------------------------------------------*/
    xmlrpc_env env;

    if (trace_abyss)
        fprintf(stderr, "xmlrpc_server_abyss RPC2 handler processing RPC.\n");

    xmlrpc_env_init(&env);

    /* SECURITY: Make sure our content length is legal.
       XXX - We can cast 'inputLen' because we know it's >= 0, yes? 
    */
    if ((size_t) inputLen > xmlrpc_limit_get(XMLRPC_XML_SIZE_LIMIT_ID))
        xmlrpc_env_set_fault_formatted(
            &env, XMLRPC_LIMIT_EXCEEDED_ERROR,
            "XML-RPC request too large (%d bytes)", inputLen);
    else {
        xmlrpc_mem_block *body;
        /* Read XML data off the wire. */
        getBody(&env, abyssSessionP, inputLen, &body);
        if (!env.fault_occurred) {
            xmlrpc_mem_block * output;
            /* Process the RPC. */
            output = xmlrpc_registry_process_call(
                &env, global_registryP, NULL, 
                XMLRPC_MEMBLOCK_CONTENTS(char, body),
                XMLRPC_MEMBLOCK_SIZE(char, body));
            if (!env.fault_occurred) {
            /* Send our the result. */
                send_xml_data(abyssSessionP, 
                              XMLRPC_MEMBLOCK_CONTENTS(char, output),
                              XMLRPC_MEMBLOCK_SIZE(char, output));
                
                XMLRPC_MEMBLOCK_FREE(char, output);
            }
            XMLRPC_MEMBLOCK_FREE(char, body);
        }
    }
    if (env.fault_occurred) {
        if (env.fault_code == XMLRPC_TIMEOUT_ERROR)
            send_error(abyssSessionP, 408); /* 408 Request Timeout */
        else
            send_error(abyssSessionP, 500); /* 500 Internal Server Error */
    }

    xmlrpc_env_clean(&env);
}



/*=========================================================================
**  xmlrpc_server_abyss_rpc2_handler
**=========================================================================
**  This handler processes all requests to '/RPC2'. See the header for
**  more documentation.
*/

xmlrpc_bool 
xmlrpc_server_abyss_rpc2_handler (TSession * const r) {

    xmlrpc_bool retval;

    if (trace_abyss)
        fprintf(stderr, "xmlrpc_server_abyss RPC2 handler called.\n");

    /* We handle only requests to /RPC2, the default XML-RPC URL.
       Everything else we pass through to other handlers. 
    */
    if (strcmp(r->uri, "/RPC2") != 0)
        retval = FALSE;
    else {
        retval = TRUE;

        /* We understand only the POST HTTP method.  For anything else, return
           "405 Method Not Allowed". 
        */
        if (r->method != m_post)
            send_error(r, 405);
        else {
            unsigned int httpError;
            storeCookies(r, &httpError);
            if (httpError)
                send_error(r, httpError);
            else {
                unsigned int httpError;
                validateContentType(r, &httpError);
                if (httpError)
                    send_error(r, httpError);
                else {
                    unsigned int httpError;
                    int inputLen;

                    processContentLength(r, &inputLen, &httpError);
                    if (httpError)
                        send_error(r, httpError);

                    processCall(r, inputLen);
                }
            }
        }
    }
    if (trace_abyss)
        fprintf(stderr, "xmlrpc_server_abyss RPC2 handler returning.\n");
    return retval;
}



/*=========================================================================
**  xmlrpc_server_abyss_default_handler
**=========================================================================
**  This handler returns a 404 Not Found for all requests. See the header
**  for more documentation.
*/

xmlrpc_bool 
xmlrpc_server_abyss_default_handler (TSession * const r) {
    send_error(r, 404);

    return TRUE;
}



/**************************************************************************
**
** The code below was adapted from the main.c file of the Abyss webserver
** project. In addition to the other copyrights on this file, the following
** code is also under this copyright:
**
** Copyright (C) 2000 by Moez Mahfoudh <mmoez@bigfoot.com>.
** All rights reserved.
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
** SUCH DAMAGE.
**
**************************************************************************/

#include <time.h>
#include <fcntl.h>

#ifdef _WIN32
#include <io.h>
#else
/* Must check this
#include <sys/io.h>
*/
#endif  /* _WIN32 */

#ifdef _UNIX
#include <sys/signal.h>
#include <sys/wait.h>
#include <grp.h>
#endif


#ifdef _UNIX
static void 
sigterm(int const sig) {
    TraceExit("Signal %d received. Exiting...\n",sig);
}
#endif


#ifdef _UNIX
static void 
sigchld(int const sig ATTR_UNUSED) {
/*----------------------------------------------------------------------------
   This is a signal handler for a SIGCHLD signal (which informs us that
   one of our child processes has terminated).

   We respond by reaping the zombie process.

   Implementation note: In some systems, just setting the signal handler
   to SIG_IGN (ignore signal) does this.  In others, it doesn't.
-----------------------------------------------------------------------------*/
    pid_t pid;
    int status;
    
    /* Reap defunct children until there aren't any more. */
    for (;;) {
        pid = waitpid( (pid_t) -1, &status, WNOHANG );
    
        /* none left */
        if (pid==0)
            break;
    
        if (pid<0) {
            /* because of ptrace */
            if (errno==EINTR)   
                continue;
        
            break;
        }
    }
}
#endif /* _UNIX */

static TServer globalSrv;
    /* When you use the old interface (xmlrpc_server_abyss_init(), etc.),
       this is the Abyss server to which they refer.  Obviously, there can be
       only one Abyss server per program using this interface.
    */


void 
xmlrpc_server_abyss_init(int          const flags ATTR_UNUSED, 
                         const char * const config_file) {

    DateInit();
    MIMETypeInit();

    ServerCreate(&globalSrv, "XmlRpcServer", 8080, DEFAULT_DOCS, NULL);
    
    ConfReadServerFile(config_file, &globalSrv);

    xmlrpc_server_abyss_init_registry();
        /* Installs /RPC2 handler and default handler that use the
           built-in registry.
        */

    ServerInit(&globalSrv);
}



static void
setupSignalHandlers(void) {
#ifdef _UNIX
    struct sigaction mysigaction;
    
    sigemptyset(&mysigaction.sa_mask);
    mysigaction.sa_flags = 0;

    /* These signals abort the program, with tracing */
    mysigaction.sa_handler = sigterm;
    sigaction(SIGTERM, &mysigaction, NULL);
    sigaction(SIGINT,  &mysigaction, NULL);
    sigaction(SIGHUP,  &mysigaction, NULL);
    sigaction(SIGUSR1, &mysigaction, NULL);

    /* This signal indicates connection closed in the middle */
    mysigaction.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &mysigaction, NULL);
    
    /* This signal indicates a child process (request handler) has died */
    mysigaction.sa_handler = sigchld;
    sigaction(SIGCHLD, &mysigaction, NULL);
#endif
}    



static void
runServer(TServer *  const srvP,
          runfirstFn const runfirst,
          void *     const runfirstArg) {

    setupSignalHandlers();

#ifdef _UNIX
    /* Become a daemon */
    switch (fork()) {
    case 0:
        break;
    case -1:
        TraceExit("Unable to become a daemon");
    default:
        exit(0);
    };
    
    setsid();

    /* Change the current user if we are root */
    if (getuid()==0) {
        if (srvP->uid == (uid_t)-1)
            TraceExit("Can't run under root privileges.  "
                      "Please add a User option in your "
                      "Abyss configuration file.");

#ifdef HAVE_SETGROUPS   
        if (setgroups(0,NULL)==(-1))
            TraceExit("Failed to setup the group.");
        if (srvP->gid != (gid_t)-1)
            if (setgid(srvP->gid)==(-1))
                TraceExit("Failed to change the group.");
#endif
        
        if (setuid(srvP->uid) == -1)
            TraceExit("Failed to change the user.");
    };
    
    if (srvP->pidfile!=(-1)) {
        char z[16];
    
        sprintf(z,"%d",getpid());
        FileWrite(&srvP->pidfile,z,strlen(z));
        FileClose(&srvP->pidfile);
    };
#endif
    
    /* We run the user supplied runfirst after forking, but before accepting
       connections (helpful when running with threads)
    */
    if (runfirst)
        runfirst(runfirstArg);

    ServerRun(srvP);

    /* We can't exist here because ServerRun doesn't return */
    XMLRPC_ASSERT(FALSE);
}



void 
xmlrpc_server_abyss_run_first(runfirstFn const runfirst,
                              void *     const runfirstArg) {
    
    runServer(&globalSrv, runfirst, runfirstArg);
}



void 
xmlrpc_server_abyss_run(void) {
    runServer(&globalSrv, NULL, NULL);
}



void
xmlrpc_server_abyss_set_handlers(TServer *         const srvP,
                                 xmlrpc_registry * const registryP) {

    /* Abyss ought to have a way to register with a handler an argument
       that gets passed to the handler every time it is called.  That's
       where we should put the registry handle.  But we don't find such
       a thing in Abyss, so we use the global variable 'global_registryP'.
    */
    global_registryP = registryP;

    trace_abyss = getenv("XMLRPC_TRACE_ABYSS");
                                 
    ServerAddHandler(srvP, xmlrpc_server_abyss_rpc2_handler);
    ServerDefaultHandler(srvP, xmlrpc_server_abyss_default_handler);
}



void
xmlrpc_server_abyss(xmlrpc_env *                      const envP,
                    const xmlrpc_server_abyss_parms * const parmsP,
                    unsigned int                      const parm_size) {
 
    XMLRPC_ASSERT_ENV_OK(envP);

    if (parm_size < XMLRPC_APSIZE(registryP))
        xmlrpc_env_set_fault_formatted(
            envP, XMLRPC_INTERNAL_ERROR,
            "You must specify members at least up through "
            "'registryP' in the server parameters argument.  "
            "That would mean the parameter size would be >= %u "
            "but you specified a size of %u",
            XMLRPC_APSIZE(registryP), parm_size);
    else {
        TServer srv;
        runfirstFn runfirst;
        void * runfirstArg;

        DateInit();
        MIMETypeInit();
        
        ServerCreate(&srv, "XmlRpcServer", 8080, DEFAULT_DOCS, NULL);
        
        ConfReadServerFile(parmsP->config_file_name, &srv);
        
        xmlrpc_server_abyss_set_handlers(&srv, parmsP->registryP);
        
        ServerInit(&srv);

        if (parm_size >= XMLRPC_APSIZE(runfirst_arg)) {
            runfirst    = parmsP->runfirst;
            runfirstArg = parmsP->runfirst_arg;
        } else {
            runfirst    = NULL;
            runfirstArg = NULL;
        }
        runServer(&srv, runfirst, runfirstArg);
    }
}



/*=========================================================================
**  XML-RPC Server Method Registry
**=========================================================================
**  A simple front-end to our method registry.
*/

/* XXX - This variable is *not* currently threadsafe. Once the server has
** been started, it must be treated as read-only. */
static xmlrpc_registry *builtin_registryP;

void 
xmlrpc_server_abyss_init_registry(void) {

    /* This used to just create the registry and Caller would be
       responsible for adding the handlers that use it.

       But that isn't very modular -- the handlers and registry go
       together; there's no sense in using the built-in registry and
       not the built-in handlers because if you're custom building
       something, you can just make your own regular registry.  So now
       we tie them together, and we don't export our handlers.  
    */
    xmlrpc_env env;

    xmlrpc_env_init(&env);
    builtin_registryP = xmlrpc_registry_new(&env);
    die_if_fault_occurred(&env);
    xmlrpc_env_clean(&env);

    xmlrpc_server_abyss_set_handlers(&globalSrv, builtin_registryP);
}



xmlrpc_registry *
xmlrpc_server_abyss_registry(void) {

    /* This is highly deprecated.  If you want to mess with a registry,
       make your own with xmlrpc_registry_new() -- don't mess with the
       internal one.
    */
    return builtin_registryP;
}



/* A quick & easy shorthand for adding a method. */
void 
xmlrpc_server_abyss_add_method (char *        const method_name,
                                xmlrpc_method const method,
                                void *        const user_data) {
    xmlrpc_env env;

    xmlrpc_env_init(&env);
    xmlrpc_registry_add_method(&env, builtin_registryP, NULL, method_name,
                               method, user_data);
    die_if_fault_occurred(&env);
    xmlrpc_env_clean(&env);
}



void
xmlrpc_server_abyss_add_method_w_doc (char *        const method_name,
                                      xmlrpc_method const method,
                                      void *        const user_data,
                                      char *        const signature,
                                      char *        const help) {

    xmlrpc_env env;
    xmlrpc_env_init(&env);
    xmlrpc_registry_add_method_w_doc(
        &env, builtin_registryP, NULL, method_name,
        method, user_data, signature, help);
    die_if_fault_occurred(&env);
    xmlrpc_env_clean(&env);    
}
