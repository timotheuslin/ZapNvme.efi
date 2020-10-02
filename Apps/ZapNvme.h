#pragma once

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/ShellLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DevicePathLib.h>
#include <Library/ShellCEntryLib.h>
#include <Library/ShellCommandLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Include/Protocol/DiskIo.h>
#include <Include/Protocol/DiskInfo.h>
#include <Include/Protocol/DriverBinding.h>
#include <Include/Protocol/ComponentName2.h>

#include <IndustryStandard/Atapi.h>
#include <IndustryStandard/Scsi.h>
#include <IndustryStandard/Nvme.h>

#include <Protocol/BlockIo.h>
#include <Protocol/DevicePath.h>
#include <Protocol/EraseBlock.h>
#include <Protocol/IdeControllerInit.h>
#include <Protocol/NvmExpressPassthru.h>

#define nGptSector0 34
#define nGptSector1 33

#define NVME_PRODUCT_MODEL_NUMBER_SIZE 40
#define NVME_PRODUCT_SERIAL_NUMBER_SIZE 20
#define NVME_DESCRIPTION_SIZE (NVME_PRODUCT_MODEL_NUMBER_SIZE+NVME_PRODUCT_SERIAL_NUMBER_SIZE+2)

#define USER_NVME_PRODUCT_SERIAL_NUMBER_SIZE 6


extern UINT8 Verbose_Level;
extern UINT8 Dangerous_Zap;
extern UINT8 Dangerous_Zap_Dry_Run;
extern UINT8 Verbose_Dump;
extern UINT8 Save_GPTs;

// UefiBootManagerLib | MdeModulePkg/Library/UefiBootManagerLib/BmBootDescription.c
CHAR16 *
BmGetNvmeDescription (
  IN EFI_HANDLE Handle
  );

// UefiBootManagerLib | MdeModulePkg/Library/UefiBootManagerLib/BmBootDescription.c
CHAR16 *
BmGetBootDescription (
  IN EFI_HANDLE Handle
);

VOID hexdump(VOID* data, UINT32 len);

VOID SafeFreePool(void **h);
VOID SafeShellCloseFile(SHELL_FILE_HANDLE *sfh);

int isBlockEmpty(VOID *data, UINT64 size);

CHAR16 *
EFIAPI
StrUprS (
  OUT CHAR16 *Destination,
  IN  UINTN DestMax,
  IN  CONST CHAR16 *Source,
  OUT UINT8 *Truncated
);

INTN
EFIAPI
StriCmp (
  IN CONST CHAR16 *FirstString,
  IN CONST CHAR16 *SecondString
);

INTN
EFIAPI
StrinCmp (
  IN CONST CHAR16 *FirstString,
  IN CONST CHAR16 *SecondString,
  IN UINTN Length
);

CHAR16 *StrStrip(CHAR16 *src);


EFI_STATUS
SaveGpts (
  IN EFI_BLOCK_IO_PROTOCOL *BlkIo,
  IN CHAR16 *Sn
);

EFI_STATUS
ZapGpts(
  IN EFI_BLOCK_IO_PROTOCOL *BlkIo
);

EFI_STATUS BlockIoProtocolHandles(EFI_HANDLE **HandleBuffer, UINTN *HandleCount);
INT64 NvmeInfo(VOID);
EFI_STATUS NvmeZap (CHAR16 *USER_SN, UINTN Length);

#define BlkIo_Terminate -1
#define BlkIo_Continue -2
#define BlkIo_Error -3
INT64 BlkIo_Iterator(INT64 index, EFI_HANDLE *Handle, EFI_BLOCK_IO_PROTOCOL **BlkIo);
INT64 NVME_Iterator(INT64 index, EFI_BLOCK_IO_PROTOCOL **BlkIo, CHAR16 *Desc_Buffer, INT16 Desc_Max);
CHAR16 *DescToMnSn(CHAR16 *Desc, CHAR16 **Sn);
EFI_STATUS NvmeGptSave (CHAR16 *USER_SN, UINTN Length);
EFI_STATUS NvmeDump (CHAR16 *USER_SN, UINTN Length, EFI_LBA Start_Lba, UINT64 Lba_Count);