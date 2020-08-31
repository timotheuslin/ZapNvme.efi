/** @file
  AMD PciePort register dump.
  Ref. AMD document #54945 Rev 3.03 Jun 14, 2019,
    "Processor Programming Reference (PPR) for AMD Family 17h Models 01h,08h, Revision B2 Processors", Chapter 11 UMC
**/

#include <Uefi.h>
#include <Library/PciLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/ShellCEntryLib.h>

#define PCIEPORT0F0_PHYSICAL_MNEMONIC 0x11140000
#define PCIEPORT0FX_INCREMENT 0x1000

//#define DEBUG_THIS 1

//
// convert a double-byte unicode-string to a UINT64
//
UINT64 touint64(CHAR16 *src, BOOLEAN *status)
{
  UINT64 ret=0, reta=0, base=10;

  if ((src[0] == '0') && (src[1] == 'x' || src[1] == 'X')) {
    base = 0x10;
    src += 2;
  }

  while(*src) {
    reta = ret = ret * base;
    if (*src <= '9' && *src >= '0') {
      ret += *src - '0';
    }
    else if (base == 16 && *src >= 'A' && *src <= 'F') {
      ret += *src - 'A' + 10;
    }
    else if (base == 16 && *src >= 'a' && *src <= 'f') {
      ret += *src - 'a' + 10;
    }
    else {
      *status = 1;
      return 0;
    }
    //assert(ret >= reta);   // overflow
    if (ret < reta) {
      *status = 2;
      return 0;
    }
    src++;
  }
  *status = 0;
  return ret;
}


#ifndef NB_SMN_INDEX_2
#define NB_SMN_INDEX_2  0xb8
#define NB_SMN_DATA_2   0xbc
#endif

//
// Smn Read.
// Note. This works for SP4r2, single-socket, single-die only. 
//
UINT32 SmnRegRead32 (
  UINT32  physical_mnemonic,
  UINT32  offset,
  UINTN   BusNumberBase
  )
{
  UINTN   pciAddress;
  UINT32  smnIndex;
  UINT32  value;

#if DEBUG_THIS
  Print (L"SmnRegRead32(1) - physical_mnemonic: %lx, offset: %lx\n", physical_mnemonic, offset);
#endif
  pciAddress = (BusNumberBase << 20) + NB_SMN_INDEX_2;
  smnIndex = physical_mnemonic + offset;
  PciWrite32 (pciAddress, smnIndex);
  pciAddress = (BusNumberBase << 20) + NB_SMN_DATA_2;
  value = PciRead32 (pciAddress);
#if DEBUG_THIS
  Print (L"SmnRegRead32(2) - value %lx\n", value);
#endif

  return value;
}


INTN EFIAPI ShellAppMain(IN UINTN argc, IN CHAR16 **argv)
{
  BOOLEAN status;
  UINT32 fn=0xFFFF, offset=0xFFFF, readout=0;

#if DEBUG_THIS
  {
    UINTN i;
    Print (L"argc: %d\n", argc);
    for (i=0; i<argc; i++) {
      Print(L"argv[%d]: %s\n", i, argv[i]);
    }
  }
#endif

  if (argc == 1) {
    Print (L"AMD PCIEPORT-0 Fn[7..0] Register Dump.\n");
    Print (L"Usage:\n");
    Print (L"  PCIEPORT Fn Offset\n");
    Print (L"Examples:\n");
    Print (L"  PCIEPORT 0 0x284 -> Dump PCIEPORT0 F0 0x284 (PCIEPORT::PCIE_LC_TRAINING_CNTL)\n");
    return 0;
  }
  if (argc > 1) {
    fn = (UINT32) touint64(argv[1], &status);
    if (status) {
      Print (L"Invalid integer: %s.\n", argv[1]);
      return 0;
    }
    if (fn > 7) {
      Print (L"Invalid Function number: %s.\nShould be within [7..0]\n", argv[1]);
      return 0;
    }
  }
  if (argc > 2) {
    offset = (UINT32) touint64(argv[2], &status);
    if (status) {
      Print (L"Invalid integer: %s.\n", argv[2]);
      return 0;
    }
    if (offset >= 0xFFF) {
      Print (L"Invaid Offset number: %s.\nShould be within [0xFFC..0]\n", argv[2]);
      return 0;
    }
  }

  offset -= offset % 4;
  readout = SmnRegRead32(PCIEPORT0F0_PHYSICAL_MNEMONIC + fn*PCIEPORT0FX_INCREMENT, offset, 0);
  Print (L"PCIEPORT0 F%d [0x%4.4X]: %4.4X_%4.4Xh\n", fn, offset, readout>>16, readout&0xFFFF);

  return 0;
}
