#ifndef HEADER_CURL_SSLUSE_H
#define HEADER_CURL_SSLUSE_H
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

#ifdef USE_OPENSSL
/*
 * This header should only be needed to get included by vtls.c and openssl.c
 */

#include "urldata.h"

CURLcode Curl_ossl_connect(struct connectdata *conn, int sockindex);
CURLcode Curl_ossl_connect_nonblocking(struct connectdata *conn,
                                       int sockindex,
                                       bool *done);

/* close a SSL connection */
void Curl_ossl_close(struct connectdata *conn, int sockindex);

/* tell OpenSSL to close down all open information regarding connections (and
   thus session ID caching etc) */
void Curl_ossl_close_all(struct Curl_easy *data);

/* Sets an OpenSSL engine */
CURLcode Curl_ossl_set_engine(struct Curl_easy *data, const char *engine);

/* function provided for the generic SSL-layer, called when a session id
   should be freed */
void Curl_ossl_session_free(void *ptr);

/* Sets engine as default for all SSL operations */
CURLcode Curl_ossl_set_engine_default(struct Curl_easy *data);

/* Build list of OpenSSL engines */
struct curl_slist *Curl_ossl_engines_list(struct Curl_easy *data);

int Curl_ossl_init(void);
void Curl_ossl_cleanup(void);

size_t Curl_ossl_version(char *buffer, size_t size);
int Curl_ossl_check_cxn(struct connectdata *cxn);
int Curl_ossl_shutdown(struct connectdata *conn, int sockindex);
bool Curl_ossl_data_pending(const struct connectdata *conn,
                            int connindex);

/* return 0 if a find random is filled in */
int Curl_ossl_random(struct Curl_easy *data, unsigned char *entropy,
                     size_t length);
void Curl_ossl_md5sum(unsigned char *tmp, /* input */
                      size_t tmplen,
                      unsigned char *md5sum /* output */,
                      size_t unused);
void Curl_ossl_sha256sum(const unsigned char *tmp, /* input */
                      size_t tmplen,
                      unsigned char *sha256sum /* output */,
                      size_t unused);

bool Curl_ossl_cert_status_request(void);

/* Support HTTPS-proxy */
#define HTTPS_PROXY_SUPPORT 1

/* Set the API backend definition to OpenSSL */
#define CURL_SSL_BACKEND CURLSSLBACKEND_OPENSSL

/* this backend supports the CAPATH option */
#define have_curlssl_ca_path 1

/* this backend supports CURLOPT_CERTINFO */
#define have_curlssl_certinfo 1

/* this backend supports CURLOPT_SSL_CTX_* */
#define have_curlssl_ssl_ctx 1

/* this backend supports CURLOPT_PINNEDPUBLICKEY */
#define have_curlssl_pinnedpubkey 1

/* API setup for OpenSSL */
#define curlssl_init Curl_ossl_init
#define curlssl_cleanup Curl_ossl_cleanup
#define curlssl_connect Curl_ossl_connect
#define curlssl_connect_nonblocking Curl_ossl_connect_nonblocking
#define curlssl_session_free(x) Curl_ossl_session_free(x)
#define curlssl_close_all Curl_ossl_close_all
#define curlssl_close Curl_ossl_close
#define curlssl_shutdown(x,y) Curl_ossl_shutdown(x,y)
#define curlssl_set_engine(x,y) Curl_ossl_set_engine(x,y)
#define curlssl_set_engine_default(x) Curl_ossl_set_engine_default(x)
#define curlssl_engines_list(x) Curl_ossl_engines_list(x)
#define curlssl_version Curl_ossl_version
#define curlssl_check_cxn Curl_ossl_check_cxn
#define curlssl_data_pending(x,y) Curl_ossl_data_pending(x,y)
#define curlssl_random(x,y,z) Curl_ossl_random(x,y,z)
#define curlssl_md5sum(a,b,c,d) Curl_ossl_md5sum(a,b,c,d)
#if (OPENSSL_VERSION_NUMBER >= 0x0090800fL) && !defined(OPENSSL_NO_SHA256)
#define curlssl_sha256sum(a,b,c,d) Curl_ossl_sha256sum(a,b,c,d)
#endif
#define curlssl_cert_status_request() Curl_ossl_cert_status_request()

#define DEFAULT_CIPHER_SELECTION \
  "ALL:!EXPORT:!EXPORT40:!EXPORT56:!aNULL:!LOW:!RC4:@STRENGTH"

#endif /* USE_OPENSSL */
#endif /* HEADER_CURL_SSLUSE_H */
