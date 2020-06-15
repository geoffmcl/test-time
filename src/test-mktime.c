/*\
 * test-mktime.c
 *
 * Copyright (c) 2020 - Geoff R. McLane
 * Licence: GNU GPL version 2
 *
 * from : https://www.tutorialspoint.com/c_standard_library/c_function_mktime.htm
 *
\*/
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <time.h>
#ifdef _WIN32
#include <Windows.h>
#endif

static const char *module = "test-mktime";

#ifdef _WIN32
// from : https://stackoverflow.com/questions/36268277/convert-100-nanoseconds-to-millisecond-confusion
DWORD get_milli_time(VOID)
{
    FILETIME ft;
    __int64 nano;
    ft.dwLowDateTime = 0;
    ft.dwHighDateTime = 0;
    GetSystemTimeAsFileTime(&ft);
    nano = ((__int64)(ft.dwHighDateTime) << 32LL)
        + (__int64)(ft.dwLowDateTime);
    // return DWORD(nano / 10e5);
    // return DWORD(nano / 10e3);
    return (DWORD)(nano * 1.0e-4);
}
// from : https://www.gamedev.net/forums/topic/565693-converting-filetime-to-time_t-on-windows/
// A FILETIME is the number of 100-nanosecond intervals since January 1, 1601.
// A time_t is the number of 1 - second intervals since January 1, 1970.

time_t  filetime_to_timet(FILETIME* pft)
{
    ULARGE_INTEGER ull;
    ull.LowPart = pft->dwLowDateTime;
    ull.HighPart = pft->dwHighDateTime;
    return ((ull.QuadPart / 10000000ULL) - 11644473600ULL); 
}
__int64 filetime_to_int64(FILETIME* pft)
{
    ULARGE_INTEGER ull;
    ull.LowPart = pft->dwLowDateTime;
    ull.HighPart = pft->dwHighDateTime;
    return (ull.QuadPart);
}
int filetime_to_buf(PSTR lpb, FILETIME* pft)
{
    int ret = sprintf(lpb, "%20lld", filetime_to_int64(pft));
    return ret;
}
static char _sBuff[264];
char* get_filetime_stg(FILETIME* pft)
{
    char* cp = _sBuff;
    filetime_to_buf(cp, pft);
    return cp;
}
#endif // _WIN32

// main() OS entry
int main( int argc, char **argv )
{
    int iret = 0;
    struct stat buf;
    time_t epoch;
    struct tm info;
    size_t len;
    char buffer[80];

    printf("Setting time to 'Wed Jul  4 00:00:01 2001'\n");
    info.tm_year = 2001 - 1900;
    info.tm_mon = 7 - 1;
    info.tm_mday = 4;
    info.tm_hour = 0;
    info.tm_min = 0;
    info.tm_sec = 1;
    info.tm_isdst = -1;

    // get Unix epoch seconds since 00:00:00 UTC on 1 January 1970.
    epoch = mktime(&info);
   if( epoch == -1 ) {
      printf("Error: unable to make time using mktime\n");
      iret = 1;
   } else {
      len = strftime(buffer, sizeof(buffer), "%c", &info );
      if (len)
          printf("Results time is '%s', i.e. %10lld secs since unix epoch, 1/1/70.\n", buffer, epoch);
      else
          printf("strftime failed, on %10lld secs since unix epoch, 1/1/70.\n",  epoch);
   }

   if (stat(argv[0], &buf) == 0) {
       struct tm* info2;
       info2 = localtime(&buf.st_mtime);
       if (info2) {
           len = strftime(buffer, sizeof(buffer), "%c", info2);
           if (len)
               printf("File    time is '%s', i.e. %10lld secs since epoch.\n", buffer, buf.st_mtime);
           else
               printf("strftime failed, on %10lld secs since unix epoch, 1/1/70.\n", buf.st_mtime);
       }
       else {
           printf("localtime failed, on %10lld secs since unix epoch, 1/1/70.\n", buf.st_mtime);
      }
   }
   else {
       printf("Error: unable to stat file '%s'\n", argv[0]);
       iret = 1;
   }
#ifdef _WIN32
   {
       WIN32_FIND_DATA fd;
       HANDLE hFind = FindFirstFile(argv[0], &fd);
       if (hFind && (hFind != INVALID_HANDLE_VALUE)) {
           time_t val = filetime_to_timet(&fd.ftLastWriteTime);
           struct tm* info3 = localtime(&val);
           if (info3) {
               char* cp = get_filetime_stg(&fd.ftLastWriteTime);
               len = strftime(buffer, sizeof(buffer), "%c", info3);
               if (len)
                   printf("FileWin time is '%s', i.e. %10lld epoch secs, %s 100-nano, since 1/1/1601 .\n", buffer, val, cp);
               else
                   printf("strftime failed, on %10lld epoch secs, %s 100-nano, since 1/1/1601.\n", val, cp);
           }
           FindClose(hFind);
       }
   }
#endif // _WIN32

    return iret;
}


// eof = test-mktime.c
