/*\
 * test-mktime.c
 *
 * Copyright (c) 2020 - Geoff R. McLane
 * Licence: GNU GPL version 2
 *
 * from : https://www.tutorialspoint.com/c_standard_library/c_function_mktime.htm
 *
\*/

#include <stdio.h>
#include <time.h>
// other includes

static const char *module = "test-mktime";

// main() OS entry
int main( int argc, char **argv )
{
    int iret = 0;
    time_t epoch;
    struct tm info;
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
      strftime(buffer, sizeof(buffer), "%c", &info );
      printf("Results time is '%s', i.e. %lld secs since unix epoch, 1/1/70.\n", buffer, epoch);
   }

    return iret;
}


// eof = test-mktime.c
