/*

timer-queue.cxx

https://msdn.microsoft.com/en-us/library/ms682485(v=vs.85).aspx
BOOL WINAPI CreateTimerQueueTimer(
_Out_    PHANDLE             phNewTimer,
_In_opt_ HANDLE              TimerQueue,
_In_     WAITORTIMERCALLBACK Callback,
_In_opt_ PVOID               Parameter,
_In_     DWORD               DueTime, // The amount of time in milliseconds relative to the current time that must elapse before the timer is signaled for the first time.
_In_     DWORD               Period,
_In_     ULONG               Flags
);

Flags:
WT_EXECUTEDEFAULT 
0x00000000
By default, the callback function is queued to a non-I/O worker thread.

WT_EXECUTEINTIMERTHREAD
0x00000020
The callback function is invoked by the timer thread itself. This flag should be used only for short tasks or it could affect other timer operations.
The callback function is queued as an APC. It should not perform alertable wait operations.

WT_EXECUTEINIOTHREAD
0x00000001
This flag is not used.
Windows Server 2003 and Windows XP:  The callback function is queued to an I/O worker thread. This flag should be used if the function should be executed in a thread that waits in an alertable state. I/O worker threads were removed starting with Windows Vista and Windows Server 2008.

WT_EXECUTEINPERSISTENTTHREAD
0x00000080
The callback function is queued to a thread that never terminates. It does not guarantee that the same thread is used each time. This flag should be used only for short tasks or it could affect other timer operations. This flag must be set if the thread calls functions that use APCs. For more information, see Asynchronous Procedure Calls.
Note that currently no worker thread is truly persistent, although no worker thread will terminate if there are any pending I/O requests.

WT_EXECUTELONGFUNCTION
0x00000010
The callback function can perform a long wait. This flag helps the system to decide if it should create a new thread.

WT_EXECUTEONLYONCE
0x00000008
The timer will be set to the signaled state only once. If this flag is set, the Period parameter must be zero.

WT_TRANSFER_IMPERSONATION
0x00000100
Callback functions will use the current access token, whether it is a process or impersonation token. If this flag is not specified, callback functions execute only with the process token.
Windows XP:  This flag is not supported until Windows XP with SP2 and Windows Server 2003.

============================================================================
VOID CALLBACK WaitOrTimerCallback(
_In_ PVOID   lpParameter,
_In_ BOOLEAN TimerOrWaitFired
);

============================================================================
BOOL WINAPI DeleteTimerQueueTimer(
_In_opt_ HANDLE TimerQueue,
_In_     HANDLE Timer,
_In_opt_ HANDLE CompletionEvent
);

*/


#include <windows.h>
#include <stdio.h>
#include "test-utils.hxx"
#include "sprtf.hxx"

#ifndef SPRTF
#define SPRTF printf
#endif // SPRTF

static HANDLE gDoneEvent;

VOID CALLBACK TimerRoutine(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
    if (lpParam == NULL)
    {
        SPRTF("CB: TimerRoutine lpParam is NULL\n");
    }
    else
    {
        // lpParam points to the argument; in this case it is an int
        SPRTF("CB: Timer routine called. Parameter is %u ms.\n", *(DWORD *)lpParam);
        if ( TimerOrWaitFired )
        {
            SPRTF("CB: The wait timed out.\n");
        }
        else
        {
            SPRTF("CB: The wait event was signaled.\n");
        }
    }

    SPRTF("CB: SetEvent(gDoneEvent) %p\n", gDoneEvent);
    SetEvent(gDoneEvent);
}

int main(int argc, char **argv)
{
    double start = get_seconds();
    HANDLE hTimer = NULL;
    HANDLE hTimerQueue = NULL;
    DWORD dwto = 10000;
    char * arg;
    int i;
    set_log_file((char *)"temptq.txt");

    for (i = 1; i < argc; i++) {
        arg = argv[i];
        if (IsInteger(arg)) {
            dwto = atoi(arg);
        }
        else {
            SPRTF("Test timer queue...\n");
            SPRTF("Only command allowed is an integer timeout in ms... def=%u\n", dwto);
            SPRTF("Error: Unknown command '%s'! Aborting...\n", arg);
            return 1;
        }
    }

    SPRTF("Create gDoneEvent = CreateEvent(NULL, TRUE, FALSE, NULL);\n");
    // Use an event object to track the TimerRoutine execution
    gDoneEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (NULL == gDoneEvent)
    {
        SPRTF("CreateEvent failed (%d)\n", GetLastError());
        return 1;
    }

    // Create the timer queue.
    SPRTF("Create hTimerQueue = CreateTimerQueue();\n");
    hTimerQueue = CreateTimerQueue();
    if (NULL == hTimerQueue)
    {
        SPRTF("CreateTimerQueue failed (%d)\n", GetLastError());
        return 2;
    }


    // Set a timer to call the timer routine in 10 seconds.
    SPRTF("CreateTimerQueueTimer(&hTimer, hTimerQueue, TimerRoutine, &dwto, %u, 0, 0);\n", dwto);

    ///////////////////////////////////////////////////////////////
    ///// time the event
    double begin = get_seconds();   // get time at beginning...
    if (!CreateTimerQueueTimer( &hTimer, hTimerQueue, 
            (WAITORTIMERCALLBACK)TimerRoutine, &dwto, dwto, 0, 0))
    {
        SPRTF("CreateTimerQueueTimer failed (%d)\n", GetLastError());
        return 3;
    }

    if (dwto > 1000) {
        // TODO: Do other useful work here 
        SPRTF("Wait for callback timer routine in %u ms...\n", dwto);
    }

    // Wait for the timer-queue thread to complete using an event 
    // object. The thread will signal the event at that time.
    if (WaitForSingleObject(gDoneEvent, INFINITE) != WAIT_OBJECT_0) {
        SPRTF("WaitForSingleObject failed (%d)\n", GetLastError());
    }
    SPRTF("Timeout in %s...\n", get_seconds_stg(get_seconds() - begin));

    ///////////////////////////////////////////////////////////////
    //// SHUTDOWN ////
    SPRTF("CloseHandle(gDoneEvent); %p\n", gDoneEvent);
    CloseHandle(gDoneEvent);
    // Delete all timers in the timer queue.
    SPRTF("DeleteTimerQueue(hTimerQueue);\n");
    if (!DeleteTimerQueue(hTimerQueue))
        SPRTF("DeleteTimerQueue failed (%d)\n", GetLastError());
    ///////////////////////////////////////////////////////////////

    SPRTF("Prcoess ran for %s...\n", get_seconds_stg(get_seconds() - start));
    return 0;
}

// eof
