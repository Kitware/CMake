#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "xmlrpc.h"
#include "xmlrpc_int.h"
#include "xmlrpc_config.h"



const char * 
xmlrpc_makePrintable(const char * const input) {
/*----------------------------------------------------------------------------
   Convert an arbitrary string of bytes (null-terminated, though) to
   printable ASCII.  E.g. convert newlines to "\n".

   Return the result in newly malloc'ed storage.  Return NULL if we can't
   get the storage.
-----------------------------------------------------------------------------*/
    char * output;
    const size_t inputLength = strlen(input);

    output = malloc(inputLength*4+1);

    if (output != NULL) {
        unsigned int inputCursor, outputCursor;

        for (inputCursor = 0, outputCursor = 0; 
             inputCursor < inputLength; 
             ++inputCursor) {

            if (isprint((int)(input[inputCursor])))
                output[outputCursor++] = input[inputCursor]; 
            else if (input[inputCursor] == '\n') {
                output[outputCursor++] = '\\';
                output[outputCursor++] = 'n';
            } else if (input[inputCursor] == '\t') {
                output[outputCursor++] = '\\';
                output[outputCursor++] = 't';
            } else if (input[inputCursor] == '\a') {
                output[outputCursor++] = '\\';
                output[outputCursor++] = 'a';
            } else if (input[inputCursor] == '\r') {
                output[outputCursor++] = '\\';
                output[outputCursor++] = 'r';
            } else {
                snprintf(&output[outputCursor], 4, "\\x%02x", 
                         input[inputCursor]);
            }
        }
        output[outputCursor+1] = '\0';
    }
    return output;
}



const char *
xmlrpc_makePrintableChar(char const input) {

    const char * retval;

    if (input == '\0')
        retval = strdup("\\0");
    else {
        char buffer[2];
        
        buffer[0] = input;
        buffer[1] = '\0';
        
        retval = xmlrpc_makePrintable(buffer);
    }
    return retval;
}
