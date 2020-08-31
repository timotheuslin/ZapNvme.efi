
#include <ZapNvme.h>

#define hexdump_bytes_in_a_line 16

VOID hexdump_line(VOID *data, UINT32 len, UINT32 address)
{
  UINT32 i;
  UINT8 *d = (UINT8*) data;
  
  Print(L"%8.8X  ", address);
  for (i=0; i<len; i++) {
    Print(L"%2.2X ", d[i]);
  }
  for (;i<hexdump_bytes_in_a_line; i++) {
    Print(L"   ");
  }
  Print(L" ");
  for (i=0; i<len; i++) {
    Print(L"%c", (d[i]<0x20 || d[i]>0x7F) ? '.' : d[i]);
  }
  Print(L"\n");
}


VOID hexdump(VOID* data, UINT32 len)
{
  UINT32 i;
  UINT8 *d = (UINT8*) data;

  for (i=0; i<len; i+=hexdump_bytes_in_a_line) {
    hexdump_line(d+i, MIN(hexdump_bytes_in_a_line, len-i), i);
  }
}