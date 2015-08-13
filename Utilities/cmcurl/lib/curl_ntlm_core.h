#ifndef HEADER_CURL_NTLM_CORE_H
#define HEADER_CURL_NTLM_CORE_H
/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2015, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at http://curl.haxx.se/docs/copyright.html.
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

#if defined(USE_NTLM)

#if !defined(USE_WINDOWS_SSPI) || defined(USE_WIN32_CRYPTO)

#ifdef USE_OPENSSL
#  if !defined(OPENSSL_VERSION_NUMBER) && \
      !defined(HEADER_SSL_H) && !defined(HEADER_MD5_H)
#    error "curl_ntlm_core.h shall not be included before OpenSSL headers."
#  endif
#  ifdef OPENSSL_NO_MD4
#    define USE_NTRESPONSES 0
#    define USE_NTLM2SESSION 0
#    define USE_NTLM_V2 0
#  endif
#endif

/* Define USE_NTRESPONSES to 1 in order to make the type-3 message include
 * the NT response message. */
#ifndef USE_NTRESPONSES
#define USE_NTRESPONSES 1
#endif

/* Define USE_NTLM2SESSION to 1 in order to make the type-3 message include the
   NTLM2Session response message, requires USE_NTRESPONSES defined to 1 and a
   Crypto engine that we have curl_ssl_md5sum() for. */
#if !defined(USE_NTLM2SESSION) && USE_NTRESPONSES && !defined(USE_WIN32_CRYPTO)
#define USE_NTLM2SESSION 1
#endif

/* Define USE_NTLM_V2 to 1 in order to allow the type-3 message to include the
   LMv2 and NTLMv2 response messages, requires USE_NTRESPONSES defined to 1
   and support for 64-bit integers. */
#if !defined(USE_NTLM_V2) && USE_NTRESPONSES && (CURL_SIZEOF_CURL_OFF_T > 4)
#define USE_NTLM_V2 1
#endif

void Curl_ntlm_core_lm_resp(const unsigned char *keys,
                            const unsigned char *plaintext,
                            unsigned char *results);

CURLcode Curl_ntlm_core_mk_lm_hash(struct SessionHandle *data,
                                   const char *password,
                                   unsigned char *lmbuffer /* 21 bytes */);

#if USE_NTRESPONSES
CURLcode Curl_ntlm_core_mk_nt_hash(struct SessionHandle *data,
                                   const char *password,
                                   unsigned char *ntbuffer /* 21 bytes */);

#if USE_NTLM_V2 && !defined(USE_WINDOWS_SSPI)

CURLcode Curl_hmac_md5(const unsigned char *key, unsigned int keylen,
                       const unsigned char *data, unsigned int datalen,
                       unsigned char *output);

CURLcode Curl_ntlm_core_mk_ntlmv2_hash(const char *user, size_t userlen,
                                       const char *domain, size_t domlen,
                                       unsigned char *ntlmhash,
                                       unsigned char *ntlmv2hash);

CURLcode  Curl_ntlm_core_mk_ntlmv2_resp(unsigned char *ntlmv2hash,
                                        unsigned char *challenge_client,
                                        struct ntlmdata *ntlm,
                                        unsigned char **ntresp,
                                        unsigned int *ntresp_len);

CURLcode  Curl_ntlm_core_mk_lmv2_resp(unsigned char *ntlmv2hash,
                                      unsigned char *challenge_client,
                                      unsigned char *challenge_server,
                                      unsigned char *lmresp);

#endif /* USE_NTLM_V2 && !USE_WINDOWS_SSPI */

#endif /* USE_NTRESPONSES */

#endif /* !USE_WINDOWS_SSPI || USE_WIN32_CRYPTO */

#endif /* USE_NTLM */

#endif /* HEADER_CURL_NTLM_CORE_H */
