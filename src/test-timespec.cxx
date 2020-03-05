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

static const char *module = "test-timespec";

// main() OS entry
int main( int argc, char **argv )
{
    int iret = 0;
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    char buff[100];
    strftime(buff, sizeof buff, "%D %T", gmtime(&ts.tv_sec));
    printf("Current time: %s.%09ld UTC\n", buff, ts.tv_nsec);

    return iret;
}


// eof = test-timespec.cxx
