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

#ifdef _WIN32

#define HAVE_PTHREADS 1
#include "xmlrpc_pthreads.h"

#include <process.h>

#undef PACKAGE
#undef VERSION

#include "xmlrpc.h"

int pthread_create(pthread_t *new_thread_ID,
          const pthread_attr_t * attr,
          pthread_func start_func, void *arg)
{
        HANDLE hThread;
        unsigned dwThreadID;

        XMLRPC_ASSERT (attr == NULL); /* unimplemented. */
    XMLRPC_ASSERT_PTR_OK(new_thread_ID);
    XMLRPC_ASSERT_PTR_OK(start_func);
    XMLRPC_ASSERT_PTR_OK(arg);

        hThread = (HANDLE) _beginthreadex (NULL, 0, 
                start_func, (LPVOID)arg, CREATE_SUSPENDED, &dwThreadID);

        SetThreadPriority (hThread, THREAD_PRIORITY_NORMAL); 
        ResumeThread (hThread);

        *new_thread_ID = hThread;

        return hThread ? 0 : -1;
}

/* Just kill it. */
int pthread_cancel(pthread_t target_thread)
{
        CloseHandle (target_thread);
        return 0;
}

/* Waits for the thread to exit before continuing. */
int pthread_join(pthread_t target_thread, void **status)
{
        DWORD dwResult = WaitForSingleObject(target_thread, INFINITE);
        (*status) = (void *)dwResult;
        return 0;
}

/* Stubbed. Do nothing. */
int pthread_detach(pthread_t target_thread)
{
        return 0;
}

int pthread_mutex_init(pthread_mutex_t *mp,
                                        const pthread_mutexattr_t * attr)
{
        InitializeCriticalSection(mp);
        return 0;
}

int pthread_mutex_lock(pthread_mutex_t *mp)
{
        EnterCriticalSection(mp);
        return 0;
}

int pthread_mutex_unlock(pthread_mutex_t *mp)
{
        LeaveCriticalSection(mp);
        return 0;
}

int pthread_mutex_destroy(pthread_mutex_t *mp)
{
        DeleteCriticalSection(mp);
        return 0;
}

#endif
