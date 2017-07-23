// test-utils.cxx

#include "test-timer.hxx"

#ifndef _MSC_VER
#include <string.h>     // for strlen()
#include <unistd.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <math.h>
#define _T (char *)
#define _finite finite
#define _isnan isnan
#endif // !_MSC_VER

#define MXLINEB 260
#define MXLINES 16

static int giLnBuf = 0;
static char gszLnBuf[MXLINEB * MXLINES];
///////////////////////////////////////////////////////////////////////////////
// FUNCTION   : GetNxtBuf
// Return type: LPTSTR 
// Argument   : VOID
// Description: Return a pointer to the "next" general buffer
///////////////////////////////////////////////////////////////////////////////
char *   GetNxtBuf( VOID )
{
   // (MXLINEB * MXLINES)
   int   i = giLnBuf;      //     W.ws_iLnBuf
   i++;
   if( i >= MXLINES )
      i = 0;
   giLnBuf = i;
   return( &gszLnBuf[ (MXLINEB * i) ] );    // W.ws_szLnBuf
}

void nice_num( char * dst, char * src ) // get nice number, with commas
{
   size_t i;
   size_t len = strlen(src);
   size_t rem = len % 3;
   size_t cnt = 0;
   for( i = 0; i < len; i++ )
   {
      if( rem ) {
         *dst++ = src[i];
         rem--;
         if( ( rem == 0 ) && ( (i + 1) < len ) )
            *dst++ = ',';
      } else {
         *dst++ = src[i];
         cnt++;
         if( ( cnt == 3 ) && ( (i + 1) < len ) ) {
            *dst++ = ',';
            cnt = 0;
         }
      }
   }
   *dst = 0;
}

char * get_nice_number_stg( int num )
{
    char * cp1 = GetNxtBuf();
    char * cp2 = GetNxtBuf();
    sprintf(cp1,"%u",num);
    nice_num(cp2,cp1);
    return cp2;
}

#define SPRINTF sprintf
#define get_next_buffer GetNxtBuf
//char * get_k_num_with_2_sig_chars( __int64 i64, int ascii, int type )
char * get_k_num_with_2_sig_chars( __int64 i64, int type, int divK, int * plev )
{
   char * pb = get_next_buffer();
   char * form = (char *)PF64U " bytes";
   uint64_t byts = i64;
   double res;
   unsigned int div = divK ? 1024 : 1000;
   double ddiv = (double)div;
   double val = (double)byts;
   int lev = 0;
   if( byts < div ) { // either 1024 or 1000
      SPRINTF(pb, form, byts);
      lev = 1;
   } else if( byts < (div*div) ) {
      res = val / ddiv;
      form = (type ? _T("%0.2f KiloBytes") : _T("%0.2f KB"));
      SPRINTF(pb, form, res);
      lev = 2;
   } else if( byts < (div*div*div) ) {
      res = (val / (ddiv*ddiv));
      form = (type ? _T("%0.2f MegaBytes") : _T("%0.2f MB"));
      SPRINTF(pb, form, res);
      lev = 3;
   } else { // if( byts <  (1024*1024*1024*1024)){
      double db = (ddiv*ddiv*ddiv);   // x3
      double db2 = db * ddiv;   // x4
      if( val < db2 )
      {
         res = val / db;
         form = (type ? _T("%0.2f GigaBytes") : _T("%0.2f GB"));
         SPRINTF(pb, form, res);
         lev = 4;
      }
      else
      {
         db *= ddiv;  // x4
         db2 *= ddiv; // x5
         if( val < db2 )
         {
            res = val / db;
            form = (type ? _T("%0.2f TeraBytes") : _T("%0.2f TB"));
            SPRINTF(pb, form, res);
            lev = 5;
         }
         else
         {
            db *= ddiv;  // x5
            db2 *= ddiv; // x6
            if( val < db2 )
            {
               res = val / db;
               form = (type ? _T("%0.2f PetaBytes") : _T("%0.2f PB"));
               SPRINTF(pb, form, res);
               lev = 6;
            }
            else
            {
               db *= ddiv;  // x6
               db2 *= ddiv; // x7
               if( val < db2 )
               {
                  res = val / db;
                  form = (type ? _T("%0.2f ExaBytes") : _T("%0.2f EB"));
                  SPRINTF(pb, form, res);
                  lev = 7;
               }
               else
               {
                  db *= ddiv;  // x7
                  res = val / db;
                  form = (type ? _T("%0.2f ZettaBytes") : _T("%0.2f ZB"));
                  SPRINTF(pb, form, res);
                  lev = 8;
               }
            }
         }
      }
   }
   if ( plev )
       *plev = lev;
   //if( ascii > 0 )
   //   Convert_2_ASCII(pb);
   return pb;
}

// _isnan() from : http://msdn.microsoft.com/en-us/library/aa298428(VS.60).aspx
// _finite() from : http://msdn.microsoft.com/en-us/library/aa246875(VS.60).aspx
// isinf, isinfinite could be built/implied from _finite()
// Due to the 64-bit LIMITS of DBL_MAX, a level > 102 (0x66) is TOO LARGE
int get_value_level( double val, double dd )
{
    long double ddiv = dd;
    int level = 1;
    while ( val > ddiv ) {
        ddiv *= dd;
        if (!_finite(ddiv))
           break;
        level++;
    }
    return level;
}

char * get_level_string( int level, int type_in )
{
    int type = (type_in & TYP_LONG) ? 1 : 0;
    int add  = (type_in & TYP_ADDB) ? 1 : 0;
    char * pb = GetNxtBuf();
    char * form = _T("");

    *pb = 0;
    if ( level <= 9 ) {
        switch(level) {
            case 1:
                form = _T("");
                break;
            case 2:
                form = (type ? _T("Kilo") : _T("K"));
                break;
            case 3:
                form = (type ? _T("Mega") : _T("M"));
                break;
            case 4:
                form = (type ? _T("Giga") : _T("G"));
                break;
            case 5:
                form = (type ? _T("Tera") : _T("T"));
                break;
            case 6:
                form = (type ? _T("Peta") : _T("P"));
                break;
            case 7:
                form = (type ? _T("Exa") : _T("E"));
                break;
            case 8:
                form = (type ? _T("Zetta") : _T("Z"));
                break;
            case 9:
                form = (type ? _T("Yotta") : _T("Y")); // 1000^8 10^24 
                break;
        }
        strcat(pb,form);
    } else {
        if (type)
            sprintf(pb, "1000^%d", level );
        else
            sprintf(pb, "10^%d", (level * 3) );
    }
    if (add) {
       if (type)
           strcat(pb,"Bytes");
       else
           strcat(pb,"B");
    }
    return pb;
}

char * get_kdouble_with_2_sig_chars( double val, int type_in, int divK, int * plev)
{
   char * pb = get_next_buffer();
   const char *form = PF64U;
   //char * form = _T("%I64u ");
   uint64_t byts = val;
   //int type = (type_in & TYP_LONG) ? 1 : 0;
   //int add  = (type_in & TYP_ADDB) ? 1 : 0;
   double res;
   unsigned int div = divK ? 1024 : 1000;
   double ddiv = (double)div;
   int lev = 0;
   int ovr = 0;
   int i;
    if ( _isnan(val) ) {
        SPRINTF(pb, "IS_NAN!");
        return pb;
    }
    if ( !_finite(val) ) {
        SPRINTF(pb, "IS_INF!");
        return pb;
    }
   if (val > (double)_UI64_MAX) {
       byts = _UI64_MAX;
   }
   if (( byts < div ) && (val < ddiv)) { // either 1024 or 1000
      SPRINTF(pb, form, byts);
      lev = 1;
   } else {
       lev = get_value_level( val, ddiv );
       form = get_level_string( lev, type_in );
       double db = ddiv;   // x1
       i = ((lev > 1) ? lev - 2 : 0);
       //i = ((lev > 0) ? lev - 1 : 0);
       //i = lev;
       while(i--) {
           db *= ddiv;
       }
       res = val / db;
       SPRINTF(pb, "%0.2f %s", res, form);
   }
   if(ovr)
       strcat(pb,"+++");

   if ( plev )
       *plev = lev;

   return pb;
}

char * get_kdouble_with_2_sig_chars_ok_maybe( double val, int type_in, int divK, int * plev)
{
    char * pb = get_next_buffer();
    char * form = _T("%I64u ");
    int type = (type_in & 1) ? 1 : 0;
    int add  = (type_in & 2) ? 1 : 0;
    double res;
    int div = divK ? 1024 : 1000;
    double ddiv = (double)div;
    int lev = 0;
    int ovr = 0;
    uint64_t bytes = val;

    if ( _isnan(val) ) {
        SPRINTF(pb, "IS_NAN!");
        return pb;
    }
    if ( !_finite(val) ) {
        SPRINTF(pb, "IS_INF!");
        return pb;
    }

    if (val > (double)_UI64_MAX) {
       bytes = _UI64_MAX;
    }
    if( val < ddiv ) { // either 1024 or 1000
        SPRINTF(pb, form, bytes);
        lev = 1;
    } else if( bytes < (div*div) ) {
        res = val / ddiv;
        form = (type ? _T("%0.2f Kilo") : _T("%0.2f K"));
        SPRINTF(pb, form, res);
        lev = 2;
    } else if( bytes < (div*div*div) ) {
        res = (val / (ddiv*ddiv));
        form = (type ? _T("%0.2f Mega") : _T("%0.2f M"));
        SPRINTF(pb, form, res);
        lev = 3;
    } else { // if( byts <  (1024*1024*1024*1024)){
        // switch to all double
      double db = (ddiv*ddiv*ddiv);   // x3
      double db2 = db * ddiv;   // x4
      if( val < db2 )
      {
         res = val / db;
         form = (type ? _T("%0.2f Giga") : _T("%0.2f G"));
         SPRINTF(pb, form, res);
         lev = 4;
      }
      else
      {
         db *= ddiv;  // x4
         db2 *= ddiv; // x5
         if( val < db2 )
         {
            res = val / db;
            form = (type ? _T("%0.2f Tera") : _T("%0.2f T"));
            SPRINTF(pb, form, res);
            lev = 5;
         }
         else
         {
            db *= ddiv;  // x5
            db2 *= ddiv; // x6
            if( val < db2 )
            {
               res = val / db;
               form = (type ? _T("%0.2f Peta") : _T("%0.2f P"));
               SPRINTF(pb, form, res);
               lev = 6;
            }
            else
            {
               db *= ddiv;  // x6
               db2 *= ddiv; // x7
               if( val < db2 )
               {
                  res = val / db;
                  form = (type ? _T("%0.2f Exa") : _T("%0.2f E"));
                  SPRINTF(pb, form, res);
                  lev = 7;
               }
               else
               {
                   db *= ddiv;  // x7
                   db2 *= ddiv; // x8
                   if (val < db2)
                   {
                      res = val / db;
                      form = (type ? _T("%0.2f Zetta") : _T("%0.2f Z"));
                      SPRINTF(pb, form, res);
                      lev = 8;
                   } else {
                       db *= ddiv;  // x8
                       db2 *= ddiv; // x9
                       res = val / db;
                       form = (type ? _T("%0.2f Yotta") : _T("%0.2f Y"));
                       SPRINTF(pb, "%0.2f", res);
                       form = (type ? _T(" Yotta") : _T(" Y"));
                       strcat(pb,form);
                       lev = 9;
                       if ( val > db2 ) {
                           strcat(pb,"+");
                           val = get_value_level( val, ddiv );
                       }
                   }
               }
            }
         }
      }
   }
   if (add) {
       if (type)
           strcat(pb,"Bytes");
       else
           strcat(pb,"B");
   }
   if(ovr)
       strcat(pb,"+++");

   if ( plev )
       *plev = lev;
   //if( ascii > 0 )
   //   Convert_2_ASCII(pb);
   return pb;
}

////////////////////////////////////////////////////////////////////////
// timing
#define USE_PERF_COUNTER
#if (defined(WIN32) && defined(USE_PERF_COUNTER))
// QueryPerformanceFrequency( &frequency ) ;
// QueryPerformanceCounter(&timer->start) ;
double get_seconds()
{
    static double dfreq;
    static bool done_freq = false;
    static bool got_perf_cnt = false;
    if (!done_freq) {
        LARGE_INTEGER frequency;
        if (QueryPerformanceFrequency( &frequency )) {
            got_perf_cnt = true;
            dfreq = (double)frequency.QuadPart;
        }
        done_freq = true;
    }
    double d;
    if (got_perf_cnt) {
        LARGE_INTEGER counter;
        QueryPerformanceCounter (&counter);
        d = (double)counter.QuadPart / dfreq;
    }  else {
        DWORD dwd = GetTickCount(); // milliseconds that have elapsed since the system was started
        d = (double)dwd / 1000.0;
    }
    return d;
}

#else // !WIN32
double get_seconds()
{
    struct timeval tv;
    gettimeofday(&tv,0);
    double t1 = (double)(tv.tv_sec+((double)tv.tv_usec/1000000.0));
    return t1;
}
#endif // WIN32 y/n

///////////////////////////////////////////////////////////////////////////////

// see : http://geoffair.org/misc/powers.htm
char *get_seconds_stg( double dsecs )
{
    static char _s_secs_buf[256];
    char *cp = _s_secs_buf;
    sprintf(cp,"%g",dsecs);
    if (dsecs < 0.0) {
        strcpy(cp,"?? secs");
    } else if (dsecs < 0.0000000000001) {
        strcpy(cp,"~0 secs");
    } else if (dsecs < 0.000000005) {
        // nano- n 10 -9 * 
        dsecs *= 1000000000.0;
        sprintf(cp,"%.3f nano-secs",dsecs);
    } else if (dsecs < 0.000005) {
        // micro- m 10 -6 * 
        dsecs *= 1000000.0;
        sprintf(cp,"%.3f micro-secs",dsecs);
    } else if (dsecs <  0.005) {
        // milli- m 10 -3 * 
        dsecs *= 1000.0;
        sprintf(cp,"%.3f milli-secs",dsecs);
    } else if (dsecs < 60.0) {
        sprintf(cp,"%.3f secs",dsecs);
    } else {
        int mins = (int)(dsecs / 60.0);
        dsecs -= (double)mins * 60.0;
        if (mins < 60) {
            sprintf(cp,"%d:%.3f min:secs", mins, dsecs);
        } else {
            int hrs = mins / 60;
            mins -= hrs * 60;
            sprintf(cp,"%d:%02d:%.3f hrs:min:secs", hrs, mins, dsecs);
        }
    }
    return cp;
}

////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
// #include "msc_unistd.hxx"

// #ifdef USE_WAIT_OBJECT  // User choosable option
////////////////////////////////////////////////////////////////////////////////////////////////////
// A neat alternate WIN32 implementation - uses WaitForSingleObject on a timer handle
// from : http://stackoverflow.com/questions/5801813/c-usleep-is-obsolete-workarounds-for-windows-mingw
int usleep_wait(int usec) 
{ 
    int iret = 0;
    HANDLE timer; 
    LARGE_INTEGER ft; 

    ft.QuadPart = -( 10 * usec ); // Convert to 100 nanosecond interval, negative value indicates relative time

    timer = CreateWaitableTimer(NULL, TRUE, NULL); 
    if (timer) {
        SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0); 
        WaitForSingleObject(timer, INFINITE); 
        CloseHandle(timer); 
    } else
        iret = -1;
    return iret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// #else // !#ifdef USE_WAIT_OBJECT
////////////////////////////////////////////////////////////////////////////////////////////////////

int usleep(int microseconds)
{
    int iret = 0;
    int ms = microseconds / 1000;   // get whole ms, if any
    int micro = microseconds - (ms * 1000);  // any remainder microseconds

    Sleep(ms);  // sleep quietly for any whole milli-seconds

    if (micro)      // if a microsecond remainder... ahhh...
    {
        LARGE_INTEGER _time1, _time2;
        _time1.QuadPart = 0;
        _time2.QuadPart = 0;
        if (QueryPerformanceCounter(&_time1)) // get mircoseconds now
        {
            do 
	        { 
                // just burn up CPU time, but ONLY for any microseconds remainder
                if (!QueryPerformanceCounter(&_time2))
                {
                    iret = -1;
                    break;
                }
            } while ( (_time2.QuadPart - _time1.QuadPart) < micro ); 
        } else
            iret = -1;
    }

    return iret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// #endif // #ifdef USE_WAIT_OBJECT y/n

#endif // _MSC_VER
////////////////////////////////////////////////////////////////////////
#ifndef ISDIGIT
#define ISDIGIT(a) ((a >= '0') && (a <= '9'))
#endif  // ISDIGIT

int IsInteger(const char *arg)
{
    if (!arg)
        return 0;
    size_t ii, len = strlen(arg);
    int c;
    for (ii = 0; ii < len; ii++) {
        c = arg[ii];
        if (!ISDIGIT(c))
            return 0;
    }
    return 1;
}

////////////////////////////////////////////////////////////////////////

// eof - test-utils.cxx
