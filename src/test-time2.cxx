// test-time2.cxx
// ==========================================================================
// 2011-03-26 - A precise timer implmentation
// from : http://www.codeguru.com/cpp/w-p/system/timers/article.php/c5759/
// Many thanks for the pointer...
// But seems to have some serious limitations, like maximum wait time
// is an unsigned int worth of usecs!!!
// In my XP machine the time frequency is only each 0.469914 ns
// That is a frequency of about 2.13 GHz
// Have yet to test it in Vista or later MS OS...
// And I seem to have made a 'math' error in the time displayed!!! ;=((
// ==========================================================================

#include <stdio.h>

char * this_mod_name = (char *)__FILE__;

#ifdef _MSC_VER

#pragma warning( disable : 4311) // 'type cast' : pointer truncation from 'void *' to 'DWORD'
#pragma warning( disable : 4312) // 'type cast' : conversion from 'DWORD' to 'void *' of greater size

#include <Windows.h>
#include "sprtf.hxx"
#include "MyTimer.hxx"

// Need Winmm.lib
#pragma comment( lib, "Winmm" )

//----------------------------------------------------------------
class PreciseTimer
{
public:
   PreciseTimer() : mRes(0), toLeave(false), stopCounter(-1)
   {
      InitializeCriticalSection(&crit);
      mRes = timeSetEvent(1, 0, &TimerProc, (DWORD)this,
                          TIME_PERIODIC);
      if (!mRes) {
          fprintf(stderr,"ERROR: PreciseTimer() timeSetEvent FAILED!\n");
          DeleteCriticalSection(&crit);
          exit(1); // what else to do?
      }
   }
   virtual ~PreciseTimer()
   {
      mRes = timeKillEvent(mRes);
      DeleteCriticalSection(&crit);
   }
   
   ///////////////////////////////////////////////////////////////
   // Function name   : Wait
   // Description     : Waits for the required duration of msecs.
   //                 : Timer resolution is precisely 1 msec
   // Return type     : void  :
   // Argument        : int timeout : timeout in msecs
   ///////////////////////////////////////////////////////////////
   void Wait(int timeout)
   {
      if ( timeout )
      {
         stopCounter = timeout;
         toLeave = true;
         // this will do the actual delay - timer callback shares
         // same crit section
         EnterCriticalSection(&crit);
         LeaveCriticalSection(&crit);
      }
   }
   ///////////////////////////////////////////////////////////////
   // Function name   : TimerProc
   // Description     : Timer callback procedure that is called
   //                 : every 1msec
   //                 : by high resolution media timers
   // Return type     : void CALLBACK  :
   // Argument        : UINT uiID :
   // Argument        : UINT uiMsg :
   // Argument        : DWORD dwUser :
   // Argument        : DWORD dw1 :
   // Argument        : DWORD dw2 :
   ///////////////////////////////////////////////////////////////
   static void CALLBACK TimerProc(UINT uiID, UINT uiMsg, DWORD
                                  dwUser, DWORD dw1, DWORD dw2)
   {
      static volatile bool entered = false;
      
      PreciseTimer* pThis = (PreciseTimer*)dwUser;
      if ( pThis )
      {
         if ( !entered && !pThis->toLeave )   // block section as
                                              // soon as we can
         {
            entered = true;
            EnterCriticalSection(&pThis->crit);
         }
         else if ( pThis->toLeave && pThis->stopCounter == 0 )
                                              // leave section
                                              // when counter
                                              // has expired
         {
            pThis->toLeave = false;
            entered = false;
            LeaveCriticalSection(&pThis->crit);
         }
         else if ( pThis->stopCounter > 0 )   // if counter is set
                                              // to anything, then
                                              // continue to drop
                                              // it...
         {
            --pThis->stopCounter;
         }
      }
   }
   
private:
   MMRESULT         mRes;
   CRITICAL_SECTION crit;
   volatile bool    toLeave;
   volatile int     stopCounter;
};

//----------------------------------------------------------------
// Class usage example
//----------------------------------------------------------------
void shortDelay(unsigned int val)
{
   static LARGE_INTEGER freq = {0};
   static double average = 0;
   static int count = 0;
   static PreciseTimer timer;

   ++count;
   // sprtf("Test shortDelay for %u usecs... (cnt=%d)\n", val, count);

   LARGE_INTEGER iStart, iStop;
   if ( freq.QuadPart == 0 ) {
      QueryPerformanceFrequency(&freq);
      freq.QuadPart /= 1000; // convert to msecs counter
   }

   double sleep = 0;
   QueryPerformanceCounter(&iStart);

   timer.Wait(val); // is there anything to wait on? ... then wait

   QueryPerformanceCounter(&iStop);
   sleep = ((double)iStop.QuadPart - (double)iStart.QuadPart)
                                   / (double)freq.QuadPart;
   //average += (val ? 100.0*(sleep-val)/(double)val : 0);
   double percent = 0.0;
   if (val > 0) {
        double dval = (double)val;
        if (sleep > dval) {
            percent = (100.0 * (sleep - dval)) / dval;
        } else {
            percent = (100.0 * (dval - sleep)) / dval;
        }
   }
   average += percent;
   //sprtf("Waited for %6.3f (%ld) usecs. error = %5.2f\n", sleep, val,
   // the frequency was DIVIDED by 1000.0 (as per the original code
   sprtf("Waited for %g (%lu) usecs (%s). error = %g (%2.4f%%)\n", sleep, val,
       g_timer.time_stg(sleep / 1000.0), // or should this be * 1000.0????
       (average/(double)count),
       (100.0 - percent));
}

#endif // #ifdef _MSC_VER
// eof - test-time2.cxx
