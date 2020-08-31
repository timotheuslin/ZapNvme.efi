/** @file

Clean the starting sectors of an NVME disk.
Ref. https://en.wikipedia.org/wiki/GUID_Partition_Table


NvmeGpt command [data]

command/data
  1. backup
  2. zap $Sn
  3. info
  4. restore

**/

#include <ZapNvme.h>

#include <Library/HandleParsingLib.h> //BUGBUG

CHAR16 NVME_SERIAL_NUMBER[NVME_PRODUCT_SERIAL_NUMBER_SIZE] = {0};
CHAR16 NVME_PRODUCT_MODEL_NUMBER[NVME_PRODUCT_MODEL_NUMBER_SIZE] = {0};
CHAR16 USER_NVME_SERIAL_NUMBER[USER_NVME_PRODUCT_SERIAL_NUMBER_SIZE] = {0};


UINT8 Verbose_Level = 1;
UINT8 Dangerous_Zap = 1;
UINT8 Dangerous_Zap_Dry_Run = 0;
UINT8 Verbose_Dump = 0;
UINT8 Save_GPTs = 1;


// Note: HandleBuffer must be freed by the caller.
EFI_STATUS 
BlockIoProtocolHandles(EFI_HANDLE **HandleBuffer, UINTN *HandleCount)
{
  EFI_STATUS Status;

  if (!HandleBuffer) {
    return EFI_NOT_FOUND;
  }

  Status = EFI_NOT_FOUND;
  Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiBlockIoProtocolGuid, NULL, HandleCount, HandleBuffer);
  if (EFI_ERROR(Status)) {
    Print(L"Error when locating BLOCK IO: %r\n", Status);
    return Status;
  }
  else {
    if (Verbose_Level) {
      Print(L"Instances of the installed BLOCK IO Protocol: %d\n", HandleCount);
    }
  }
  return Status;
}


EFI_STATUS
ZapNvme(CHAR16 *USER_SN)
{
  UINTN HandleCount;
  EFI_STATUS Status;
  EFI_HANDLE *HandleBuffer;
  UINTN nblocks = 0x1000l;
  VOID *Buffer;
  UINT8 NVME_FOUND = 0;
  UINT8 NVME_SN_MATCHED = 0;

  Status = BlockIoProtocolHandles(&HandleBuffer, &HandleCount);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  for (INTN i=0;;i++) {
    EFI_HANDLE Handle;
    EFI_BLOCK_IO_PROTOCOL *BlkIo;
    CHAR16 *Description, *Sn, *Mn;

    i = BlkIo_Iterator(i, &Handle, &BlkIo);
    if (i < 0) {
      break;
    }
    Description = BmGetNvmeDescription(Handle);
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

  }
  return 0;

  for (INTN i = 0; i < HandleCount; i++) {
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

VOID Usage(VOID)
{
  Print (L"Usage: \n"
         L"   NvmeGpt [arg1 [arg2 ...]] \n"
         L"   arg: \n"
         L"     ZAP Serial_Number\n"
         L"     STORE\n"
         L"     RESTORE\n"
         L"     INFO\n"
         L"     HELP\n"
         );

}

#define NVME_ZAP_SHORTHAND_LENGTH 6

INTN
EFIAPI
ShellAppMain (
    IN UINTN argc,
    IN CHAR16 **argv
)
{
  UINTN i;

  if (Verbose_Level > 1) {
    for (i=0; i<argc; i++) {
      Print (L"argv[%d]: %s\n", i, argv[i]);
    }
  }
  //ShellSetPageBreakMode(TRUE);
  if (argc > 1) {
    if (StriCmp(argv[1], L"ZAP") == 0) {
      if (argc > 2) {
        Dangerous_Zap = 1;
        return NvmeZap (argv[2], MAX(StrLen(argv[2]), NVME_ZAP_SHORTHAND_LENGTH));
      }
      else {
        Print (L"%s needs the NVME's serial number assiged.\n", argv[1]);
        Usage();
        return 1;
      }
    }
    else if (StriCmp(argv[1], L"BACKUP") == 0 || StriCmp(argv[1], L"STORE") == 0) {
      if (argc > 2) {
        return NvmeGptSave (argv[2], MAX(StrLen(argv[2]), NVME_ZAP_SHORTHAND_LENGTH));
      }
      else {
        Print (L"%s needs the NVME's serial number assiged.\n", argv[1]);
        Usage();
        return 1;
      }
    }
    else if (StriCmp(argv[1], L"INFO") == 0) {
      NvmeInfo();
    }
    else if (StriCmp(argv[1], L"RESTORE") == 0) {
      //NvmeRestore();
    }
    else if (StriCmp(argv[1], L"DUMP") == 0) {
      //NvmeDumpHex();
      //Nvme    DumpHex();
      if (argc > 2) {
        ShellSetPageBreakMode(TRUE);
        //BUGBUG: page-break causes system hang when 'Q' is pressed.
        return NvmeDump (argv[2], MAX(StrLen(argv[2]), NVME_ZAP_SHORTHAND_LENGTH), 0, 0);
      }
      else {
        Print (L"%s needs the NVME's serial number assiged.\n", argv[1]);
        Usage();
        return 1;
      }
    }

//    else if (StriCmp(argv[1], L"DEBUG") == 0) {
//      static UINTN HandleCount=0;
//      static EFI_HANDLE *HandleBuffer = NULL;
//      EFI_STATUS Status;
//      INT16 i;
//
//      ShellSetPageBreakMode(TRUE);
//      Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiBlockIoProtocolGuid, NULL, &HandleCount, &HandleBuffer);
//      Print(L"Block IO, Instance Count: %d\n", HandleCount);
//      for (i=0; i<HandleCount; i++) {
//        CHAR16 *s;
//        Print(L"i: %d\n", i);
//        s = GetProtocolInformationDump(HandleBuffer[i], &gEfiBlockIoProtocolGuid, TRUE);
//        Print (L"Handle %p: %s\n", HandleBuffer[i], s);
//        SafeFreePool((void**)&s);
//      }
//      SafeFreePool((void**)&HandleBuffer);
//    }
    else {
      Usage();
      return 0;
    }
  }
  return 0;
}
