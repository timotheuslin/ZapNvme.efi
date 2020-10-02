
#include <ZapNvme.h>

//
// Save GPT tables to files.
//
EFI_STATUS
SaveGpts (
  IN EFI_BLOCK_IO_PROTOCOL *BlkIo,
  IN CHAR16 *Sn
)
{
  VOID *Buffer;
  EFI_STATUS Status = 0;
  UINT64 nGptSectorBytes0;
  UINT64 nGptSectorBytes1;
  CHAR16 PrimaryPartitionTableFn[1024];
  CHAR16 SecondryPartitionTableFn[1024];
  SHELL_FILE_HANDLE DestHandleP = NULL;
  SHELL_FILE_HANDLE DestHandleS = NULL;


  PrimaryPartitionTableFn[0] = SecondryPartitionTableFn[0] = L'\0';
  // where is sprintf() ?!
  StrCpyS(PrimaryPartitionTableFn, sizeof(PrimaryPartitionTableFn)/sizeof(CHAR16)- 1, L"GPT-1.");
  StrCatS(PrimaryPartitionTableFn, sizeof(PrimaryPartitionTableFn)/sizeof(CHAR16)- 1, Sn);
  StrCatS(PrimaryPartitionTableFn, sizeof(PrimaryPartitionTableFn)/sizeof(CHAR16)- 1, L".bin");

  StrCpyS(SecondryPartitionTableFn, sizeof(SecondryPartitionTableFn)/sizeof(CHAR16)- 1, L"GPT-2.");
  StrCatS(SecondryPartitionTableFn, sizeof(SecondryPartitionTableFn)/sizeof(CHAR16)- 1, Sn);
  StrCatS(SecondryPartitionTableFn, sizeof(SecondryPartitionTableFn)/sizeof(CHAR16)- 1, L".bin");

  nGptSectorBytes0 =  nGptSector0 * BlkIo->Media->BlockSize;
  nGptSectorBytes1 =  nGptSector1 * BlkIo->Media->BlockSize;
  Buffer = AllocateZeroPool (MAX(nGptSectorBytes0, nGptSectorBytes1));
  if (!Buffer) {
    return EFI_OUT_OF_RESOURCES;
  }
  for(;;) {
    // Read the MBR + the primary GPT entries + the primary GPT header
    Status = BlkIo->ReadBlocks (
                      BlkIo,
                      BlkIo->Media->MediaId,
                      0,
                      nGptSectorBytes0,
                      Buffer
                      );
    if (EFI_ERROR (Status)) {
      break;
    }
    if (!isBlockEmpty(Buffer, nGptSectorBytes0)) {
      Print(L"  Empty primary partition filled with byte: %2.2X\n", *(UINT8*)Buffer);
    }
    else if (AsciiStrnCmp(((CHAR8*) Buffer)+BlkIo->Media->BlockSize, "EFI PART", 8) != 0) {
        Print(L"  Invalid signature \"EFI PART\" in the primary GPT header.\n", *(UINT8*)Buffer);
    }
    // TODO: CRC
    else {
      Status = ShellOpenFileByName(PrimaryPartitionTableFn, &DestHandleP, EFI_FILE_MODE_READ|EFI_FILE_MODE_WRITE|EFI_FILE_MODE_CREATE, 0);
      if (EFI_ERROR (Status)) break;
      Status = ShellWriteFile(DestHandleP, &nGptSectorBytes0, Buffer);
      if (EFI_ERROR (Status)) break;
      Print(L"  Write MBR (LBA 0), the primary GPT header (LBA 1) and entries (LBA 2 ~ LBA 33):\n    %s, %ld bytes\n", PrimaryPartitionTableFn, nGptSectorBytes0);
    }
    // Read the secondary GPT entries and the secondary GPT header
    Status = BlkIo->ReadBlocks (
                      BlkIo,
                      BlkIo->Media->MediaId,
                      BlkIo->Media->LastBlock - (nGptSector1-1),
                      nGptSectorBytes1,
                      Buffer
                      );
    if (!isBlockEmpty(Buffer, nGptSectorBytes1)) {
      Print(L"  Empty secondary partition filled with byte: %2.2X\n", *(UINT8*)Buffer);
    }
    else if (AsciiStrnCmp(((CHAR8*) Buffer)+nGptSectorBytes1-BlkIo->Media->BlockSize, "EFI PART", 8) != 0) {
      Print(L"  Invalid signature \"EFI PART\" in the secondary GPT header.\n", *(UINT8*)Buffer);
    }
    // TODO: CRC
    else {
      Status = ShellOpenFileByName(SecondryPartitionTableFn, &DestHandleS, EFI_FILE_MODE_READ|EFI_FILE_MODE_WRITE|EFI_FILE_MODE_CREATE, 0);
      if (EFI_ERROR (Status)) break;
      Status = ShellWriteFile(DestHandleS, &nGptSectorBytes1, Buffer);
      if (EFI_ERROR (Status)) break;
      Print(L"  Write the secondary GPT entries (LBA -33 ~ LBA -2) and the header (LBA -1):\n    %s, %ld bytes\n", SecondryPartitionTableFn, nGptSectorBytes1);
    }
    break;  
  }
  
  SafeFreePool((VOID**)&Buffer);
  SafeShellCloseFile(&DestHandleP);
  SafeShellCloseFile(&DestHandleS);

  return Status;
}

EFI_STATUS NvmeGptSave (CHAR16 *USER_SN, UINTN Length)
{
  INT64 i=0;
  UINT16 NVME_Count=0;
  INT16 NVME_SN_MATCHED=0;
  EFI_STATUS Status;

  Print(L"Saving NVME GPT\n");
  for (i=0; ; i++) {
    EFI_BLOCK_IO_PROTOCOL *BlkIo;
    CHAR16 Desc[NVME_DESCRIPTION_SIZE+1], *Sn, Sn_Shorthand[NVME_PRODUCT_SERIAL_NUMBER_SIZE+1];

    if ((i = NVME_Iterator(i, &BlkIo, Desc, NVME_DESCRIPTION_SIZE)) <0) {
      break;
    }
    NVME_Count++;
    DescToMnSn(Desc, &Sn);
    StrnCpy(Sn_Shorthand, Sn, Length);
    Print(L"  Storage %2d - Serial Number : %s \n", NVME_Count, Sn);
    Print(L"    Serial Number Shorthand  : %s \n", Sn_Shorthand);

    if (StrinCmp(USER_SN, Sn_Shorthand, Length) == 0) {
      Print(L"    Matched Serial Number : %s as %s\n", USER_SN, Sn);
      NVME_SN_MATCHED++;
      Status = SaveGpts(BlkIo, Sn);
      if (EFI_ERROR (Status)) {
        Print (L"    Read NVME Storage failure: %r\n", Status);
        return EFI_LOAD_ERROR;
      }
    }
  }
  if (!NVME_SN_MATCHED) {
    Print(L"  No (partial) match for NVME storage with serial number: %s.\n", USER_SN);
  }
  return NVME_SN_MATCHED ? 0 : EFI_NOT_FOUND;
}
