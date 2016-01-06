// From Atlas - hatlas5m - file : win_utils.cxx

#ifdef _MSC_VER
#pragma warning( disable : 4996) // like 'sprintf': This function or variable may be unsafe. Consider using sprintf_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.
#endif // _MSC_VER

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <sys/types.h>
#include <sys/stat.h>   // for stat
#include <string>
#include <sstream>
#include <iostream>
#ifdef _MSC_VER
#include <windows.h>    // for Sleep(ms)
#else
#include <string.h>     // for strlen()
#include <stdio.h>      // for sprintf()
#endif // _MSC_VER y/n
#include <math.h>
#include "MyTimer.hxx"

using namespace std;

MyTimer g_timer;

// === a simple WIN32 timer ===
void MyTimer::reset()
{
    PGTM ptm = &gtm;
#ifdef _MSC_VER
    ptm->tm_bt = QueryPerformanceFrequency( &ptm->tm_lif );
    if(ptm->tm_bt)
        QueryPerformanceCounter( &ptm->tm_lib ); // counter value
    else
        ptm->tm_dwtc = GetTickCount(); // ms since system started
#else // !_MSC_VER
    ptm->tm_bt = 1;
    gettimeofday( &ptm->tm_lib, &ptm->tz );
#endif // _MSC_VER y/n
}

double MyTimer::elapsed(void)
{
    PGTM ptm = &gtm;
    double db = 0.0;
#ifdef _MSC_VER
    if( ptm->tm_bt ) {
        LARGE_INTEGER lid;
        QueryPerformanceCounter( &ptm->tm_lie ); // counter value
        lid.QuadPart = ( ptm->tm_lie.QuadPart - ptm->tm_lib.QuadPart ); // get difference
        // performance-counter frequency, in counts per second
        db  = (double)lid.QuadPart / (double)ptm->tm_lif.QuadPart;
    } else {
        DWORD dwd = (GetTickCount() - ptm->tm_dwtc);   // ms elapsed
        db = ((double)dwd / 1000.0);
    }
#else // !_MSC_VER
    gettimeofday( &ptm->tm_lie, &ptm->tz );
    #define diff ptm->tm_lif
    #define begin ptm->tm_lib
    #define current ptm->tm_lie
    diff.tv_sec = current.tv_sec - begin.tv_sec;
    if (begin.tv_usec > current.tv_usec) {
        diff.tv_usec = (current.tv_usec + 1000000) - begin.tv_usec;
        diff.tv_sec--;
    } else {
        diff.tv_usec = current.tv_usec - begin.tv_usec;
    }
    db = (double)diff.tv_sec;
    db += (double)diff.tv_usec / 1000000.0; 
#endif // _MSC_VER y/n
    return db;
}

// A microsecond is an SI unit of time equal to one millionth (10?6) of a second. 
// Its symbol is �s.
// A microsecond is equal to 1000 nanoseconds or 1/1000 millisecond. 
// Because the next SI unit is 1000 times larger, measurements of 
// 10?5 and 10?4 seconds are typically expressed as tens or 
// hundreds of microseconds.
char * MyTimer::time_stg(double seconds)
{
    double elap = seconds;
    int negative = 0;
    char * units;
    int can_clear = 1;
    if (elap < 0) {
        negative = 1;
        elap = -elap;
    }
    char * cp = &gtm.tm_buf[0];
    if ( !(elap > 0.0) ) {
        cp = (char *)"0.0 s";
        return cp;
    }
    if (elap < 0.000000000000000000001) {
        // yocto - 10^-24
        sprintf(cp,"%g", elap * 1000000000000000000000000.0);
        units = (char *)" yocto-s";
    } else
    if (elap < 0.000000000000000001) {
        // zepto - 10^-21
        sprintf(cp,"%.6f", elap * 1000000000000000000000.0);
        units = (char *)" zepto-s";
    } else
    if (elap < 0.000000000000001) {
        // atto - 10^-18
        sprintf(cp,"%.6f", elap * 1000000000000000000.0);
        units = (char *)" atto-s";
    } else
    if (elap < 0.000000000001) {
        // femto - 10^-15
        sprintf(cp,"%.6f", elap * 1000000000000000.0);
        units = (char *)" femto-s";
    } else
    if (elap < 0.000000001) {
        // pico - 10^-12
        sprintf(cp,"%.6f", elap * 1000000000000.0);
        units = (char *)" pico-s";
    } else
    if (elap < 0.000001) {
        // nanosecond - one thousand millionth (10?9) of a second
        //                          123456789
        sprintf(cp,"%2.6f", elap * 1000000000.0);
        units = (char *)" ns";
    } else if (elap < 0.001) {
        // microsecond - one millionth (10?6) of a second
        //sprintf(cp,"%2.6f �s", elap * 1000000.0);
        // unfortunately, not all code pages display '�' correctly, so
        sprintf(cp,"%2.6f", elap * 1000000.0);
        units = (char *)" us";
    } else if (elap < 1.0) {
        // millisecond
        sprintf(cp,"%2.2f", elap * 1000.0);
        units = (char *)" ms";
    } else if (elap < 60.0) {
        sprintf(cp,"%2.2f", elap);
        units = (char *)" secs";
    } else {
        int secs = (int)(elap + 0.5);
        int mins = (int)(secs / 60);
        can_clear = 0; // integers only
        secs = (secs % 60);
        if (mins >= 60) {
            int hrs = mins / 60;
            mins = mins % 60;
            if (hrs >= 24) {
                int days = hrs / 24;
                hrs = hrs % 24;
                sprintf(cp, "%d days %2d:%02d:%02d", days, hrs, mins, secs);
                units = (char *)" hh:mm:ss";
            } else {
                sprintf(cp, "%2d:%02d:%02d", hrs, mins, secs);
                units = (char *)" hh:mm:ss";
            }
        } else {
            sprintf(cp, "%2d:%02d mm:ss", mins, secs);
            units = (char *)" mm:ss";
        }
    }
    if (negative) {
        char tmp_tb[TM_BUF_SIZE];
        strcpy(tmp_tb,cp);
        cp[0] = '-';
        strcpy(&cp[1],tmp_tb);
    }
    if (ctz && can_clear && strchr(cp,'.')) {
        size_t len = strlen(cp);
        size_t i, ch;
        for (i = len - 1; i > 0; i--) {
            ch = cp[i];
            if (ch == '.') {
                cp[i] = 0;
                break;
            } else if ( ch != '0' )
                break;
            cp[i] = 0;
        }
    }
    strcat(cp,units); // add units
    return cp;
}

char * MyTimer::elap_stg(void)
{
    return time_stg(elapsed());
}

/* end class MyTimer implementation */
