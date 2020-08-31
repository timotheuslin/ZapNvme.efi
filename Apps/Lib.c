
#include <ZapNvme.h>

VOID SafeFreePool(void **h)
{
    if (*h) {
        gBS->FreePool(*h);
        *h = NULL;
    }
}

VOID SafeShellCloseFile(SHELL_FILE_HANDLE *sfh)
{
  if (*sfh != NULL) {
    ShellCloseFile(sfh);
    *sfh = NULL;
  }
}

CHAR16 *
EFIAPI
StrUprS (
  OUT CHAR16 *Destination,
  IN  UINTN DestMax,
  IN  CONST CHAR16 *Source,
  OUT UINT8 *Truncated
)
{
  CHAR16 *Destination0 = Destination;

  while (*Destination && (Destination < (Destination + DestMax))) {
    *Destination = CharToUpper(*Source);
    Destination++;
    Source++;
  }
  *Destination = L'\0';
  *Truncated = (*Source == *Destination);

  return Destination0;
}


INTN
EFIAPI
StriCmp (
  IN CONST CHAR16 *FirstString,
  IN CONST CHAR16 *SecondString
)
{
  INTN sub = 0;

  do {
    sub = CharToUpper(*FirstString) - CharToUpper(*SecondString);
    if (sub != 0) {
      break;
    }
    FirstString++;
    SecondString++;
  } while (*FirstString && *SecondString);

  return sub;
}

INTN
EFIAPI
StrinCmp (
  IN CONST CHAR16 *FirstString,
  IN CONST CHAR16 *SecondString,
  IN UINTN Length
)
{
  INTN sub = 0;

  if (!Length) {
      return 0;
  }
  do {
    sub = CharToUpper(*FirstString) - CharToUpper(*SecondString);
    if (sub != 0) {
      break;
    }
    FirstString++;
    SecondString++;
    Length--;
  } while (*FirstString && *SecondString && Length);

  return Length ? (CharToUpper(*FirstString) - CharToUpper(*SecondString)) : sub;
}


CHAR16 *StrStrip(CHAR16 *src)
{
  CHAR16 *src_end = src+StrLen(src)-1;

  while (*src == L' ') {
    src++;
  }
  while(src_end >= src && *src_end == L' ') {
    *src_end = L'\0';
    src_end--;
  }
  return src;
}


int isBlockEmpty(VOID *data, UINT64 size)
{
  UINT8 *d = (UINT8*) data;
  UINT8 d0 = *d;
  UINT64 i;

  for (i=1; i<size; i++) {
    if (d0 != *d) {
      return 0;
    }
  }
  return 1;
}

