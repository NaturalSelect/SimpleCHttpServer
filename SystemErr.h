#pragma once
#ifndef _SYSTEMERR_H
#define _SYSTEMERR_H

#include "SystemDef.h"
#include <stdint.h>

#ifdef IS_WIN
#include "IncWindows.h"

static uint32_t LastError()
{
    return GetLastError();
}
#else
#include <errno.h>

static uint32_t LastError()
{
    return errno;
}
#endif

#endif