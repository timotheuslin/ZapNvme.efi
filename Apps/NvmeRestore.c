#include <ZapNvme.h>

EFI_STATUS
NvmeRestore(CHAR16 *USER_SN)
{
  int i;
  UINTN HandleCount;
  EFI_STATUS Status;
  EFI_HANDLE *HandleBuffer;
  UINTN nblocks = 0x1000l;
  VOID *Buffer;
  UINT8 NVME_FOUND = 0;
  UINT8 NVME_SN_MATCHED = 0;

  Status = EFI_NOT_FOUND;
  Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiBlockIoProtocolGuid, NULL, &HandleCount, &HandleBuffer);
  if (EFI_ERROR(Status)) {
    Print(L"Error when locating BLOCK IO: %r\n", Status);
    return Status;
  }
  else {
    if (Verbose_Level) {
      Print(L"Instances of the installed BLOCK IO Protocol: %d\n", HandleCount);
    }
  }
  for (i = 0; i < HandleCount; i++) {
    EFI_BLOCK_IO_PROTOCOL *BlkIo;
    CHAR16 *Description, *Sn, *Mn;
    Status = gBS->HandleProtocol (
                    HandleBuffer[i],
                    &gEfiBlockIoProtocolGuid,
                    (VOID **) &BlkIo
                    );
    if (EFI_ERROR (Status)) {
      continue;
    }
    // Skip the logical partitions and the removable media
    if (BlkIo->Media->LogicalPartition || BlkIo->Media->RemovableMedia) {
      continue;
    }
    Description = BmGetNvmeDescription(HandleBuffer[i]);
    if (!Description) {
      continue;
    }
    Mn = Description;
    Sn = Description+NVME_PRODUCT_MODEL_NUMBER_SIZE+1;
    Mn[NVME_PRODUCT_MODEL_NUMBER_SIZE] = L'\0';
    Sn[NVME_PRODUCT_SERIAL_NUMBER_SIZE] = L'\0';
    Mn = StrStrip(Mn);
    Sn = StrStrip(Sn);
    NVME_FOUND ++;
    Print(L"NVME Block IO\n");
    Print(L"  Product model number:  [%s]\n", Mn);
    Print(L"  Product serial number: [%s]\n", Sn);
    Print(L"  Total blocks: %d (%ld bytes).\n", BlkIo->Media->LastBlock, BlkIo->Media->LastBlock*BlkIo->Media->BlockSize);

    if (Verbose_Dump) {
      EFI_LBA lba = 0;
      UINTN nbytes = BlkIo->Media->BlockSize * nblocks;
      Buffer = AllocateZeroPool (nbytes);
      Status = BlkIo->ReadBlocks (
                        BlkIo,
                        BlkIo->Media->MediaId,
                        lba,
                        nbytes,
                        Buffer
                        );
      if (EFI_ERROR (Status)) {
        SafeFreePool ((VOID**)&Buffer);
        Print (L"Read Disk Failed");
        return Status;
      }
      hexdump(Buffer, MIN(nbytes, BlkIo->Media->BlockSize * 4));
      SafeFreePool ((VOID**)&Buffer);
    }
    if (Save_GPTs) {
      Status = SaveGpts(BlkIo, Sn);
      Print (L"SaveGpts() returns: %r\n", Status);
    }
    if (Dangerous_Zap) {
      if (StriCmp(USER_SN, Sn) == 0) {
        NVME_SN_MATCHED++;
        Status = ZapGpts(BlkIo);
        if (EFI_ERROR (Status)) {
          Print (L"Write Disk Failed");
          return EFI_LOAD_ERROR;
        }
        Print (L"GPTs are wiped out!\n");
      }
    }
    SafeFreePool((void**)&Description);
  }

  SafeFreePool((void**)&HandleBuffer);

  if (Dangerous_Zap && !NVME_SN_MATCHED) {
    Print (L"Error: unable to find an NVME storage matching serial number: %s.\n", USER_SN);
    Status = EFI_NOT_FOUND;
  }
  else {
    Status = 0;
  }
  
  return Status;
}