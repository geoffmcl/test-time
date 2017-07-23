// sprtf.hxx
#ifndef _sprtf_hxx_
#define _sprtf_hxx_
#ifdef   __cplusplus
extern "C" {
#endif
#ifdef _MSC_VER
#define MCDECL _cdecl
#else
#define MCDECL
#endif

extern void set_log_file( char * nf ); // set new log file name, and open it
extern int open_log_file( void ); // open the default, but is done automatically
extern int MCDECL sprtf( char * pf, ... ); // like printf, except also write to log
extern void close_log_file( void );
extern int get_log_handle(void); // get log file hande (as an 'int') -1 == INVALID
extern void flush_log_file(void);

#define  VFP(a)   ( a && ( a != (FILE *)-1 ))
#define SPRTF sprtf

#ifdef   __cplusplus
}
#endif
#endif // #ifndef _sprtf_hxx_
// eof - sprtf.hxx
