
#include <ZapNvme.h>

INT64 NvmeInfo(VOID)
{
  INT64 NVME_Count=0, i=0;

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
