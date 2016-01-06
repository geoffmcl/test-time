// timer.hxx
#ifndef _WIN_TIMER_HXX_
#define _WIN_TIMER_HXX_

#ifdef _MSC_VER
#pragma warning(disable: 4996) // unsafe!!!
#pragma warning(disable: 4244) //'initializing' : conversion from 'double' to '__int64', possible loss of data
#include <windows.h>
#else // _MSC_VER
#include <sys/time.h>
#endif // _MSC_VER y/n

#ifdef NDEBUG
#define dbg_printf
#else
#if (defined(DEBUG) || defined(_DEBUG))
#define dbg_printf printf
#else
#define dbg_printf(x,...)
#endif
#endif
static int _s_win_timer_instance = 0;

class win_Timer {
public:
    win_Timer() {
        _s_win_timer_instance++;
        wt_inst = _s_win_timer_instance;
        init();
        dbg_printf("Constructor called %d\n", wt_inst);
    };
    ~win_Timer() { dbg_printf("Destructor  called %d\n", wt_inst); };
    void init() {
#ifdef _MSC_VER
        got_qpt = QueryPerformanceFrequency( &freq ); 
        if(got_qpt) {
            QueryPerformanceCounter( &begin ); // counter value
        } else {
            begin.HighPart = 0;
            begin.LowPart  = GetTickCount(); // ms since system started
            freq.HighPart = 0;
            freq.LowPart  = 1000;
        }
#else // !_MSC_VER
        gettimeofday(&begin,&tz); /* struct sec + usecs */
#endif // _MSC_VER y/n
    }
    double elapsed() {
        double db;
#ifdef _MSC_VER
        if ( got_qpt ) {
            QueryPerformanceCounter( &current ); // counter value
        } else {
            current.HighPart = 0;
            current.LowPart  = GetTickCount();   // ms elapsed
        }
        diff.QuadPart = (current.QuadPart - begin.QuadPart); // get difference
        db  = (double)diff.QuadPart / (double)freq.QuadPart;
#else // !_MSC_VER
        gettimeofday(&current,&tz);
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
    };

    void reset() { init(); };
#ifdef _MSC_VER
    double get_freq() { return (double)freq.QuadPart; };
    double get_begin() { return (double)begin.QuadPart; };
    double get_current() { elapsed(); return (double)current.QuadPart; };
    double get_diff() { elapsed(); return (double)diff.QuadPart; };
    LARGE_INTEGER freq, begin, current, diff;
#else // !_MSC_VER
    double get_freq() { return 1000000.0; }; // unknown, so return 1 mico-second (usec)
    double get_begin() { return (double)begin.tv_sec + ((double)begin.tv_usec / 1000000.0); };
    double get_current() { elapsed(); return (double)current.tv_sec + ((double)current.tv_usec / 1000000.0); };
    double get_diff() { return elapsed(); };
    struct timezone tz;
    struct timeval freq, begin, current, diff;    
#endif // _MSC_VER y/n
    int got_qpt;
    int wt_inst;
};

#endif // #ifndef _WIN_TIMER_HXX_
// eof - win_timer.hxx
