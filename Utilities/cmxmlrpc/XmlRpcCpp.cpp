// Copyright (C) 2001 by Eric Kidd. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. The name of the author may not be used to endorse or promote products
//    derived from this software without specific prior written permission. 
//  
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
// SUCH DAMAGE.


#include "XmlRpcCpp.h"


//=========================================================================
//  XmlRpcFault Methods
//=========================================================================

XmlRpcFault::XmlRpcFault (const XmlRpcFault &fault) {
    xmlrpc_env_init(&mFault);
    xmlrpc_env_set_fault(&mFault,
                         fault.mFault.fault_code,
                         fault.mFault.fault_string);
}

XmlRpcFault::XmlRpcFault (const int faultCode, const string faultString) {
    xmlrpc_env_init(&mFault);
    xmlrpc_env_set_fault(&mFault, faultCode,
                         const_cast<char*>(faultString.c_str()));
}
 
XmlRpcFault::XmlRpcFault (const xmlrpc_env *env) {
    if (!env->fault_string)
        throw XmlRpcFault(XMLRPC_INTERNAL_ERROR,
                          "Tried to create empty fault");
    xmlrpc_env_init(&mFault);
    xmlrpc_env_set_fault(&mFault, env->fault_code,
                         const_cast<char*>(env->fault_string));
}

XmlRpcFault::~XmlRpcFault (void) {
    xmlrpc_env_clean(&mFault);
}

string XmlRpcFault::getFaultString (void) const {
    XMLRPC_ASSERT(mFault.fault_occurred);
    return string(mFault.fault_string);
}


//=========================================================================
//  XmlRpcEnv Methods
//=========================================================================

XmlRpcEnv::XmlRpcEnv (const XmlRpcEnv &env) {
    xmlrpc_env_init(&mEnv);
    if (env.hasFaultOccurred())
        xmlrpc_env_set_fault(&mEnv,
                             env.mEnv.fault_code,
                             env.mEnv.fault_string);
}

XmlRpcFault XmlRpcEnv::getFault (void) const {
    return XmlRpcFault(&mEnv);
}

void XmlRpcEnv::throwMe (void) const {
    throw XmlRpcFault(&mEnv);
}


//=========================================================================
//  XmlRpcValue Methods
//=========================================================================

// If the user doesn't tell us what kind of value to create, use
// a false boolean value as the default.
XmlRpcValue::XmlRpcValue (void) {
    XmlRpcEnv env;
    mValue = xmlrpc_build_value(env, "b", (xmlrpc_bool) 0);
    env.throwIfFaultOccurred();
}

XmlRpcValue XmlRpcValue::makeInt (const XmlRpcValue::int32 i) {
    XmlRpcEnv env;
    xmlrpc_value *value = xmlrpc_build_value(env, "i", i);
    env.throwIfFaultOccurred();
    return XmlRpcValue(value, CONSUME_REFERENCE);
}

XmlRpcValue XmlRpcValue::makeBool (const bool b) {
    XmlRpcEnv env;
    xmlrpc_value *value = xmlrpc_build_value(env, "b", (xmlrpc_bool) b);
    env.throwIfFaultOccurred();
    return XmlRpcValue(value, CONSUME_REFERENCE);
}

XmlRpcValue XmlRpcValue::makeDouble (const double d) {
    XmlRpcEnv env;
    xmlrpc_value *value = xmlrpc_build_value(env, "d", d);
    env.throwIfFaultOccurred();
    return XmlRpcValue(value, CONSUME_REFERENCE);
}

XmlRpcValue XmlRpcValue::makeDateTime (const string& dateTime) {
    XmlRpcEnv env;
    xmlrpc_value *value;
    const char *data = dateTime.c_str(); // Make sure we're not using wchar_t.
    value = xmlrpc_build_value(env, "8", data);
    env.throwIfFaultOccurred();
    return XmlRpcValue(value, CONSUME_REFERENCE);    
}

XmlRpcValue XmlRpcValue::makeString (const string& str) {
    XmlRpcEnv env;
    const char *data = str.data();      // Make sure we're not using wchar_t.
    size_t size = str.size();
    xmlrpc_value *value = xmlrpc_build_value(env, "s#", data, size);
    env.throwIfFaultOccurred();
    return XmlRpcValue(value, CONSUME_REFERENCE);    
}

XmlRpcValue XmlRpcValue::makeString (const char *const str) {
    XmlRpcEnv env;
    xmlrpc_value *value = xmlrpc_build_value(env, "s", str);
    env.throwIfFaultOccurred();
    return XmlRpcValue(value, CONSUME_REFERENCE);    
}

XmlRpcValue XmlRpcValue::makeString (const char *const str, size_t len) {
    XmlRpcEnv env;
    xmlrpc_value *value = xmlrpc_build_value(env, "s#", str, len);
    env.throwIfFaultOccurred();
    return XmlRpcValue(value, CONSUME_REFERENCE);    
}

XmlRpcValue XmlRpcValue::makeArray (void) {
    XmlRpcEnv env;
    xmlrpc_value *value = xmlrpc_build_value(env, "()");
    env.throwIfFaultOccurred();
    return XmlRpcValue(value, CONSUME_REFERENCE);    
}

XmlRpcValue XmlRpcValue::makeStruct (void) {
    XmlRpcEnv env;
    xmlrpc_value *value = xmlrpc_struct_new(env);
    env.throwIfFaultOccurred();
    return XmlRpcValue(value, CONSUME_REFERENCE);
}

XmlRpcValue XmlRpcValue::makeBase64 (const unsigned char *const data,
                                     size_t len)
{
    XmlRpcEnv env;
    xmlrpc_value *value = xmlrpc_build_value(env, "6", data, len);
    env.throwIfFaultOccurred();
    return XmlRpcValue(value, CONSUME_REFERENCE);
}

XmlRpcValue::int32 XmlRpcValue::getInt (void) const {
    XmlRpcEnv env;
    XmlRpcValue::int32 result;
    xmlrpc_parse_value(env, mValue, "i", &result);
    env.throwIfFaultOccurred();
    return result;
}

bool XmlRpcValue::getBool (void) const {
    XmlRpcEnv env;
    xmlrpc_bool result;
    xmlrpc_parse_value(env, mValue, "b", &result);
    env.throwIfFaultOccurred();
    return result;
}

double XmlRpcValue::getDouble (void) const {
    XmlRpcEnv env;
    double result;
    xmlrpc_parse_value(env, mValue, "d", &result);
    env.throwIfFaultOccurred();
    return result;
}

string XmlRpcValue::getRawDateTime (void) const {
    XmlRpcEnv env;
    char *result;
    xmlrpc_parse_value(env, mValue, "8", &result);
    env.throwIfFaultOccurred();
    return string(result);
}

string XmlRpcValue::getString (void) const {
    XmlRpcEnv env;
    char *result;
    size_t result_len;
    xmlrpc_parse_value(env, mValue, "s#", &result, &result_len);
    env.throwIfFaultOccurred();
    return string(result, result_len);
    
}

XmlRpcValue XmlRpcValue::getArray (void) const {
    XmlRpcEnv env;
    xmlrpc_value *result;
    xmlrpc_parse_value(env, mValue, "A", &result);
    env.throwIfFaultOccurred();
    return XmlRpcValue(result);
}

XmlRpcValue XmlRpcValue::getStruct (void) const {
    XmlRpcEnv env;
    xmlrpc_value *result;
    xmlrpc_parse_value(env, mValue, "S", &result);
    env.throwIfFaultOccurred();
    return XmlRpcValue(result);
}

void XmlRpcValue::getBase64 (const unsigned char *& out_data,
                             size_t& out_len) const
{
    XmlRpcEnv env;
    xmlrpc_parse_value(env, mValue, "6", &out_data, &out_len);
    env.throwIfFaultOccurred();
}

size_t XmlRpcValue::arraySize (void) const {
    XmlRpcEnv env;
    size_t result = xmlrpc_array_size(env, mValue);
    env.throwIfFaultOccurred();
    return result;
}

void XmlRpcValue::arrayAppendItem (const XmlRpcValue& value) {
    XmlRpcEnv env;
    xmlrpc_array_append_item(env, mValue, value.borrowReference());
    env.throwIfFaultOccurred();
}

XmlRpcValue XmlRpcValue::arrayGetItem (int index) const {
    XmlRpcEnv env;
    xmlrpc_value *result = xmlrpc_array_get_item(env, mValue, index);
    env.throwIfFaultOccurred();
    return XmlRpcValue(result);
}

size_t XmlRpcValue::structSize (void) const {
    XmlRpcEnv env;
    size_t result = xmlrpc_struct_size(env, mValue);
    env.throwIfFaultOccurred();
    return result;
}

bool XmlRpcValue::structHasKey (const string& key) const {
    XmlRpcEnv env;
    const char *keystr = key.data();
    size_t keylen = key.size();
    bool result = xmlrpc_struct_has_key_n(env, mValue,
                                          const_cast<char*>(keystr), keylen);
    env.throwIfFaultOccurred();
    return result;
}

XmlRpcValue XmlRpcValue::structGetValue (const string& key) const {
    XmlRpcEnv env;
    const char *keystr = key.data();
    size_t keylen = key.size();
    xmlrpc_value *result =
        xmlrpc_struct_get_value_n(env, mValue,
                                  const_cast<char*>(keystr), keylen);
    env.throwIfFaultOccurred();
    return XmlRpcValue(result);
}

void XmlRpcValue::structSetValue (const string& key, const XmlRpcValue& value)
{
    XmlRpcEnv env;
    const char *keystr = key.data();
    size_t keylen = key.size();
    xmlrpc_struct_set_value_n(env, mValue, (char*) keystr, keylen,
                              value.borrowReference());
    env.throwIfFaultOccurred();
}

void XmlRpcValue::structGetKeyAndValue (const int index,
                                        string& out_key,
                                        XmlRpcValue& out_value) const
{
    XmlRpcEnv env;

    xmlrpc_value *key, *value;
    xmlrpc_struct_get_key_and_value(env, mValue, index, &key, &value);
    env.throwIfFaultOccurred();

    out_key = XmlRpcValue(key).getString();
    out_value = XmlRpcValue(value);
}

XmlRpcGenSrv& XmlRpcGenSrv::addMethod (const string& name,
                                                        xmlrpc_method method,
                                                        void *data)
{
    XmlRpcEnv env;

        xmlrpc_registry_add_method (env, mRegistry, NULL,
                                name.c_str (),
                                method, data);

    env.throwIfFaultOccurred ();
        return (*this);
}

XmlRpcGenSrv& XmlRpcGenSrv::addMethod (const string& name,
                        xmlrpc_method method,
                        void* data,
                        const string& signature,
                        const string& help)
{
    XmlRpcEnv env;

        xmlrpc_registry_add_method_w_doc (env, mRegistry, NULL,
                                name.c_str (),
                                method, data,
                                signature.c_str (),
                                help.c_str ());

    env.throwIfFaultOccurred ();
        return (*this);
}

xmlrpc_mem_block* XmlRpcGenSrv::alloc (XmlRpcEnv& env, const string& body) const
{
        xmlrpc_mem_block*       result = NULL;
        char*                           contents;

        result          = xmlrpc_mem_block_new (env, body.length ());
        env.throwIfFaultOccurred ();

        contents        = XMLRPC_TYPED_MEM_BLOCK_CONTENTS(char, result);

        memcpy (contents, body.c_str (), body.length ());
        return result;
}

string XmlRpcGenSrv::handle (const string& body) const
{
        XmlRpcEnv env;
        string result;
        xmlrpc_mem_block*       input = NULL, * output = NULL; 
        char* input_data, * output_data;
        size_t input_size, output_size;

        if (body.length () > xmlrpc_limit_get (XMLRPC_XML_SIZE_LIMIT_ID))
                throw XmlRpcFault (XMLRPC_LIMIT_EXCEEDED_ERROR, "XML-RPC request too large");

        input   = alloc (env, body);
        input_data = XMLRPC_TYPED_MEM_BLOCK_CONTENTS(char, input);
        input_size = XMLRPC_TYPED_MEM_BLOCK_SIZE(char, input);

        output  = xmlrpc_registry_process_call (env, mRegistry, NULL,
                        input_data, input_size);

        if (output)
        {
        output_data = XMLRPC_TYPED_MEM_BLOCK_CONTENTS(char, output);
                output_size = XMLRPC_TYPED_MEM_BLOCK_SIZE(char, output);

                result.assign (output_data, output_size);
                xmlrpc_mem_block_free (output);
        }

        xmlrpc_mem_block_free (input);
        if (!result.length ())
                throw XmlRpcFault (env);

        return result;
}
