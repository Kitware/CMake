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


#ifndef  _XMLRPC_SERVER_ABYSS_H_
#define  _XMLRPC_SERVER_ABYSS_H_ 1

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct _TServer;

/*=========================================================================
**  XML-RPC Server (based on Abyss)
**=========================================================================
**  A simple XML-RPC server based on the Abyss web server. If errors
**  occur during server setup, the server will exit. In general, if you
**  want to use this API, you'll need to be familiar with Abyss.
**
**  There are two ways to use Abyss:
**    1) You can use the handy wrapper functions.
**    2) You can set up Abyss yourself, and install the appropriate
**       handlers manually.
*/

#define XMLRPC_SERVER_ABYSS_NO_FLAGS (0)




/*=========================================================================
**  Basic Abyss Server Functions
**=======================================================================*/

typedef void ((*runfirstFn)(void *));

typedef struct {
    const char *      config_file_name;
    xmlrpc_registry * registryP;
    runfirstFn        runfirst;
    void *            runfirst_arg;
} xmlrpc_server_abyss_parms;


#define XMLRPC_APSIZE(MBRNAME) \
    XMLRPC_STRUCTSIZE(xmlrpc_server_abyss_parms, MBRNAME)

/* XMLRPC_APSIZE(xyz) is the minimum size a struct xmlrpc_server_abyss_parms
   must be to include the 'xyz' member.  This is essential to forward and
   backward compatbility, as new members will be added to the end of the
   struct in future releases.  This is how the callee knows whether or
   not the caller is new enough to have supplied a certain parameter.
*/

void
xmlrpc_server_abyss(xmlrpc_env *                      const envP,
                    const xmlrpc_server_abyss_parms * const parms,
                    unsigned int                      const parm_size);

void
xmlrpc_server_abyss_set_handlers(struct _TServer * const srvP,
                                 xmlrpc_registry * const registryP);


/*=========================================================================
**  Handy Abyss Extensions
**=======================================================================*/

/* These are functions that have nothing to do with Xmlrpc-c, but provide
   convenient Abyss services beyond those provided by the Abyss library.
*/

/* Start an Abyss webserver running (previously created and
** initialized).  Under Unix, this routine will attempt to do a
** detaching fork, drop root privileges (if any) and create a pid
** file.  Under Windows, this routine merely starts the server.  This
** routine never returns.
**
** Once you call this routine, it is illegal to modify the server any
** more, including changing any method registry.
*/
void
xmlrpc_server_abyss_run(void);

/* Same as xmlrpc_server_abyss_run(), except you get to specify a "runfirst"
** function.  The server runs this just before executing the actual server
** function, after any daemonizing.  NULL for 'runfirst' means no runfirst
** function.  'runfirstArg' is the argument the server passes to the runfirst
** function.
**/
void 
xmlrpc_server_abyss_run_first(void (runfirst(void *)),
                              void * const runfirstArg);

/*=========================================================================
**  Method Registry
**=========================================================================
   These functions are for the built-in xmlrpc_server_abyss registry.
   It's usually simpler to skip all this and use the regular method
   registry services (from xmlrpc_server.h) to build a registry and
   pass it to xmlrpc_server_abyss.
*/

/* Call this function to create a new Abyss webserver with the default
** options and the built-in method registry.  If you've already
** initialized Abyss using Abyss functions, you can instead call
** xmlrpc_server_abyss_init_registry() to make it an Xmlrpc-c server.
** Or use a regular method registry and call
** xmlrpc_server_abyss_set_handlers().
**/
void 
xmlrpc_server_abyss_init(int          const flags, 
                         const char * const config_file);

/* This is called automatically by xmlrpc_server_abyss_init. */
void xmlrpc_server_abyss_init_registry (void);

/* Fetch the internal registry, if you happen to need it. 
   If you're using this, you really shouldn't be using the built-in
   registry at all.  It exists today only for backward compatibilty.
*/
extern xmlrpc_registry *
xmlrpc_server_abyss_registry (void);

/* A quick & easy shorthand for adding a method. Depending on
** how you've configured your copy of Abyss, it's probably not safe to
** call this method after calling xmlrpc_server_abyss_run. */
void xmlrpc_server_abyss_add_method (char *method_name,
                                     xmlrpc_method method,
                                     void *user_data);
    
/* As above, but provide documentation (see xmlrpc_registry_add_method_w_doc
** for more information). You should really use this one. */
extern void
xmlrpc_server_abyss_add_method_w_doc (char *method_name,
                                      xmlrpc_method method,
                                      void *user_data,
                                      char *signature,
                                      char *help);

/*=========================================================================
**  Content Handlers
**=======================================================================*/
/* Abyss contents handlers xmlrpc_server_abyss_rpc2_handler()
   and xmlrpc_server_abyss_default_handler() were available in older
   Xmlrpc-c, but starting with Release 1.01, they are not.  Instead,
   call xmlrpc_server_abyss_set_handlers() to install them.

   Alternatively, you can write your own handlers that do the same thing.
   It's not hard, and if you're writing low enough level Abyss code that
   you can't use xmlrpc_server_abyss_set_handlers(), you probably want to
   anyway.
*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
