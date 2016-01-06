// test-time.hxx
#ifndef _TEST_TIME_HXX_
#define _TEST_TIME_HXX_

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#include <stdio.h>
#ifdef _MSC_VER
#include <tchar.h>
#include <crtdbg.h>
#include <process.h>    // for _beginthread
#include <conio.h>      // for _kbhit() and _getch()
#include <direct.h>     // for _getcwd()
//#include <io.h>
#else // !_MSC_VER
#include <unistd.h>
#include <fcntl.h>
#endif // _MSC_VER y/n

#include <string>
#include <limits.h>     // for UINT_MAX ...
#include <float.h>      // for DBL_MAX, FLT_MAX (my MAX_FLOAT)

#include "win_Timer.hxx"
#include "test-utils.hxx"

#ifdef _DEBUG
   #define DEBUG_CLIENTBLOCK   new( _CLIENT_BLOCK, __FILE__, __LINE__)
#else
   #define DEBUG_CLIENTBLOCK
#endif // _DEBUG

#endif // #ifndef _TEST_TIME_HXX_
// eof - test-time.hxx

