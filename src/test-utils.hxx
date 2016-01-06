// test-utils.hxx
#ifndef _TEST_UTILS_HXX_
#define _TEST_UTILS_HXX_
#ifdef _MSC_VER
#include <stdint.h>
#else
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#endif

#define TYP_LONG    0x00000001
#define TYP_ADDB    0x00000002
#ifndef PTSTR
typedef char * PTSTR;
#endif
#ifndef _TCHAR
typedef char _TCHAR;
#endif
#ifndef _MSC_VER
typedef int64_t __int64;
#endif
#ifndef VOID
#define VOID void
#endif
#ifndef _UI64_MAX
#define _UI64_MAX ULLONG_MAX
#endif

#ifdef _MSC_VER
#define PF64U   "%I64u"
#else
#define PF64U   "%" PRIu64
#endif

extern char * GetNxtBuf( void );
extern void nice_num( _TCHAR * dst, _TCHAR * src ); // get nice number, with commas
extern char * get_nice_number_stg( int num );
extern char * get_k_num_with_2_sig_chars( __int64 i64,
                                           int type, int divK, int * plev );
extern char * get_kdouble_with_2_sig_chars( double d, int type = 0, int divK = 0, int * plev = NULL);
extern int get_value_level( double val, double dd );

// 20121230
extern double get_seconds();
extern char *get_seconds_stg( double dsecs );

#ifdef _MSC_VER
extern int usleep(int usec);
extern int usleep_wait(int usec);
#endif // _MSC_VER
#endif // #ifndef _TEST_UTILS_HXX_
// eof - test-utils.hxx
