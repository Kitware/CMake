#ifndef HEADER_CURL_VTLS_H
#define HEADER_CURL_VTLS_H
/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2016, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at https://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ***************************************************************************/
#include "curl_setup.h"

#include "openssl.h"        /* OpenSSL versions */
#include "gtls.h"           /* GnuTLS versions */
#include "nssg.h"           /* NSS versions */
#include "gskit.h"          /* Global Secure ToolKit versions */
#include "polarssl.h"       /* PolarSSL versions */
#include "axtls.h"          /* axTLS versions */
#include "cyassl.h"         /* CyaSSL versions */
#include "schannel.h"       /* Schannel SSPI version */
#include "darwinssl.h"      /* SecureTransport (Darwin) version */
#include "mbedtls.h"        /* mbedTLS versions */

#ifndef MAX_PINNED_PUBKEY_SIZE
#define MAX_PINNED_PUBKEY_SIZE 1048576 /* 1MB */
#endif

#ifndef MD5_DIGEST_LENGTH
#define MD5_DIGEST_LENGTH 16 /* fixed size */
#endif

#ifndef SHA256_DIGEST_LENGTH
#define SHA256_DIGEST_LENGTH 32 /* fixed size */
#endif

/* see https://tools.ietf.org/html/draft-ietf-tls-applayerprotoneg-04 */
#define ALPN_HTTP_1_1_LENGTH 8
#define ALPN_HTTP_1_1 "http/1.1"

/* set of helper macros for the backends to access the correct fields. For the
   proxy or for the remote host - to properly support HTTPS proxy */

#define SSL_IS_PROXY() (CURLPROXY_HTTPS == conn->http_proxy.proxytype && \
  ssl_connection_complete != conn->proxy_ssl[conn->sock[SECONDARYSOCKET] == \
  CURL_SOCKET_BAD ? FIRSTSOCKET : SECONDARYSOCKET].state)
#define SSL_SET_OPTION(var) (SSL_IS_PROXY() ? data->set.proxy_ssl.var : \
                             data->set.ssl.var)
#define SSL_CONN_CONFIG(var) (SSL_IS_PROXY() ?          \
  conn->proxy_ssl_config.var : conn->ssl_config.var)

bool Curl_ssl_config_matches(struct ssl_primary_config* data,
                             struct ssl_primary_config* needle);
bool Curl_clone_primary_ssl_config(struct ssl_primary_config *source,
                                   struct ssl_primary_config *dest);
void Curl_free_primary_ssl_config(struct ssl_primary_config* sslc);
int Curl_ssl_getsock(struct connectdata *conn, curl_socket_t *socks,
                     int numsocks);

int Curl_ssl_backend(void);

#ifdef USE_SSL
int Curl_ssl_init(void);
void Curl_ssl_cleanup(void);
CURLcode Curl_ssl_connect(struct connectdata *conn, int sockindex);
CURLcode Curl_ssl_connect_nonblocking(struct connectdata *conn,
                                      int sockindex,
                                      bool *done);
/* tell the SSL stuff to close down all open information regarding
   connections (and thus session ID caching etc) */
void Curl_ssl_close_all(struct Curl_easy *data);
void Curl_ssl_close(struct connectdata *conn, int sockindex);
CURLcode Curl_ssl_shutdown(struct connectdata *conn, int sockindex);
CURLcode Curl_ssl_set_engine(struct Curl_easy *data, const char *engine);
/* Sets engine as default for all SSL operations */
CURLcode Curl_ssl_set_engine_default(struct Curl_easy *data);
struct curl_slist *Curl_ssl_engines_list(struct Curl_easy *data);

/* init the SSL session ID cache */
CURLcode Curl_ssl_initsessions(struct Curl_easy *, size_t);
size_t Curl_ssl_version(char *buffer, size_t size);
bool Curl_ssl_data_pending(const struct connectdata *conn,
                           int connindex);
int Curl_ssl_check_cxn(struct connectdata *conn);

/* Certificate information list handling. */

void Curl_ssl_free_certinfo(struct Curl_easy *data);
CURLcode Curl_ssl_init_certinfo(struct Curl_easy *data, int num);
CURLcode Curl_ssl_push_certinfo_len(struct Curl_easy *data, int certnum,
                                    const char *label, const char *value,
                                    size_t valuelen);
CURLcode Curl_ssl_push_certinfo(struct Curl_easy *data, int certnum,
                                const char *label, const char *value);

/* Functions to be used by SSL library adaptation functions */

/* Lock session cache mutex.
 * Call this before calling other Curl_ssl_*session* functions
 * Caller should unlock this mutex as soon as possible, as it may block
 * other SSL connection from making progress.
 * The purpose of explicitly locking SSL session cache data is to allow
 * individual SSL engines to manage session lifetime in their specific way.
 */
void Curl_ssl_sessionid_lock(struct connectdata *conn);

/* Unlock session cache mutex */
void Curl_ssl_sessionid_unlock(struct connectdata *conn);

/* extract a session ID
 * Sessionid mutex must be locked (see Curl_ssl_sessionid_lock).
 * Caller must make sure that the ownership of returned sessionid object
 * is properly taken (e.g. its refcount is incremented
 * under sessionid mutex).
 */
bool Curl_ssl_getsessionid(struct connectdata *conn,
                           void **ssl_sessionid,
                           size_t *idsize, /* set 0 if unknown */
                           int sockindex);
/* add a new session ID
 * Sessionid mutex must be locked (see Curl_ssl_sessionid_lock).
 * Caller must ensure that it has properly shared ownership of this sessionid
 * object with cache (e.g. incrementing refcount on success)
 */
CURLcode Curl_ssl_addsessionid(struct connectdata *conn,
                               void *ssl_sessionid,
                               size_t idsize,
                               int sockindex);
/* Kill a single session ID entry in the cache
 * Sessionid mutex must be locked (see Curl_ssl_sessionid_lock).
 * This will call engine-specific curlssl_session_free function, which must
 * take sessionid object ownership from sessionid cache
 * (e.g. decrement refcount).
 */
void Curl_ssl_kill_session(struct curl_ssl_session *session);
/* delete a session from the cache
 * Sessionid mutex must be locked (see Curl_ssl_sessionid_lock).
 * This will call engine-specific curlssl_session_free function, which must
 * take sessionid object ownership from sessionid cache
 * (e.g. decrement refcount).
 */
void Curl_ssl_delsessionid(struct connectdata *conn, void *ssl_sessionid);

/* get N random bytes into the buffer */
CURLcode Curl_ssl_random(struct Curl_easy *data, unsigned char *buffer,
                         size_t length);
CURLcode Curl_ssl_md5sum(unsigned char *tmp, /* input */
                         size_t tmplen,
                         unsigned char *md5sum, /* output */
                         size_t md5len);
/* Check pinned public key. */
CURLcode Curl_pin_peer_pubkey(struct Curl_easy *data,
                              const char *pinnedpubkey,
                              const unsigned char *pubkey, size_t pubkeylen);

bool Curl_ssl_cert_status_request(void);

bool Curl_ssl_false_start(void);

#define SSL_SHUTDOWN_TIMEOUT 10000 /* ms */

#else
/* Set the API backend definition to none */
#define CURL_SSL_BACKEND CURLSSLBACKEND_NONE

/* When SSL support is not present, just define away these function calls */
#define Curl_ssl_init() 1
#define Curl_ssl_cleanup() Curl_nop_stmt
#define Curl_ssl_connect(x,y) CURLE_NOT_BUILT_IN
#define Curl_ssl_close_all(x) Curl_nop_stmt
#define Curl_ssl_close(x,y) Curl_nop_stmt
#define Curl_ssl_shutdown(x,y) CURLE_NOT_BUILT_IN
#define Curl_ssl_set_engine(x,y) CURLE_NOT_BUILT_IN
#define Curl_ssl_set_engine_default(x) CURLE_NOT_BUILT_IN
#define Curl_ssl_engines_list(x) NULL
#define Curl_ssl_send(a,b,c,d,e) -1
#define Curl_ssl_recv(a,b,c,d,e) -1
#define Curl_ssl_initsessions(x,y) CURLE_OK
#define Curl_ssl_version(x,y) 0
#define Curl_ssl_data_pending(x,y) 0
#define Curl_ssl_check_cxn(x) 0
#define Curl_ssl_free_certinfo(x) Curl_nop_stmt
#define Curl_ssl_connect_nonblocking(x,y,z) CURLE_NOT_BUILT_IN
#define Curl_ssl_kill_session(x) Curl_nop_stmt
#define Curl_ssl_random(x,y,z) ((void)x, CURLE_NOT_BUILT_IN)
#define Curl_ssl_cert_status_request() FALSE
#define Curl_ssl_false_start() FALSE
#endif

#endif /* HEADER_CURL_VTLS_H */
