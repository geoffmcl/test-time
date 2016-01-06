// MyTimer.hxx - extracted from Atlas - hatlas5m - 
#ifdef _MSC_VER
#include <windows.h>
#else // ! _MSC_VER
#include <sys/time.h>
#endif // _MSC_VER y/n
#include <string>

using namespace std;

#define TM_BUF_SIZE 264
#ifndef _MSC_VER
typedef unsigned int DWORD;
#define BOOL int
#endif

typedef  struct tagGTM { /* gtm */
   BOOL  tm_bt;
#ifdef _MSC_VER   
   LARGE_INTEGER  tm_lif, tm_lib, tm_lie;
#else // ! _MSC_VER
    struct timeval tm_lif, tm_lib, tm_lie;
    struct timezone tz;
#endif // _MSC_VER y/n
   DWORD tm_dwtc;
   char  tm_buf[TM_BUF_SIZE];
}GTM, * PGTM;

class MyTimer
{
public:
    MyTimer() { ctz = 1; reset(); };
    ~MyTimer() { };
    void reset(void);
    double elapsed(void);
    char * elap_stg(void);
    char * time_stg(double seconds);
    int ctz;
    GTM gtm;
};

// global instance
extern MyTimer g_timer;

// eof - MyTimer.hxx
