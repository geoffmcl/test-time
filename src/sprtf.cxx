// sprtf.cxx

#ifdef _MSC_VER
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#pragma warning( disable:4996 )
#endif // #ifndef _CRT_SECURE_NO_DEPRECATE
#define STRICT 1
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else // !_MSC_VER
#define strcmpi strcasecmp
#define _fileno fileno
#endif // _MSC_VER y/n
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "sprtf.hxx"

#define  MXIO     256
#ifndef M_MX_SPRT_BUF
#define M_MX_SPRT_BUF 4096
#endif

static char def_log[] = "temptt.txt";
static char logfile[256] = "\0";
static FILE * outfile = NULL;

void flush_log_file(void)
{
    if ( VFP(outfile) )
        fflush(outfile);
}

int get_log_handle(void)
{
    if ( VFP(outfile) )
        return _fileno(outfile);
    return -1; // INVALID_HANDLE_VALUE;
}

char * get_runtime_directory( char * prt )
{
    *prt = 0;
#ifdef _MSC_VER
    if( GetModuleFileName( NULL, prt, 256 ) ) {
        size_t len = strlen(prt);
        size_t i;
        int c;
        for (i = len - 1; i > 0; i--) {
            c = prt[i];
            if (( c == '\\' )||( c == '/'))
                break;
            prt[i] = 0;
        }
    }
#else // _MSC_VER
    // need port for this function
#endif // _MSC_VER y/n
    return prt;
}

void set_def_log(char * plog)
{
    char * pl = logfile;
    get_runtime_directory(plog);
    strcat(plog,def_log);
}


int   open_log_file( void )
{
   if (logfile[0] == 0)
      set_def_log(logfile);
   outfile = fopen(logfile, "wb");
   if( outfile == 0 ) {
      outfile = (FILE *)-1;
      printf("WARNING: Failed to open log file [%s] ...\n", logfile);
      return 0;   /* failed */
   }
   return 1; /* success */
}

void close_log_file( void )
{
   if( VFP(outfile) ) {
      fclose(outfile);
   }
   outfile = NULL;
}

void   set_log_file( char * nf )
{
   if (logfile[0] == 0)
      set_def_log(logfile);
   if ( nf && *nf && strcmpi(nf,logfile) ) {
      close_log_file(); // remove any previous
      strcpy(logfile,nf); // set new name
      open_log_file();  // and open it ... anything previous written is 'lost'
   }
}

static void oi( char * pc )
{
   int len = (int)strlen(pc);
   if(len) {
      if( outfile == 0 ) {
         open_log_file();
      }
      if( VFP(outfile) ) {
         int w = (int)fwrite( pc, 1, len, outfile );
         if( w != len ) {
            fclose(outfile);
            outfile = (FILE *)-1;
            printf("WARNING: Failed write to log file [%s] ...\n", logfile);
         }
         fflush( outfile );
      }
      fwrite( pc, 1, len, stdout );
      //printf(pc);
   }
}

static void	prt( char * ps )
{
   static char buf[MXIO + 32];
	char * pb = buf;
	size_t i, j, k;
	char   c, d;
   i = strlen(ps);
	if(i) {
		k = 0;
		d = 0;
		for( j = 0; j < i; j++ )
		{
			c = ps[j];
			if( c == 0x0d ) {
				if( (j+1) < i ) {
					if( ps[j+1] != 0x0a ) {
						pb[k++] = c;
						c = 0x0a;
					}
            } else {
					pb[k++] = c;
					c = 0x0a;
				}
			} else if( c == 0x0a ) {
				if( d != 0x0d ) {
					pb[k++] = 0x0d;
				}
			}
			pb[k++] = c;
			d = c;
			if( k >= MXIO ) {
				pb[k] = 0;
				oi( pb );
				k = 0;
			}
		}	// for length of string
		if( k ) {
			//if( ( gbCheckCrLf ) &&
			//	( d != 0x0a ) ) {
				// add Cr/Lf pair
				//pb[k++] = 0x0d;
				//pb[k++] = 0x0a;
				//pb[k] = 0;
			//}
			pb[k] = 0;
			oi( pb );
		}
	}
}

int MCDECL sprtf( char * pf, ... )
{
   static char _s_sprtfbuf[M_MX_SPRT_BUF];
   char * pb = _s_sprtfbuf;
   int   i;
   va_list arglist;
   va_start(arglist, pf);
   i = vsprintf( pb, pf, arglist );
   va_end(arglist);
   prt(pb);
   return i;
}

// eof - sprtf.cxx
