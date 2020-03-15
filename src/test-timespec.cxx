/*\
 * test-timespec.cxx
 *
 * Copyright (c) 2020 - Geoff R. McLane
 * Licence: GNU GPL version 2
 *
\*/
/* from: https://en.cppreference.com/w/c/chrono/timespec_get */
#include <stdio.h>
#include <time.h>
// other includes
#if defined(__MINGW32__)
#include <Windows.h>
#ifndef TIME_UTC
#define TIME_UTC 1
#endif
//struct timespec { long tv_sec; long tv_nsec; };    //header part
int clock_gettime(int, struct timespec *spec)      //C-file part
{
    __int64 wintime;
    GetSystemTimeAsFileTime((FILETIME*)&wintime);
    wintime -= 116444736000000000; //i64;  //1jan1601 to 1jan1970
    spec->tv_sec = wintime / 10000000; //i64;           //seconds
    spec->tv_nsec = wintime % 10000000 * 100; //i64 * 100;      //nano-seconds
    return 0;
}
// #include "timespec_get.c"

// 20200313 from : https://mailman.videolan.org/pipermail/vlc-commits/2015-December/033263.html

/*****************************************************************************
* timespec_get.c: C11 timespec_get() replacement
*****************************************************************************
* Copyright © 2015 Rémi Denis-Courmont
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation; either version 2.1 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this program; if not, write to the Free Software Foundation,
* Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
*****************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <time.h>
#include <unistd.h> /* _POSIX_TIMERS */
#ifndef _POSIX_TIMERS
#define _POSIX_TIMERS (-1)
#endif

int timespec_get(struct timespec *ts, int base)
{
    switch (base)
    {
    case TIME_UTC:
#if (_POSIX_TIMERS >= 0)
        if (clock_gettime(CLOCK_REALTIME, ts) == 0)
            break;
#endif
#if (_POSIX_TIMERS <= 0)
        {
            struct timeval tv;

            if (gettimeofday(&tv, NULL) == 0)
            {
                ts->tv_sec = tv.tv_sec;
                ts->tv_nsec = tv.tv_usec * 1000;
                break;
            }
        }
#endif
        /* fall through */
    default:
        return 0;
    }
    return base;
}

// eof

#endif

static const char *module = "test-timespec";

// main() OS entry
int main( int argc, char **argv )
{
    int iret = 0;
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    char buff[100];
// #if defined(__MINGW32__)
    strftime(buff, sizeof buff, "%d/%m/%Y %H:%M:%S", gmtime(&ts.tv_sec));
// #else
//    strftime(buff, sizeof buff, "%D %T", gmtime(&ts.tv_sec));
// #endif
    printf("%s: Current time: %s.%09ld UTC\n", module, buff, ts.tv_nsec);

    return iret;
}


// eof = test-timespec.cxx
