// test-timer.cxx : Defines the entry point for the console application.
// 20111219 - will not compile - missing strlen() function...
// 20110408 - port to Ubuntu (some tests only...)
// 20130331 - Added using select (ADD_SEL_DELAY) test
// 20091011 - To test a win_Timer class

// FEATURES ENABLED/DISABLED
#ifdef _MSC_VER
#ifdef WIN64
#undef ADD_CB_DELAY
#else
#define ADD_CB_DELAY
#endif
#undef ADD_SEL_DELAY // UGH! all tests FAILED until created a socket as well!?
#undef ADD_FILE_TEST
#define ADD_THREAD_TEST
#define ADD_TIMEB_TEST
#else
// have NOT ported some tests
#undef ADD_CB_DELAY
#define ADD_SEL_DELAY // UGH! all tests FAILED until created a socket as well!?
#undef ADD_FILE_TEST
#undef ADD_THREAD_TEST
#undef ADD_TIMEB_TEST
#endif // _MSC_VER y/n
// ===========================

#ifdef _MSC_VER
#pragma warning( disable : 4312 ) // 'type cast' : conversion from 'int' to 'HANDLE' of greater size
#else
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#endif // _MSC_VER

#include "test-timer.hxx"
#include <stdio.h>
#include <sys/timeb.h>
#include <time.h>

#include "MyTimer.hxx"
#include "sprtf.hxx"

#ifdef ADD_SEL_DELAY
/* ========================================= */
#include <math.h> // for fabs()
#ifdef _MSC_VER
#pragma comment( lib, "ws2_32" )
#define SERROR(a) (a == SOCKET_ERROR)
#define SCLOSE(s) closesocket(s)
#define SLASTERR WSAGetLastError()
#else // !_MSC_VER
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#define SERROR(a) ( a < 0 )
#define SCLOSE(s) close(s)
#define SLASTERR errno
#ifndef SOCKET
#define SOCKET int
#endif
#endif // _MSC_VER y/n
/* ========================================= */
#endif // ADD_SEL_DELAY

#ifndef _MSC_VER
#include <string.h>     // for strlen()
#include <stdint.h>     // for uint64_t
#include <stdlib.h>     // for getenv()
#include <termios.h>
#include <sched.h> // for int sched_yield(void);   
#include <pthread.h> // for _beginthread, ...
//#include <limits.h>     // for std::numeric_limits<type>::max()
#include <limits>     // for std::numeric_limits<type>::max()
#define Sleep(a) if (a > 1000) sleep(a / 1000); else sched_yield();
#define _getcwd getcwd
#endif // !_MSC_VER

#ifndef EndBuf
#define EndBuf(a) ( a + strlen(a) )
#endif

#ifdef ADD_CB_DELAY
extern void shortDelay(unsigned int val);
#endif

#if (defined(_MSC_VER) && defined(_DEBUG))
#define new DEBUG_CLIENTBLOCK
#endif

static int output_to_file = 0;
static int add_iter_test = 0; // // this test take 20-70 seconds to complete

#define KBD_SLEEP   1

#define TEST_STRINGIZE(X) TEST_DO_STRINGIZE(X)
#define TEST_DO_STRINGIZE(X) #X
#ifdef __GNUC__
#define TEST_COMPILER_STR "GNU C++ version " TEST_STRINGIZE(__GNUC__) "." TEST_STRINGIZE(__GNUC_MINOR__)
#else
 #if defined(__KCC)
 #define TEST_COMPILER_STR "Kai C++ version " TEST_STRINGIZE(__KCC_VERSION)
 #else
  #ifdef _MSC_VER
  #define TEST_COMPILER_STR "Microsoft Visual C++ version " TEST_STRINGIZE(_MSC_VER)
  #else
   #if defined(sgi)
   #define TEST_COMPILER_STR "SGI MipsPro version " TEST_STRINGIZE(_COMPILER_VERSION)
   #else
    #if defined(__sun)
    #define TEST_COMPILER_STR "Sun compiler version " TEST_STRINGIZE(__SUNPRO_CC)
    #else
     #if defined(__ICC) || defined(__ECC)
     #define TEST_COMPILER_STR "Intel C++ version " TEST_STRINGIZE(__ICC)
     #else
      #define TEST_COMPILER_STR "Compile type/version NOT known"
     #endif
    #endif
   #endif
  #endif
 #endif
#endif

// using namespace std;

static int thread_exit = 0;
static int got_thread = 0;
static int keyboard_checks = 0;

win_Timer start_up;

char * get_datetime_str( void )
{
    static char _s_ds_buf[64];
    char * cp = _s_ds_buf;
    time_t  date;
    struct tm *tmr;
    date = time(0);
    tmr = localtime(&date);
    sprintf (cp, "%02d.%02d.%04d %02d:%02d:%02d ",
        tmr->tm_mday,
        tmr->tm_mon+1,
        tmr->tm_year+1900,
        tmr->tm_hour,
        tmr->tm_min,
        tmr->tm_sec);
    return cp;
}

#ifdef ADD_FILE_TEST
static char * ptmpfile = "temptest.txt"; 
//long _get_osfhandle( int fd );
void file_test(void)
{
    static char _s_tmp_buf[1024];
    char * cp = _s_tmp_buf;
    size_t i, i2, i3;
    FILE * fp = fopen(ptmpfile, "wb");
    if(fp) {
        intptr_t fh = _get_osfhandle( fp->_file );
        i = sprintf(cp, "Item 1: Got file pointer %p... or HANDLE %p\n", fp, fh );
        sprtf(cp);
        i2 = fwrite(cp, 1, i, fp ); // write using the pointer
        fflush(fp); // ensure FLUSHED to this file, or things will be out of order
        // AND WITHOUT THIS, A CloseHandle() WILL NOT EVEN WRITE THIS TO THE FILE!!!
        if ( i == i2 )
            sprtf((char *)"First write using pointer ok (%d vs %d)\n", i, i2 );
        else
            sprtf((char *)"First write using pointer appears to have failed! (%d vs %d)\n", i, i2 );
        if ( fh && ( fh != (intptr_t)-1 ) ) {
            i = sprintf(cp, "Item 2: Written %d (%d) to file using pointer... and this using the HANDLE\n", i, i2);
            if (WriteFile((HANDLE)fh, cp, (DWORD)i, (LPDWORD)&i3, NULL)) {
                sprtf((char *)"And success writing %d bytes (%d) using the HANDLE ;=))\n", i3, i );
                CloseHandle((HANDLE)fh);
                fp = NULL;
                sprtf(cp);
            } else {
                sprtf((char *)"Failed to wirte to handle returned!\n");
            }
        } else {
            sprtf((char *)"ERROR: _get_osfhandle(%d) returned %d! FAILED!!\n", fp->_file, fh );
        }
        if(fp)
            fclose(fp);
    } else {
        sprtf((char *)"Failed to open %s file...\n", ptmpfile );
    }
}
#endif // #ifdef ADD_FILE_TEST

#ifdef _MSC_VER
#include <conio.h>
DWORD StdinAvail() {
  static HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
  DWORD bytes_left = 0;
  if (PeekNamedPipe(handle, NULL, 0, NULL, &bytes_left, NULL))
      return bytes_left;
  return 0;
}

int check_console_events(void)
{
  static HANDLE handle = GetStdHandle(STD_INPUT_HANDLE);
    INPUT_RECORD ir;
    DWORD dw = 0;
    if (PeekConsoleInput(handle,    // _In_  HANDLE        hConsoleInput,
        &ir,    // _Out_ PINPUT_RECORD lpBuffer,
        1,      // _In_  DWORD         nLength,
        &dw ))  // _Out_ LPDWORD       lpNumberOfEventsRead
    {
        if (dw)
            return 1;
    }
    return 0;
}


int test_for_input2(void)
{
    //int chr = StdinAvail();
    int chr = check_console_events();
    if (chr)
        chr = _getch();
    return chr;
}
int test_for_input(void)
{
    int chr = _kbhit();
    if (chr)
        chr = _getch();
    return chr;
}

#else /* !_MSC_VER */
int kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

  ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);

  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }

  return 0;
}
int test_for_input(void)
{
    int chr = kbhit();
    if (chr)
        chr = getchar();
    return chr;
}
#endif /* _MSC_VER y/n */

int check_key_available( void )
{
   int chr = test_for_input();
   keyboard_checks++;
   return chr;
}


int watch_keyboard(void)
{
    int chr = 0;
    while ( (chr = check_key_available()) == 0 ) {
#ifdef _MSC_VER
        Sleep(KBD_SLEEP);
#else // !_MSC_VER
        sched_yield();   // from #include <sched.h>
#endif // _MSC_VER y/n
        if (thread_exit) {
            chr = -1;
            break;
        }
    }
#ifdef _MSC_VER
    if ( chr == 0 ) {
        int off;
        char c[4];
        chr = _getch();
        while (check_key_available()) _getch();
        off = 0;
        if (chr & 0x80) {
            c[off++] = '@';
            chr &= ~0x80;
        }
        if (chr < ' ' ) {
            chr += '@';
            c[off++] = '^';
            c[off++] = chr;
        } else {
            c[off++] = chr;
        }
        c[off] = 0;
        sprtf((char *)"Got keyboard input [%s]... thread exiting...\n", c );
    }
#endif // _MSC_VER
    return chr;
}

/* =====================================================
// NOT USED
DWORD WINAPI ThreadProc( LPVOID lpParameter )
{
    watch_keyboard();
    thread_exit |= 2;
    return 0xdead;
}

// from : http://msdn.microsoft.com/en-us/library/ms682453(VS.85).aspx
// could use the _beginthread and _endthread functions 
void exit_daemion_thread(void)
{
    // start a thread
    DWORD   id;
    HANDLE thread = CreateThread(
        NULL,       // LPSECURITY_ATTRIBUTES lpThreadAttributes,
        0,          // SIZE_T dwStackSize,
        ThreadProc,  // LPTHREAD_START_ROUTINE lpStartAddress,
        NULL,       // LPVOID lpParameter,
        0,          // DWORD dwCreationFlags,
        &id );      // LPDWORD lpThreadId
}
  ================================================================== */
#ifdef ADD_THREAD_TEST
void thread_proc(void * vp)
{
    sprtf((char *)"Thread running... any key to exit thread...\n");
    watch_keyboard();
    thread_exit |= 2;
}

// from : http://msdn.microsoft.com/en-us/library/kdzttdcb.aspx
void start_keyboard_thread(void)
{
    sprtf((char *)"Using Windows native '_beginthread' to create a thread...\n");
    uintptr_t thread = _beginthread( // NATIVE CODE
        thread_proc,    // void( __cdecl *start_address )( void * ),
        0,              // unsigned stack_size,
        NULL );         // void *arglist 
    if (thread == -1) {
        sprtf((char *)"Failed to create thread...\n");
    } else {
        got_thread = 1;
        sprtf((char *)"Create thread id %p...\n", thread);
    }
}
#endif // #ifdef ADD_THREAD_TEST

void show_timer_info(win_Timer & tm)
{
    double elap;
#ifdef _MSC_VER    
    double freq = tm.get_freq();
    double bgn = tm.get_begin();
    char * pfrq = get_kdouble_with_2_sig_chars(freq);
    char * pcnt = get_kdouble_with_2_sig_chars(bgn);
    strcat(pfrq,"Hz");
    strcat(pcnt,"ticks");
    sprtf((char *)"Timer frequency %g Hz (%s or each %s ",
        freq, pfrq, g_timer.time_stg(1000.0 / freq) );
    g_timer.reset();
    elap = g_timer.elapsed();
    sprtf((char *)"- %s)\n", g_timer.time_stg(elap));
    sprtf((char *)"and begin counter is %g (%s).\n",
        bgn, pcnt );
#else // !_MSC_VER
    double min = 1000000.0;
    int i;
    for (i = 0; i < 500; i++) {
        g_timer.reset();
        elap = g_timer.elapsed();
        if (elap > 0.0) {
            if (elap < min)
                min = elap;
        }
    }
    sprtf((char *)"Shortest estimated elapse = %s\n", g_timer.time_stg(min));
#endif // _MSC_VER
}

double timer_test(void)
{
    win_Timer tm;
    show_timer_info(tm);
    Sleep( 50 );
    double elap = tm.elapsed();
    if (elap < 0.001 ) {
        //      Been running for
        sprtf((char *)"Test ran for     %g milliseconds...", (elap * 1000.0));
    } else {
        sprtf((char *)"Test ran for     %g seconds...", elap );
    }
    sprtf((char *)" (%s)\n", g_timer.time_stg(elap) );
    return elap;
}


#if (defined(_MSC_VER) && defined(_DEBUG))
//int newFlag = _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_CRT_DF | _CRTDBG_DELAY_FREE_MEM_DF |
//    _CRTDBG_LEAK_CHECK_DF;
int newFlag = _CRTDBG_ALLOC_MEM_DF | _CRTDBG_DELAY_FREE_MEM_DF |  _CRTDBG_LEAK_CHECK_DF;
int oldFlag = 0;

HANDLE get_win_file_handle( char * fname )
{
    HANDLE hand = CreateFile(
        fname,                          // LPCTSTR lpFileName,
        GENERIC_READ|GENERIC_WRITE,     // DWORD dwDesiredAccess,
        0,                              // DWORD dwShareMode,
        NULL,                           // LPSECURITY_ATTRIBUTES lpSecurityAttributes,
        CREATE_ALWAYS,                  // DWORD dwCreationDisposition,
        FILE_ATTRIBUTE_ARCHIVE,         // DWORD dwFlagsAndAttributes,
        NULL );                         // HANDLE hTemplateFile
    return hand;
}

void set_crt_out_file( _HFILE hf )
{
   // Send all reports to STDOUT
   _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE );
   _CrtSetReportFile( _CRT_WARN, hf );
   _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_FILE );
   _CrtSetReportFile( _CRT_ERROR, hf );
   //_CrtSetReportFile( _CRT_ERROR, _CRTDBG_FILE_STDERR );
   _CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_FILE );
   _CrtSetReportFile( _CRT_ASSERT, hf );
}


void set_crt_dbg_mem(void)
{
    int tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    tmpDbgFlag |= newFlag;
    oldFlag = _CrtSetDbgFlag( tmpDbgFlag );
   // Send all reports to STDOUT
    set_crt_out_file( _CRTDBG_FILE_STDOUT );
}

void show_crt_dbg_mem(void)
{
    HANDLE hand = INVALID_HANDLE_VALUE;
    char * msg1 = "\nDump of memory statistics...\n";
    char * msg2 = "Dump of memory objects...\n";
    _CrtMemState s1;
    _CrtMemCheckpoint( &s1 );
    if ( output_to_file ) {
        hand = (HANDLE)get_log_handle();
        // hand = get_win_file_handle( "tempmemdbg.txt" );
    }
    if ( hand && (hand != INVALID_HANDLE_VALUE) ) {
        //DWORD dww;
        flush_log_file();
        set_crt_out_file( hand );
        sprtf(msg1);
        //flush_log_file();
        //WriteFile(hand, msg1, (DWORD)strlen(msg1), &dww, NULL);
        _CrtMemDumpStatistics( &s1 );
        sprtf(msg2);
        //WriteFile(hand, msg2, (DWORD)strlen(msg2), &dww, NULL);
        _CrtMemDumpAllObjectsSince( NULL );
    } else {
        sprtf(msg1);
        _CrtMemDumpStatistics( &s1 );
        sprtf(msg2);
        _CrtMemDumpAllObjectsSince( NULL );
    }
}
#endif // DEBUG = #if (defined(_MSC_VER) && defined(_DEBUG))

void show_inter_info( unsigned int i, double elap, int k )
{
    // get cyles per second - Hz
    double hetz = (double)i / elap;
    char * cp1 = GetNxtBuf();
    char * cp2 = GetNxtBuf();
    __int64 val = i;
    __int64 hv = hetz;
    int lev;
    char * knum = get_k_num_with_2_sig_chars( val, 1, 0, &lev );
    if ( lev == 1 ) // tailed with ' bytes'
        knum[ strlen(knum) - 6 ] = 0;
    else
        knum[ strlen(knum) - 5 ] = 0;
    sprintf(cp1,"%u", i);
    nice_num(cp2,cp1);
    char * hnum = get_k_num_with_2_sig_chars( hv, 0, 0, &lev );
    if ( lev == 1 ) // tailed with ' bytes'
        hnum[ strlen(hnum) - 6 ] = 0;
    else
        hnum[ strlen(hnum) - 1 ] = 0;
    strcat(hnum,"Hz");

    char * cp3 = GetNxtBuf();
    if (got_thread) {
        char * cp4 = GetNxtBuf();
        sprintf(cp3,"%u", k );
        nice_num(cp4,cp3);
        sprintf(cp3,"\n(with %s Sleep(0))", cp4);
    } else
        cp3 = (char *)"";

    sprtf((char *)"Done  %s (%s) interations, in %g seconds,\n %g Hz (%s)... %s\n", 
        cp2, knum, elap, hetz, hnum, cp3 );
}

void do_interations(void)
{
    win_Timer tm;
    unsigned int i = 0;
    unsigned int k = 0;
    sprtf((char *)"\nIteration test takes about 20-60 seconds+ to complete... " );
    if (got_thread)
        sprtf((char *)"Any key to exit before...");
    sprtf((char *)"\n");
    for (;;) {
        i++;
        if(i == 0)
            break;
        else if (( i % 1000 ) == 0 ) {
            k++;
            if ( got_thread ) {
                Sleep(0);
            }
        } else if ( got_thread && thread_exit ) {
            break;
        }
    }

    sprtf((char *)"Did %u iterations...\n", (i - 1));

    double elap = tm.elapsed();
    i -= 1;
    show_inter_info( i, elap, k );
}

char * basename( char * name )
{
   size_t len = strlen(name);
   size_t i, j;
   int c;
   j = 0;
   for( i = 0; i < len; i++ ) {
      c = name[i];
      if(( c == '/' )||
         ( c == '\\')||
         ( c == ':' ))
      {
         j = i + 1;
      }
   }
   return &name[j];
}

std::string get_compiler_version_string(int cr)
{
    std::string s = "Compiled on ";
    s += __DATE__;
    s += " at ";
    s += __TIME__;
    char * env = getenv("COMPUTERNAME");
    if(env) {
        if (cr & 1)
            s += "\n";
        else
            s += " ";
        s += "in ";
        s += env;
        env = getenv("USERNAME");
        if(env) {
            s += " by ";
            s += env;
        }
    }
    if (cr & 4) {
        if (cr & 2)
            s += "\n";
        else
            s += " ";
        s += "with ";
        s += TEST_COMPILER_STR;
    }
    return s;
}

// from : http://msdn.microsoft.com/en-us/library/sf98bd4y(VS.80).aspx
char * get_cwd_stg(void)
{
    char * cp = GetNxtBuf();
    _getcwd(cp, 256);
    return cp;
}

extern _TCHAR * get_kdouble_with_2_sig_chars_ok_maybe( double val, 
                                                      int type_in = 0,
                                                      int divK = 0,
                                                      int * plev = NULL);

#ifndef _UI64_MAX
#define _UI64_MAX ULLONG_MAX
#endif

void test_max(void)
{
    uint64_t i64m = _UI64_MAX;
    double dval = (double)i64m;
    int type = ( TYP_LONG | TYP_ADDB );
    char * cp1 = GetNxtBuf();
    char * cp2 = GetNxtBuf();
    int lev2, lev3;

    int lev = get_value_level( (double)DBL_MAX, 1000.0 );
    sprtf((char *)"LEVEL for DBL_MAX is %d (%#X)\n", lev, lev );

    char * cmax = get_kdouble_with_2_sig_chars(dval,type,0,&lev2); // = 3
    char * cmax2 = get_kdouble_with_2_sig_chars_ok_maybe(dval,type,0,&lev3); // = 3
    lev = get_value_level( dval, 1000.0 );
    sprintf(cp1, PF64U, i64m);
    nice_num(cp2,cp1);
    sprtf((char *)"Max UINT = %s (%s) or \n %s (%d)\n %s (%d) %d\n", cp1, cp2,
        cmax, lev2, cmax2, lev, lev3);
    dval = DBL_MAX;
    char * cmax3 = get_kdouble_with_2_sig_chars(dval,TYP_ADDB);
    sprtf((char *)"DBL_MAX = %s\n", cmax3 );

    dval *= 100.0;
    cmax3 = get_kdouble_with_2_sig_chars(dval,TYP_ADDB);
    sprtf((char *)"DBL_MAX * 100 = %s\n", cmax3 );

    exit(1);
}

#ifdef _MSC_VER
//#include <stdio.h>
//#include <sys/timeb.h>
//#include <time.h>

int test_timeb_main( void )
{
    int iret = 0;
    struct _timeb timebuffer;
    char timeline[26];
    errno_t err;
    time_t time1;
    unsigned short millitm1;
    short timezone1;
    short dstflag1;

    _ftime( &timebuffer ); // C4996
    // Note: _ftime is deprecated; consider using _ftime_s instead

    time1 = timebuffer.time;
    millitm1 = timebuffer.millitm;
    timezone1 = timebuffer.timezone;
    dstflag1 = timebuffer.dstflag;

    sprtf((char *)"Seconds since midnight, January 1, 1970 (UTC): " PF64U "\n or %s\n",
            (__int64)time1,
            g_timer.time_stg((double)time1) );

    sprtf((char *)"Milliseconds: %d (%s)\n", millitm1, g_timer.time_stg((double)millitm1 / 1000.0));

    sprtf((char *)"Minutes between UTC and local time: %d (%s)\n",
        timezone1,
        g_timer.time_stg((double)(timezone1 * 60)));

    sprtf((char *)"Daylight savings time flag (1 means Daylight time is in "
           "effect): %d\n", dstflag1); 
   
    err = ctime_s( timeline, 26, & ( timebuffer.time ) );
    if (err) {
        sprtf((char *)"Invalid argument to ctime_s. ");
        iret = 1;
    }
    sprtf((char *)"The time is %.19s.%hu %s", timeline, timebuffer.millitm, &timeline[20] );

    return iret;
}

#endif // _MSC_VER

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

#ifdef _MSC_VER
struct timezone {
int     tz_minuteswest; /* minutes W of Greenwich */
int     tz_dsttime;     /* type of dst correction */
};

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
  FILETIME ft;
  unsigned __int64 tmpres = 0;
  static int tzflag;
 
  if (NULL != tv)
  {
    GetSystemTimeAsFileTime(&ft);
 
    tmpres |= ft.dwHighDateTime;
    tmpres <<= 32;
    tmpres |= ft.dwLowDateTime;
 
    /*converting file time to unix epoch*/
    tmpres /= 10;  /*convert into microseconds*/
    tmpres -= DELTA_EPOCH_IN_MICROSECS; 
    tv->tv_sec = (long)(tmpres / 1000000UL);
    tv->tv_usec = (long)(tmpres % 1000000UL);
  }
 
  if (NULL != tz)
  {
    if (!tzflag)
    {
      _tzset();
      tzflag++;
    }
    tz->tz_minuteswest = _timezone / 60;
    tz->tz_dsttime = _daylight;
  }
 
  return 0;
}

void test_ftime(struct timeval * tv)
{
  struct _timeb timebuf;
  _ftime (&timebuf);
  tv->tv_sec = timebuf.time;
  tv->tv_usec = timebuf.millitm * 1000;
}

void test_gettimeofday_function(void)
{
    struct timeval tv;
    struct timeval tv2;
    struct timezone tz;
    gettimeofday( &tv, &tz);
    sprtf((char *)"gettimeofday results...\n");
    sprtf((char *)"timeval: tv_secs=%d tv_usecs=%d,\ntimezone: tz_minutesweat=%d, tz_dsttime=%d\n",
        tv.tv_sec, tv.tv_usec, tz.tz_minuteswest, tz.tz_dsttime );
    test_ftime( &tv2 );
    sprtf((char *)"timeval: tv_secs=%d tv_usecs=%d by _ftime()\n",
        tv2.tv_sec, tv2.tv_usec );

}
#endif // _MSC_VER

#ifdef ADD_SEL_DELAY
void sock_init(void)
{
#if defined(_MSC_VER)
	/* Start up the windows networking */
	WORD version_wanted = MAKEWORD(1,1);
	WSADATA wsaData;
	if ( WSAStartup(version_wanted, &wsaData) != 0 ) {
        int wsa_errno = WSAGetLastError();
		//ulSetError(UL_WARNING,"Couldn't initialize Winsock 1.1");
        fprintf(stderr, "WSAStartup FAILED! %d",wsa_errno );
		exit(1);
	}
#endif
}

void sock_end(void)
{
#ifdef _MSC_VER
    WSACleanup();
#endif // _MSC_VER
}

#ifdef _MSC_VER
int selDelay( double secs )
{
    int status;
    int usecs = (int)(secs * 1000000.0);
    struct timeval tv;
    tv.tv_sec = usecs / 1000000;
    tv.tv_usec = usecs % 1000000;
    if ((tv.tv_sec == 0) && (tv.tv_usec == 0))
        return -3;
    fd_set set;
    FD_ZERO(&set);
    SOCKET s = socket(AF_INET,SOCK_STREAM,0);
    if ( SERROR(s) ) {
        sprtf((char *)"ERROR: Got SOCKET_ERROR %d from socket()!\n",
            SLASTERR );
        return -2;
    }
    FD_SET(s,&set);
    status = select(0, &set, 0, 0, &tv);
    if ( SERROR(status) )
        sprtf((char *)"ERROR: Got SOCKET_ERROR %d from select()! (s=%d,us=%d)\n",
            SLASTERR, tv.tv_sec, tv.tv_usec);
    SCLOSE(s);
    return status;
}

#else // !_MSC_VER
/* this can be MUCH simpler in unix ;=)) */
int selDelay( double secs )
{
    int status;
    int usecs = (int)(secs * 1000000.0);
    struct timeval tv;
    tv.tv_sec = usecs / 1000000;
    tv.tv_usec = usecs % 1000000;
    if ((tv.tv_sec == 0) && (tv.tv_usec == 0))
        return -3;
    status = select(0, 0, 0, 0, &tv); /* works with ALL null, except timeval */
    if ( SERROR(status) )
        sprtf((char *)"ERROR: Got SOCKET_ERROR %d from select()! (s=%d,us=%d)\n",
            SLASTERR, tv.tv_sec, tv.tv_usec);
    return status;
}
#endif // _MSC_VER y/n

#define MX_SD_TESTS 6
void test_select_delay(void)
{
    static char _s_tsl_buf[256];
    char * cp = _s_tsl_buf;
    sprtf((char *)"\nTest using select() as the delay... %d tests... 1st is 10 secs..\n", MX_SD_TESTS);
    MyTimer t;
    double secs = 10.0;
    int i;
    double elap, des_total, elap_total, t1, elap2;
    sock_init();
    size_t max1 = 20;
    size_t max2 = max1 + 20;
    size_t max3 = max2 + 16;
    des_total = 0.0;
    elap_total = 0.0;

    for (i = 0; i < MX_SD_TESTS; i++)
    {
        t.reset();
        t1 = get_seconds();
        selDelay(secs);
        elap2 = get_seconds() - t1;
        elap = t.elapsed();
        des_total += secs;
        elap_total += elap;
        //sprtf((char *)"Delay for %s", t.time_stg(secs));
        //sprtf((char *)", took %s", t.time_stg(elap));
        //sprtf((char *)", error %1.3f%% ", ((100.0 * (elap - secs)) / secs) );
        //sprtf((char *)"(%s)\n", get_seconds_stg(elap2));
        sprintf(cp,"Delay for %s,", t.time_stg(secs));
        while (strlen(cp) < max1) strcat(cp," ");
        sprintf(EndBuf(cp),"took %s,", t.time_stg(elap));
        while (strlen(cp) < max2) strcat(cp," ");
        sprintf(EndBuf(cp),"error %1.3f%% ", ((100.0 * (elap - secs)) / secs) );
        while (strlen(cp) < max3) strcat(cp," ");
        sprtf((char *)"%s (%s)\n", cp, get_seconds_stg(elap2));
        secs /= 10.0;
    }
    sock_end();
    sprtf((char *)"Desired total of %s", t.time_stg(des_total) );
    sprtf((char *)", took %s", t.time_stg(elap_total));
    sprtf((char *)", error %1.3f%%\n\n", ((100.0 * (elap_total - des_total)) / des_total) );

}
#endif // #ifdef ADD_SEL_DELAY

void clock_wait(int in_ms)
{
    clock_t ms = in_ms;
    clock_t last_clock, curr_clock, msecs;

    msecs = 0;
    last_clock = clock();
    while (msecs < ms) {
        curr_clock = clock();
        // get elapsed milliseconds
        msecs = ((curr_clock - last_clock) * 1000) / CLOCKS_PER_SEC;
        if (msecs < ms) {
            Sleep(0);
        }
    }
}

static int max_tests = 6;
void test_clock_delay(void)
{
    static char _s_tcd_buf[256];
    char *cp = _s_tcd_buf;
    MyTimer t;
    clock_t ms = 1000;
    int i;
    double elap, dsecs, t1, elap2;
    sprtf((char *)"\nTesting clock() delay... in milliseconds...\n" );
    size_t max1 = 20;
    size_t max2 = max1 + 22;
    for (i = 0; i < max_tests; i++) {
        t.reset();
        t1 = get_seconds();
        clock_wait(ms);
        elap2 = get_seconds() - t1;
        elap = t.elapsed();
        dsecs = ((double)ms / 1000.0); 
        //sprtf((char *)"Delay for %s, took %s (%s)\n",
        //    t.time_stg(dsecs),
        //    t.time_stg(elap),
        //    get_seconds_stg(elap2) );
        sprintf(cp,"Delay for %s, ", t.time_stg(dsecs));
        while (strlen(cp) < max1) strcat(cp," ");
        sprintf(EndBuf(cp),"took %s ", t.time_stg(elap));
        while (strlen(cp) < max2) strcat(cp," ");
        sprintf(EndBuf(cp),"(%s)", get_seconds_stg(elap2) );
        sprtf("%s\n",cp);
        ms = ms / 10;
    }
}

void test_usleep_delay(void)
{
    static char _s_tcd_buf[256];
    char *cp = _s_tcd_buf;
    MyTimer t;
    clock_t micro = 1000000;
    int i;
    double elap, dsecs, t1, elap2;
    sprtf((char *)"\nTesting usleep() delay... in microseconds...\n" );
    size_t max1 = 20;
    size_t max2 = max1 + 22;
    for (i = 0; i < max_tests; i++) {
        t.reset();
        t1 = get_seconds();
        usleep(micro);
        elap2 = get_seconds() - t1;
        elap = t.elapsed();
        dsecs = ((double)micro / 1000000.0); 
        //sprtf((char *)"Delay for %s, took %s (%s)\n",
        //    t.time_stg(dsecs),
        //    t.time_stg(elap),
        //    get_seconds_stg(elap2) );
        sprintf(cp,"Delay for %s, ", t.time_stg(dsecs));
        while (strlen(cp) < max1) strcat(cp," ");
        sprintf(EndBuf(cp),"took %s ", t.time_stg(elap));
        while (strlen(cp) < max2) strcat(cp," ");
        sprintf(EndBuf(cp),"(%s)", get_seconds_stg(elap2) );
        sprtf("%s\n",cp);
        micro = micro / 10;
    }
}

#ifdef _MSC_VER
void test_usleep_wait(void)
{
    static char _s_tcd_buf[256];
    char *cp = _s_tcd_buf;
    MyTimer t;
    clock_t micro = 1000000;
    int i;
    double elap, dsecs, t1, elap2;
    sprtf((char *)"\nTesting usleep_wait() delay... in microseconds...\n" );
    size_t max1 = 20;
    size_t max2 = max1 + 22;
    for (i = 0; i < max_tests; i++) {
        t.reset();
        t1 = get_seconds();
        if (usleep_wait(micro))
            break;
        elap2 = get_seconds() - t1;
        elap = t.elapsed();
        dsecs = ((double)micro / 1000000.0); 
        //sprtf((char *)"Delay for %s, took %s (%s)\n",
        //    t.time_stg(dsecs),
        //    t.time_stg(elap),
        //    get_seconds_stg(elap2) );
        sprintf(cp,"Delay for %s, ", t.time_stg(dsecs));
        while (strlen(cp) < max1) strcat(cp," ");
        sprintf(EndBuf(cp),"took %s ", t.time_stg(elap));
        while (strlen(cp) < max2) strcat(cp," ");
        sprintf(EndBuf(cp),"(%s)", get_seconds_stg(elap2) );
        sprtf("%s\n",cp);
        micro = micro / 10;
    }
}
#endif // #ifdef _MSC_VER

#ifdef ADD_CB_DELAY     // TODO: Need to redo this in 646-bits
#define MX_DTEST 7
void do_short_delay_test()
{
    static char _s_dsdt_buf[256];
    char *cp = _s_dsdt_buf;
    //unsigned int usecs = 1000000000;
    unsigned int usecs = UINT_MAX; // = 4294967295 (0xffffffff)
    double elapsed[MX_DTEST];
    double elapsed2[MX_DTEST];
    //unsigned int val_usecs[MX_DTEST];
    ULARGE_INTEGER val_usecs[MX_DTEST];
    double us_div = 1000000000.0;
    double el_mul = 1000.0;
    win_Timer * t3 = new win_Timer;
    win_Timer * t2 = new win_Timer;
    sprtf((char *)"\nBegin shortDelay() testing ... using timeSetEvent()\n");
    int ii;
    ULARGE_INTEGER sum;
    double t1, elap2;
    for (ii = 0; ii < MX_DTEST; ii++) {
        t3->reset();
        t1 = get_seconds();
        shortDelay(usecs);
        elap2 = get_seconds() - t1;
        elapsed[ii] = t3->elapsed();
        val_usecs[ii].QuadPart = usecs;
        elapsed2[ii] = elap2;
        usecs /= 10;
    }
    double elap = t2->elapsed();
    sprtf((char *)"Summary of each of %d events...\n", MX_DTEST);
    sum.QuadPart = 0;
    size_t max1 = 24;
    size_t max2 = max1 + 18;
    size_t max3 = max2 + 16;
    for (ii = 0; ii < MX_DTEST; ii++) {
        sum.QuadPart += val_usecs[ii].QuadPart;
        //sprtf((char *)"shortDelay %g, elapsed %g (%s)\n",
        //    (double)val_usecs[ii].QuadPart / us_div,
        //    elapsed[ii] * el_mul,
        //    g_timer.time_stg(elapsed[ii]) );
        sprintf(cp,"shortDelay %g,", (double)val_usecs[ii].QuadPart / us_div);
        while (strlen(cp) < max1) strcat(cp," ");
        sprintf(EndBuf(cp),"elapsed %g ", elapsed[ii] * el_mul);
        while (strlen(cp) < max2) strcat(cp," ");
        sprintf(EndBuf(cp),"(%s)", g_timer.time_stg(elapsed[ii]) );
        while (strlen(cp) < max3) strcat(cp," ");
        sprintf(EndBuf(cp),"%s", get_seconds_stg( elapsed2[ii] ));
        sprtf("%s\n",cp);
    }
    sprtf((char *)"End shortDelay() total %g us ... elapsed %g us (%s)\n\n",
        (double)sum.QuadPart / us_div,
        elap * el_mul,
        g_timer.time_stg(elap) );
    delete t3;
    delete t2;
}
#endif // #ifdef ADD_CB_DELAY     // TODO: Need to redo this in 646-bits


int do_delay( struct timeval *ptv, int ms_delay, int *cyc, double *slept )
{
    int delay_secs = ptv->tv_sec;
    int delay_ms   = ptv->tv_usec / 1000;
    int cycle_count = 0;
    double slept_for = 0.0;
    int res = test_for_input();
    while (!res && (delay_secs || delay_ms)) {
        if (!delay_secs && (delay_ms < ms_delay))
            ms_delay = delay_ms;    // reduce this last sleep
        Sleep(ms_delay);
        cycle_count++;
        slept_for += (double)ms_delay / 1000.0;
        if (delay_ms >= ms_delay)
            delay_ms -= ms_delay;
        else {
            if (delay_secs) {
                delay_ms += 1000;
                delay_secs--;
            }
            if (delay_ms >= ms_delay)
                delay_ms -= ms_delay;
            else
                delay_ms = 0;
        }
        res = test_for_input();
    }
    *cyc = cycle_count;
    *slept = slept_for;
    return res;
}

char *get_tv_secs_stg( struct timeval * ptv )
{
    // tv.tv_sec = 4;
    // tv.tv_usec = 999 * 1000;
    double secs = (double)ptv->tv_sec;
    secs += (double)ptv->tv_usec / 1000000.0;
    char *cp = get_seconds_stg(secs);
    return cp;
}


int main(int argc, char * argv[])
{
    int i;
    char * arg;
    double begin = get_seconds();
    for ( i = 1; i < argc; i++ ) {
        arg = argv[i];
        if (*arg == 'i') {
            add_iter_test = 1;
            sprtf((char *)"Will add iteration test...\n");
        }
        else {
            sprtf((char *)"%s: Just a bunch of time and timer tests...\n", basename(argv[0]));
            sprtf((char *)"Only argument is 'i', to add iteration test...\n");
            sprtf((char *)"Unknown argument '%s'! Aborting...\n",arg);
            return 1;
        }
    }
    //test_max();
    char * cp;
#ifdef _DEBUG
    set_crt_dbg_mem();
#endif // #ifdef _DEBUG
    double elap, tst;
    win_Timer * pat = new win_Timer;
    sprtf((char *)"%s: %s:\n %s\n",
        get_datetime_str(),
        basename(argv[0]),
        get_compiler_version_string(5).c_str() );
    sprtf((char *)"Running in [%s] folder...\n", get_cwd_stg() );

#ifdef _MSC_VER
    // Now the DEFAULT - removed from <stdio.h>
    // https://connect.microsoft.com/VisualStudio/feedback/details/1368280
    // _set_output_format( _TWO_DIGIT_EXPONENT );
#endif // _MSC_VER
#ifdef ADD_THREAD_TEST
    start_keyboard_thread();    // start a thread to watch the keyboard
    Sleep(10);  // sleep 10 ms, to start thread...
#endif

    sprtf((char *)"Max double = %g or more precisely\n%f\n",
        std::numeric_limits<double>::max(),
        std::numeric_limits<double>::max());

#ifdef ADD_CB_DELAY
    do_short_delay_test();
#else   // !#ifdef ADD_CB_DELAY
#ifdef _MSC_VER
    sprtf((char *)"\nShort delay test disabled! TODO: Need 64-bit short delay service...\n\n");
#endif // #ifdef _MSC_VER
#endif  // #ifdef ADD_CB_DELAY y/n

#ifdef ADD_TIMEB_TEST
    test_timeb_main();
    test_gettimeofday_function();
#endif

#ifdef ADD_SEL_DELAY
    test_select_delay();
#endif

#ifdef ADD_FILE_TEST
    file_test();
#endif // #ifdef ADD_FILE_TEST
    tst = timer_test();

    if (add_iter_test)
        do_interations();

    elap = pat->elapsed();
    if (elap < 0.001 ) {
        sprtf((char *)"Been running for %g milliseconds... ", (elap * 1000.0));
    } else {
        sprtf((char *)"Been running for %g seconds... ", elap );
    }
    sprtf((char *)"(%s)\n", g_timer.time_stg(elap) );

    // some memory items
    cp = (char *)malloc(256);
    strcpy(cp, "This is a 256 byte block");
    free(cp);
    delete pat;

    if (got_thread && (thread_exit == 0)) {
        sprtf((char *)"Wait thread exit...\n" );
        pat = new win_Timer;
        thread_exit |= 1;
        while ( !(thread_exit & 2) )
            Sleep(1);
        elap = pat->elapsed();
        sprtf((char *)"Thread exit, in %g secs... Did %s keyboard checks...\n",
            elap, get_nice_number_stg(keyboard_checks) );
        delete pat;
    } else if (got_thread) {
        sprtf((char *)"Got thread exit... Did %s keyboard checks...\n",
            get_nice_number_stg(keyboard_checks) );
    }

    test_clock_delay();
    test_usleep_delay();
#ifdef _MSC_VER
    test_usleep_wait();
#endif // #ifdef _MSC_VER

    // EXIT CODE
    sprtf((char *)"\n%s: ", get_datetime_str());
    elap = start_up.elapsed();
    if (elap < 0.001 ) {
        //      Been running for
        sprtf((char *)"Since start up   %g milliseconds... ", (elap * 1000.0));
    } else {
        sprtf((char *)"Since start up   %g seconds... ", elap );
    }
    sprtf((char *)"(%s)\n", g_timer.time_stg(elap) );

    sprtf((char *)"Ran for %s\n", get_seconds_stg( get_seconds() - begin ));

#ifdef _DEBUG
    show_crt_dbg_mem();
#endif // #ifdef _DEBUG
#ifdef _MSC_VER
    int cycle_count = 0;
    double slept_for = 0;
    int ms_delay = 55;
    struct timeval tv;
    //tv.tv_sec = 4;
    //tv.tv_usec = 999 * 1000;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    printf("\07");  // bell character
    sprtf((char *)"\nWill wait %s for any key to exit...\n", get_tv_secs_stg(&tv));
    ////////////////////////////////////////////////////////////////////////////
    double bgn = get_seconds();
    int res = do_delay( &tv, ms_delay, &cycle_count, &slept_for );
    double end = get_seconds();
    /////////////////////////////////////////////////////////////////////////////
    sprtf("Got %s. I waited %s for your input, slept for %s,\nlooped %d times, using %d ms sleep.\n", 
        (res ? "INPUT" : "TIMEOUT"),
        get_seconds_stg(end - bgn), get_seconds_stg(slept_for), cycle_count, ms_delay );

#endif // _MSC_VER
    // exit(0);
	return 0;
}

// eof - test-timer.cxx
