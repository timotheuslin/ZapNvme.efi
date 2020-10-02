
#include <ZapNvme.h>

// Wipe out primary and secondary GPT.
EFI_STATUS
ZapGpts (
  IN EFI_BLOCK_IO_PROTOCOL *BlkIo
)
{
  VOID *Buffer;
  EFI_STATUS Status;
  UINT64 nGptSectorBytes0, nGptSectorBytes1;

  if (Dangerous_Zap_Dry_Run) {
    Print (L"    ***ZapGpts() returns with dry run !**\n");
    return 0;
  }

  nGptSectorBytes0 = BlkIo->Media->BlockSize * nGptSector0;
  nGptSectorBytes1 = BlkIo->Media->BlockSize * nGptSector1;
  Buffer = AllocateZeroPool (MAX(nGptSectorBytes0, nGptSectorBytes1));
  if (!Buffer) {
    return EFI_OUT_OF_RESOURCES;
  }

  for (;;) {
    if (Verbose_Level) {
      Print(L"  Wiping out MBR and the primary GPT: %d blocks\n", nGptSector0);
    }
    Status = BlkIo->WriteBlocks (
                      BlkIo,
                      BlkIo->Media->MediaId,
                      0,
                      nGptSectorBytes0,
                      Buffer
                      );
    if (EFI_ERROR (Status)) {
      break;
    }

    if (Verbose_Level) {
      Print(L"  Wiping the secondary GPT: %d blocks.\n", nGptSector1);
    }
    Status = BlkIo->WriteBlocks (
                      BlkIo,
                      BlkIo->Media->MediaId,
                      BlkIo->Media->LastBlock-nGptSector1,
                      nGptSectorBytes1,
                      Buffer
                      );
    break;  
  }
  SafeFreePool((VOID**)&Buffer);
  return Status;
}


//-EFI_STATUS NvmeZap (CHAR16 *USER_SN, UINTN Length)
//-{
//-  UINTN BlkIo_count;
//-  UINTN NVME_SN_MATCHED = 0;
//-  EFI_STATUS Status;
//-
//-  Print (L"NVME Storage (Block IO):\n");
//-  for (INT16 i=0;;i++) {
//-    EFI_HANDLE Handle=NULL;
//-    EFI_BLOCK_IO_PROTOCOL *BlkIo=NULL;
//-    CHAR16 *Description=NULL, *Sn=NULL, *Mn=NULL;
//-
//-    i = BlkIo_Iterator(i, &Handle, &BlkIo);
//-    if (i < 0) {
//-      break;
//-    }
//-
//-    Description = BmGetNvmeDescription(Handle);
//-    if (!Description) {
//-      continue;
//-    }
//-    Mn = Description;
//-    Sn = Description+NVME_PRODUCT_MODEL_NUMBER_SIZE+1;
//-    Mn[NVME_PRODUCT_MODEL_NUMBER_SIZE] = L'\0';
//-    Sn[NVME_PRODUCT_SERIAL_NUMBER_SIZE] = L'\0';
//-
//-    Mn = StrStrip(Mn);
//-    Sn = StrStrip(Sn);
//-
//-    if (StrinCmp(USER_SN, Sn, Length) == 0) {
//-      Print(L"  Matched Serial# : [%s] - [%s]\n", USER_SN, Sn);
//-      NVME_SN_MATCHED++;
//-      Status = ZapGpts(BlkIo);
//-      if (EFI_ERROR (Status)) {
//-        Print (L"Write Disk Failed");
//-        return EFI_LOAD_ERROR;
//-      }
//-      if (Verbose_Level) {
//-        Print (L"GPTs are wiped out!\n");
//-      }
//-    }
//-    SafeFreePool((void**)&Description);
//-    BlkIo_count++;
//-  }
//-
//-  if (!BlkIo_count) {
//-    Print(L"  Not found.\n");
//-  }
//-
//-  return 0;
//-}



EFI_STATUS NvmeZap (CHAR16 *USER_SN, UINTN Length)
{
  INT64 i=0;
  INT16 NVME_Count=0;
  INT16 NVME_SN_MATCHED=0;
  EFI_STATUS Status;

  Print(L"Zapping NVME GPT\n");
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
      Status = ZapGpts(BlkIo);
      if (EFI_ERROR (Status)) {
        Print (L"Write NVME storage failure: %r", Status);
        return EFI_LOAD_ERROR;
      }
      if (Verbose_Level) {
        Print (L"GPTs are wiped out!\n");
      }
    }
  }
  if (!NVME_SN_MATCHED) {
    Print(L"  No (partial) match for NVME storage with serial number: %s.\n", USER_SN);
  }
  return NVME_SN_MATCHED ? 0 : EFI_NOT_FOUND;
}