// Basic code from ChaN, minor modifications by Martin Thomas

#include <stdarg.h>
#include "term_io.h"

int xatoi (char **str, long *res)
{
  uint32_t val;
  uint8_t c, radix, s = 0;

  while ((c = **str) == ' ')
    (*str)++;
  if (c == '-')
  {
    s = 1;
    c = *(++(*str));
  }
  if (c == '0')
  {
    c = *(++(*str));
    if (c <= ' ')
    {
      *res = 0;
      return 1;
    }
    if (c == 'x')
    {
      radix = 16;
      c = *(++(*str));
    }
    else
    {
      if (c == 'b')
      {
        radix = 2;
        c = *(++(*str));
      }
      else
      {
        if ((c >= '0') && (c <= '9'))
          radix = 8;
        else
          return 0;
      }
    }
  }
  else
  {
    if ((c < '1') || (c > '9'))
      return 0;
    radix = 10;
  }
  val = 0;
  while (c > ' ')
  {
    if (c >= 'a')
      c -= 0x20;
    c -= '0';
    if (c >= 17)
    {
      c -= 7;
      if (c <= 9)
        return 0;
    }
    if (c >= radix)
      return 0;
    val = val * radix + c;
    c = *(++(*str));
  }
  if (s)
    val = -val;
  *res = val;
  return 1;
}

void xputc (char c)
{
  if (c == '\n')
    comm_put('\r');
  comm_put(c);
}

void xputs (const char* str)
{
  while (*str)
    xputc(*str++);
}

void xitoa (long val, int radix, int len)
{
  uint8_t c, r, sgn = 0, pad = ' ';
  uint8_t s[20], i = 0;
  uint32_t v;

  if (radix < 0)
  {
    radix = -radix;
    if (val < 0)
    {
      val = -val;
      sgn = '-';
    }
  }
  v = val;
  r = radix;
  if (len < 0)
  {
    len = -len;
    pad = '0';
  }
  if (len > 20)
    return;
  do
  {
    c = (uint8_t) (v % r);
    if (c >= 10)
      c += 7;
    c += '0';
    s[i++] = c;
    v /= r;
  } while (v);
  if (sgn)
    s[i++] = sgn;
  while (i < len)
    s[i++] = pad;
  do
    xputc(s[--i]);
  while (i);
}

void xprintf (const char* str, ...)
{
  va_list arp;
  int d, r, w, s, l;

  va_start(arp, str);

  while ((d = *str++) != 0)
  {
    if (d != '%')
    {
      xputc(d);
      continue;
    }
    d = *str++;
    w = r = s = l = 0;
    if (d == '0')
    {
      d = *str++;
      s = 1;
    }
    while ((d >= '0') && (d <= '9'))
    {
      w += w * 10 + (d - '0');
      d = *str++;
    }
    if (s)
      w = -w;
    if (d == 'l')
    {
      l = 1;
      d = *str++;
    }
    if (!d)
      break;
    if (d == 's')
    {
      xputs(va_arg(arp, char*));
      continue;
    }
    if (d == 'c')
    {
      xputc((char) va_arg(arp, int));
      continue;
    }
    if (d == 'u')
      r = 10;
    if (d == 'd')
      r = -10;
    if (d == 'X' || d == 'x')
      r = 16; // 'x' added by mthomas in increase compatibility
    if (d == 'b')
      r = 2;
    if (!r)
      break;
    if (l)
    {
      xitoa((long) va_arg(arp, long), r, w);
    }
    else
    {
      if (r > 0)
        xitoa((unsigned long) va_arg(arp, int), r, w);
      else
        xitoa((long) va_arg(arp, int), r, w);
    }
  }

  va_end(arp);
}

void put_dump (const uint8_t *buff, uint32_t ofs, int cnt)
{
  uint8_t n;

  xprintf("%08lX ", ofs);
  for (n = 0; n < cnt; n++)
    xprintf(" %02X", buff[n]);
  xputc(' ');
  for (n = 0; n < cnt; n++)
  {
    if ((buff[n] < 0x20) || (buff[n] >= 0x7F))
      xputc('.');
    else
      xputc(buff[n]);
  }
  xputc('\n');
}

void get_line (char *buff, int len)
{
  char c;
  int idx = 0;

  for (;;)
  {
    c = xgetc();
    if (c == '\r')
      break;
    if ((c == '\b') && idx)
    {
      idx--;
      xputc(c);
      xputc(' ');
      xputc(c); // added by mthomas for Eclipse Terminal plug-in
    }
    if (((uint8_t) c >= ' ') && (idx < len - 1))
    {
      buff[idx++] = c;
      xputc(c);
    }
  }
  buff[idx] = 0;
  xputc('\n');
}

// function added by mthomas:
int get_line_r (char *buff, int len, int* idx)
{
  char c;
  int retval = 0;
  int myidx;

  if (xavail())
  {
    myidx = *idx;
    c = xgetc();
    if (c == '\r')
    {
      buff[myidx] = 0;
      xputc('\n');
      retval = 1;
    }
    else
    {
      if ((c == '\b') && myidx)
      {
        myidx--;
        xputc(c);
        xputc(' ');
        xputc(c); // added by mthomas for Eclipse Terminal plug-in
      }
      if (((uint8_t) c >= ' ') && (myidx < len - 1))
      {
        buff[myidx++] = c;
        xputc(c);
      }
    }
    *idx = myidx;
  }

  return retval;
}
