
#include <ZapNvme.h>

//UINT16 NvmeInfo0(VOID)
//{
//  UINT16 BlkIo_count=0;
//
//  Print(L"NVME Storage:\n");
//  for (INT16 i=0;;i++) {
//    EFI_HANDLE Handle;
//    EFI_BLOCK_IO_PROTOCOL *BlkIo;
//    CHAR16 *Description, *Sn, *Mn;
//
//    i = BlkIo_Iterator(i, &Handle, &BlkIo);
//    if (i < 0) {
//      break;
//    }
//
//    Description = BmGetNvmeDescription(Handle);
//    if (!Description) {
//      continue;
//    }
//    Mn = Description;
//    Sn = Description+NVME_PRODUCT_MODEL_NUMBER_SIZE+1;
//    Mn[NVME_PRODUCT_MODEL_NUMBER_SIZE] = L'\0';
//    Sn[NVME_PRODUCT_SERIAL_NUMBER_SIZE] = L'\0';
//
//    Mn = StrStrip(Mn);
//    Sn = StrStrip(Sn);
//
//    Print(L"  Model#  : [%s]\n", Mn);
//    Print(L"  Serial# : [%s]\n", Sn);
//    Print(L"  %d blocks (%ld bytes).\n", BlkIo->Media->LastBlock, BlkIo->Media->LastBlock*BlkIo->Media->BlockSize);
//
//    SafeFreePool((void**)&Description);
//
//    BlkIo_count++;
//  }
//
//  if (!BlkIo_count) {
//    Print(L"  Not found.\n");
//  }
//
//  return BlkIo_count;
//}

INT16 NvmeInfo(VOID)
{
  INT16 NVME_Count=0, i=0;

  Print(L"NVME Storage\n");
  for (i=0; ; i++) {
    EFI_BLOCK_IO_PROTOCOL *BlkIo;
    CHAR16 Desc[NVME_DESCRIPTION_SIZE+1], *Mn, *Sn;

    if ((i = NVME_Iterator(i, &BlkIo, Desc, NVME_DESCRIPTION_SIZE)) <0) {
      break;
    }
    NVME_Count++;
    Print(L"  #%d:\n", NVME_Count);
    Mn = DescToMnSn(Desc, &Sn);
    Print(L"  Model  : %s\n", Mn);
    Print(L"  Serial : %s\n", Sn);
    Print(L"  %d blocks = %ld bytes\n", BlkIo->Media->LastBlock+1, (BlkIo->Media->LastBlock+1)*BlkIo->Media->BlockSize);
  }
  if (!NVME_Count) {
    Print(L"  Not found.\n");
  }
  return NVME_Count;
}
