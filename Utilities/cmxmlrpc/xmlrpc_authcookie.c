/* Copyright (C) 2002 by jeff@ourexchange.net. All rights reserved.
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "xmlrpc.h"

/* set cookie function */
void xmlrpc_authcookie_set ( xmlrpc_env *env, 
                        const char *username, 
                        const char *password ) {
    char *unencoded;
    xmlrpc_mem_block *token;
    static char env_buffer[1024];
    const char* block;

    /* Check asserts. */
    XMLRPC_ASSERT_ENV_OK(env);
    XMLRPC_ASSERT_PTR_OK(username);
    XMLRPC_ASSERT_PTR_OK(password);

    /* Clear out memory. */
    unencoded = (char *) malloc ( sizeof ( char * ) );

    /* Create unencoded string/hash. */
    sprintf(unencoded, "%s:%s", username, password);

    /* Create encoded string. */
    token = xmlrpc_base64_encode_without_newlines(env, (unsigned char*)unencoded,
                                strlen(unencoded));
    XMLRPC_FAIL_IF_FAULT(env);

    /* Set HTTP_COOKIE_AUTH to the character representation of the
    ** encoded string. */
    block = XMLRPC_TYPED_MEM_BLOCK_CONTENTS(char, token);
    sprintf(env_buffer, "HTTP_COOKIE_AUTH=%s", block);
    putenv(env_buffer);

 cleanup:
    if (token) xmlrpc_mem_block_free(token);
}

char *xmlrpc_authcookie ( void ) {
    return getenv("HTTP_COOKIE_AUTH");
}
