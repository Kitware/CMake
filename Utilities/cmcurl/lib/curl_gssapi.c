/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 2011 - 2015, Daniel Stenberg, <daniel@haxx.se>, et al.
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

#ifdef HAVE_GSSAPI

#include "curl_gssapi.h"
#include "sendf.h"

static char spnego_oid_bytes[] = "\x2b\x06\x01\x05\x05\x02";
gss_OID_desc Curl_spnego_mech_oid = { 6, &spnego_oid_bytes };
static char krb5_oid_bytes[] = "\x2a\x86\x48\x86\xf7\x12\x01\x02\x02";
gss_OID_desc Curl_krb5_mech_oid = { 9, &krb5_oid_bytes };

OM_uint32 Curl_gss_init_sec_context(
    struct SessionHandle *data,
    OM_uint32 *minor_status,
    gss_ctx_id_t *context,
    gss_name_t target_name,
    gss_OID mech_type,
    gss_channel_bindings_t input_chan_bindings,
    gss_buffer_t input_token,
    gss_buffer_t output_token,
    const bool mutual_auth,
    OM_uint32 *ret_flags)
{
  OM_uint32 req_flags = GSS_C_REPLAY_FLAG;

  if(mutual_auth)
    req_flags |= GSS_C_MUTUAL_FLAG;

  if(data->set.gssapi_delegation & CURLGSSAPI_DELEGATION_POLICY_FLAG) {
#ifdef GSS_C_DELEG_POLICY_FLAG
    req_flags |= GSS_C_DELEG_POLICY_FLAG;
#else
    infof(data, "warning: support for CURLGSSAPI_DELEGATION_POLICY_FLAG not "
        "compiled in\n");
#endif
  }

  if(data->set.gssapi_delegation & CURLGSSAPI_DELEGATION_FLAG)
    req_flags |= GSS_C_DELEG_FLAG;

  return gss_init_sec_context(minor_status,
                              GSS_C_NO_CREDENTIAL, /* cred_handle */
                              context,
                              target_name,
                              mech_type,
                              req_flags,
                              0, /* time_req */
                              input_chan_bindings,
                              input_token,
                              NULL, /* actual_mech_type */
                              output_token,
                              ret_flags,
                              NULL /* time_rec */);
}

/*
 * Curl_gss_log_error()
 *
 * This is used to log a GSS-API error status.
 *
 * Parameters:
 *
 * data    [in] - The session handle.
 * status  [in] - The status code.
 * prefix  [in] - The prefix of the log message.
 */
void Curl_gss_log_error(struct SessionHandle *data, OM_uint32 status,
                        const char *prefix)
{
  OM_uint32 maj_stat;
  OM_uint32 min_stat;
  OM_uint32 msg_ctx = 0;
  gss_buffer_desc status_string;
  char buf[1024];
  size_t len;

  snprintf(buf, sizeof(buf), "%s", prefix);
  len = strlen(buf);
  do {
    maj_stat = gss_display_status(&min_stat,
                                  status,
                                  GSS_C_MECH_CODE,
                                  GSS_C_NO_OID,
                                  &msg_ctx,
                                  &status_string);
    if(sizeof(buf) > len + status_string.length + 1) {
      snprintf(buf + len, sizeof(buf) - len,
        ": %s", (char*)status_string.value);
      len += status_string.length;
    }
    gss_release_buffer(&min_stat, &status_string);
  } while(!GSS_ERROR(maj_stat) && msg_ctx != 0);

  infof(data, "%s\n", buf);
}

#endif /* HAVE_GSSAPI */
