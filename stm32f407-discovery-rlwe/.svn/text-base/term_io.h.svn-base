#ifndef TERM_IO_H_
#define TERM_IO_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>
#include "comm.h"

#define xgetc() (char)comm_get()
#define xavail() comm_test()

int xatoi (char**, long*);
void xputc (char);
void xputs (const char*);
void xitoa (long, int, int);
void xprintf (const char*, ...) __attribute__ ((format (__printf__, 1, 2)));
void put_dump (const uint8_t*, uint32_t ofs, int cnt);
void get_line (char*, int len);
int get_line_r (char*, int len, int *idx);

#ifdef __cplusplus
 }
#endif

#endif /* TERM_IO_H_ */
