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

#include "xmlrpc_config.h"

#undef PACKAGE
#undef VERSION

#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#ifdef WIN32
#ifdef _DEBUG
#       include <crtdbg.h>
#       define new DEBUG_NEW
#       define malloc(size) _malloc_dbg( size, _NORMAL_BLOCK, __FILE__, __LINE__)
#       undef THIS_FILE
        static char THIS_FILE[] = __FILE__;
#endif
#endif /*WIN32*/

#include "xmlrpc.h"
#include "xmlrpc_client.h"

#if defined (WIN32)
#include <process.h>
#endif

/* For debugging the xmlrpc_transport threading. */
/* #define tdbg_printf printf */
#define tdbg_printf (void *)

/* Lacking from the abyss/thread.c implimentaion. */
void wait_for_asynch_thread(pthread_t *thread)
{
#if WIN32
        unsigned long milliseconds = INFINITE;
        switch (WaitForSingleObject (
        *thread /* handle to object to wait for */,
        milliseconds /* time-out interval in milliseconds*/) )
        {
                /* One may want to handle these cases  */
        case WAIT_OBJECT_0:
        case WAIT_TIMEOUT:
                break;
        }
#else
        void * result;
        int success;
        success = pthread_join (*thread, &result);
#endif
}

/* MRB-WARNING: Only call when you have successfully
**     acquired the Lock/Unlock mutex! */
void unregister_asynch_thread (running_thread_list *list, pthread_t *thread)
{
        running_thread_info * pCur = NULL;
        XMLRPC_ASSERT_PTR_OK(thread);
        XMLRPC_ASSERT_PTR_OK(list);

        tdbg_printf("unregister_asynch_thread: &pthread_id = %08X *(%08X)\n", thread, *thread);
        /* Removal */
        /* Lock (); */
        for (pCur = list->AsyncThreadHead; pCur != NULL; pCur = (running_thread_info *)pCur->Next)
        {
                if (pCur->_thread == *thread)
                {
                        if (pCur == list->AsyncThreadHead)
                                list->AsyncThreadHead = pCur->Next;
                        if (pCur == list->AsyncThreadTail)
                                list->AsyncThreadTail = pCur->Last;
                        if (pCur->Last)
                                ((running_thread_info *)(pCur->Last))->Next = pCur->Next;
                        if (pCur->Next)
                                ((running_thread_info *)(pCur->Next))->Last = pCur->Last;
                        /* Free malloc'd running_thread_info */
                        free (pCur);
                        return;
                }
        }

        /* This is a serious progmatic error, since the thread
        ** should be in that list! */
        XMLRPC_ASSERT_PTR_OK(0x0000);

        /* Unlock (); */
}

/* MRB-WARNING: Only call when you have successfully
**     acquired the Lock/Unlock mutex! */
void register_asynch_thread (running_thread_list *list, pthread_t *thread)
{
        running_thread_info* info = (running_thread_info *) malloc(sizeof(running_thread_info));

        XMLRPC_ASSERT_PTR_OK(thread);
        XMLRPC_ASSERT_PTR_OK(list);
        
        tdbg_printf("register_asynch_thread: &pthread_id = %08X *(%08X)\n", thread, *thread);

        info->_thread = *thread;

        /* Insertion */
        /* Lock (); */
        if (list->AsyncThreadHead == NULL)
        {
                list->AsyncThreadHead = list->AsyncThreadTail = info;
                list->AsyncThreadTail->Next = list->AsyncThreadHead->Next = NULL;
                list->AsyncThreadTail->Last = list->AsyncThreadHead->Last = NULL;
        }
        else
        {
                info->Last = list->AsyncThreadTail;
                list->AsyncThreadTail->Next = info;
                list->AsyncThreadTail = list->AsyncThreadTail->Next;
                list->AsyncThreadTail->Next = NULL;
        }
        /* Unlock (); */
}
