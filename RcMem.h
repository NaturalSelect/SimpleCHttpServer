#pragma once
#ifndef _RCMEM_H
#define _RCMEM_H
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef NDEBUG
#include <stdio.h>
#endif

#include "SystemDef.h"

#ifdef IS_WIN
#include "IncWindows.h"
#include <process.h>
#else
#include <pthread.h>
#endif

typedef struct _MemBlock
{
    volatile uint32_t rc_;
    void *mem_;
} MemBlock;

#ifdef IS_WIN
#define INC_RC(rc) InterlockedIncrement(&rc)
#define DEC_RC(rc) InterlockedDecrement(&rc)
#else
#define INC_RC(rc) __sync_fetch_and_add(&rc,1)
#define DEC_RC(rc) __sync_fetch_and_sub(&rc,1)
#endif

//allocate memory with RC control block
static void *RcAlloc(size_t size)
{
    //alloc memory
    MemBlock *mb = (MemBlock*)malloc(sizeof(MemBlock) + size);
    //if mb == NULL,we fail
    if (!mb)
    {
        return NULL;
    }
    //init RC
    mb->rc_ = 1;
    //compute memory pointer and return
    mb->mem_ = (void*)((uintptr_t)mb + sizeof(MemBlock));
    return mb->mem_;
}

//calloc version
static void *RcCalloc(size_t count,size_t size)
{
    return RcAlloc(count*size);
}

//dealloc memory
static void RcFree(void *rcMem)
{
    //if rcMem == NULL,we return directly
    if (!rcMem)
    {
        return;
    }
    //get RC control block
    MemBlock *mb = (MemBlock*)((uintptr_t)rcMem - sizeof(MemBlock));
    //as same as mb->rc_ -= 1
    uint32_t rc = DEC_RC(mb->rc_);
    //if rc == 0,we release memory and control block
    if (!rc)
    {
        free(mb);
    }
}

//pass a pointer
static void *RcPass(void *rcMem)
{
    //if rcMem == NULL,we return NULL
    if (!rcMem)
    {
        return NULL;
    }
    //get RC control block
    MemBlock *mb = (MemBlock*)((uintptr_t)rcMem - sizeof(MemBlock));
    //as same as mb->rc_ += 1
    INC_RC(mb->rc_);
    return mb->mem_;
}

//print debug information
static void RcDbgPrint(void *rcMem)
{
#ifndef NDEBUG
    if (!rcMem)
    {
        return;
    }
    MemBlock *mb = (MemBlock*)((uintptr_t)rcMem - sizeof(MemBlock));
    printf("rc address %p\nrc count %u\nrc mem %p\n",mb,mb->rc_,mb->mem_);
#endif
}

#define RCNEW(type) (type*)RcAlloc(sizeof(type))
#define RCPASS(type,mem) (type*)RcPass(mem)
#define RCDELETE(mem) RcFree(mem)
#define RCCALLOC(type,count) (type*)RcCalloc(sizeof(type),count)

#ifdef IS_WIN
typedef uintptr_t thread_t;
#else
typedef pthread_t thread_t;
#endif

static int ThreadCreate(thread_t *handle,void(*fun)(void *),void *arg)
{
#ifdef IS_WIN
     *handle = _beginthread(fun,0,arg);
     if (*handle == (thread_t)INVALID_HANDLE_VALUE)
     {
         return -1;
     }
     return 0;
#else
    return pthread_create(handle,NULL,fun,arg);
#endif
}

static int RcThreadCreate(thread_t *handle,void(*fun)(void *),void *arg)
{
    return ThreadCreate(handle,fun,RcPass(arg));
}

static void JoinThread(thread_t *handle)
{
#ifdef IS_WIN
    WaitForSingleObject((HANDLE)*handle,INFINITE);
    CloseHandle((HANDLE)(*handle));
#else
    pthread_join(*handle,NULL);
#endif
}

static uint32_t GetTid()
{
#ifdef IS_WIN
    return GetCurrentThreadId();
#else
    return pthread_self();
#endif
}

static void DetachThread(thread_t *handle)
{
#ifdef IS_WIN
    CloseHandle((HANDLE)*handle);
#else
    pthread_detach(*handle);
#endif
}

#endif