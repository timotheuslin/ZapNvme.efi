/** @file

A protocol instance counter.


** Ref: dh -p <protocol_name>

**/
#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DevicePathLib.h>
#include <Library/ShellCEntryLib.h>
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

#include <Protocol/DevicePath.h>
#include <Protocol/IdeControllerInit.h>
#include <Protocol/NvmExpressPassthru.h>


CHAR16 NVME_PATH[] = L"NVMe(0x1,00-00-00-00-00-00-00-00)";

VOID SafeFreePool(void **h)
{
    if (*h) {
        gBS->FreePool(*h);
        *h = NULL;
    }
}


/**
  Worker function that prints an EFI_GUID into specified Buffer.

  @param[in]     Guid          Pointer to GUID to print.
  @param[in]     Buffer        Buffer to print Guid into.
  @param[in]     BufferSize    Size of Buffer.

  @retval    Number of characters printed.

**/
UINTN
GuidToString (
  IN  EFI_GUID  *Guid,
  IN  CHAR16    *Buffer,
  IN  UINTN     BufferSize
  )
{
  UINTN Size;

  Size = UnicodeSPrint (
            Buffer,
            BufferSize,
            L"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            (UINTN)Guid->Data1,
            (UINTN)Guid->Data2,
            (UINTN)Guid->Data3,
            (UINTN)Guid->Data4[0],
            (UINTN)Guid->Data4[1],
            (UINTN)Guid->Data4[2],
            (UINTN)Guid->Data4[3],
            (UINTN)Guid->Data4[4],
            (UINTN)Guid->Data4[5],
            (UINTN)Guid->Data4[6],
            (UINTN)Guid->Data4[7]
            );

  //
  // SPrint will null terminate the string. The -1 skips the null
  //
  return Size - 1;
}


VOID PrintGuid(IN EFI_GUID *Guid)
{
  static CHAR16 buffer[0x256];

  GuidToString(Guid, buffer, 255);
  Print(L"%s\n", buffer);
}


UINT32 hexdump_bytes_in_a_line = 16;

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

// my_memset() to avoid intric function linkage.Wwhen the execution speed does not matter...
void my_memset(VOID *mem, UINT8 value, UINT32 len)
{
  UINT8* m = (UINT8*) mem;

  for(; len; len--) {
    *m++ = value;
  }
}

VOID PrintDevicePath(EFI_HANDLE Handle)
{
  CHAR16 *TmpStr;
  EFI_DEVICE_PATH_PROTOCOL *DevicePathP;

  DevicePathP = DevicePathFromHandle(Handle);
  TmpStr = ConvertDevicePathToText (DevicePathP, TRUE, TRUE);
  if (TmpStr == NULL) {
    Print(L"ConvertDevicePathToText(%p) FAILED ", DevicePathP);
  }
  else {
    Print(L"Disk path: %s\n", TmpStr);
  }
  SafeFreePool((void**)&TmpStr);
}


UINT32 identify_buffer_size = 0x4000;


EFI_STATUS ZapNvme(VOID)
{
    int i;
    UINT32 BufferSize;
    UINTN HandleCount;
    EFI_HANDLE Handle;
    EFI_STATUS Status;
    EFI_HANDLE *HandleBuffer;
    EFI_DISK_INFO_PROTOCOL *DiskInfo;

//    UINT8 *IdentifyData;
//    UINT8 *IdentifyData[0x1000];
//    EFI_IDENTIFY_DATA IdentifyData;
    NVME_ADMIN_NAMESPACE_DATA IdentifyData;

    Status = EFI_NOT_FOUND;

//    Status = gBS->AllocatePool(EfiBootServicesData, identify_buffer_size*2, (VOID**)&IdentifyData);
//    if (IdentifyData == NULL) {
//      Print(L"Error: AllocateZeroPool()\n");
//      return -1;
//    }

    Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiDiskInfoProtocolGuid, NULL, &HandleCount, &HandleBuffer);
    if (EFI_ERROR(Status)) {
      Print(L"Error: %r\n", Status);
      return Status;
    }
    else
      Print(L"Instances of the installed protocol: %d\n", HandleCount);

    for (i = 0; i < HandleCount; i++) {
      Print(L"i: %d\n", i);
      Handle = HandleBuffer[i];
      Print(L"Handle: %p\n", Handle);

      PrintDevicePath(Handle);

      Status = gBS->HandleProtocol (
                      Handle,
                      &gEfiDiskInfoProtocolGuid,
                      (VOID **) &DiskInfo
                      );
      if (EFI_ERROR (Status)) {
        return Status;
      }

      BufferSize   = sizeof(NVME_ADMIN_NAMESPACE_DATA);
      // Display the storage's type.
      if (CompareGuid (&DiskInfo->Interface, &gEfiDiskInfoAhciInterfaceGuid)) {
        Print(L"AHCI Storage.\n");
      }
      else if (CompareGuid (&DiskInfo->Interface, &gEfiDiskInfoIdeInterfaceGuid)) {
        Print(L"IDE Storage.\n");
      }
      else if (CompareGuid (&DiskInfo->Interface, &gEfiDiskInfoScsiInterfaceGuid)){
        Print(L"SCSI Interface Storage.\n");
      }
      else if (CompareGuid (&DiskInfo->Interface, &gEfiDiskInfoSdMmcInterfaceGuid)) {
        Print(L"SD/MMC Storage.\n");
      }
      else if (CompareGuid (&DiskInfo->Interface, &gEfiDiskInfoNvmeInterfaceGuid)) {
        Print(L"NVME Storage.\n");
        BufferSize   = sizeof(NVME_ADMIN_NAMESPACE_DATA);
      }
      else if (CompareGuid (&DiskInfo->Interface, &gEfiDiskInfoUsbInterfaceGuid)) {
        Print(L"USB Storage.\n");
      }
      else {
        CHAR16 guid_buffer[0x256];
        GuidToString(&DiskInfo->Interface, guid_buffer, sizeof(guid_buffer)-1);
        Print(L"Unknown Storage with GUID: %s\n", guid_buffer);
        BufferSize   = sizeof(NVME_ADMIN_NAMESPACE_DATA);
      }

      Status = DiskInfo->Identify (
                           DiskInfo,
                           &IdentifyData,
                           &BufferSize
                           );
      Print(L"BufferSize:  %d\n", BufferSize);
      if (EFI_ERROR (Status)) {
        switch (Status) {
        case EFI_NOT_FOUND:
          Status = DiskInfo->Inquiry (
                               DiskInfo,
                               &IdentifyData,
                               &BufferSize
                               );
          if (!EFI_ERROR (Status)) {
            break;
          }
        case EFI_BUFFER_TOO_SMALL:
        default:
          Print(L"Identify/Inquiry errror: %r\n", Status);
          continue;
        }
      }

      hexdump((VOID*)&IdentifyData, BufferSize);

    }

    SafeFreePool((void**)&HandleBuffer);
//    SafeFreePool((void**)&IdentifyData);
    
    return EFI_SUCCESS;
}

INTN
EFIAPI
ShellAppMain (
    IN UINTN Argc,
    IN CHAR16 **Argv
)
{
    ZapNvme();
    return 0;
}
