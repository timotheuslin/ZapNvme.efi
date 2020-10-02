
#include <ZapNvme.h>


EFI_STATUS NvmeDump (CHAR16 *USER_SN, UINTN Length, EFI_LBA Start_Lba, UINT64 Lba_Count)
{
  INT64 NVME_Count=0, i=0;

  Print(L"NVME Storage\n");
  for (i=0; ; i++) {
    UINT64 j;
    VOID *Buffer;
    EFI_BLOCK_IO_PROTOCOL *BlkIo;
    CHAR16 Desc[NVME_DESCRIPTION_SIZE+1], *Sn;

    if ((i = NVME_Iterator(i, &BlkIo, Desc, NVME_DESCRIPTION_SIZE)) <0) {
      break;
    }

    NVME_Count++;
    DescToMnSn(Desc, &Sn);
    Print(L"  Storage %2d - Serial Number : %s \n", NVME_Count, Sn);

    Buffer = AllocateZeroPool (BlkIo->Media->BlockSize);
    if (!Buffer) {
      return EFI_OUT_OF_RESOURCES;
    }
    if (Start_Lba > BlkIo->Media->LastBlock) {
        Print(L"    Error : Start LBA %ld > Last LBA \n", Start_Lba, BlkIo->Media->LastBlock);
        continue;
    }
    if (Lba_Count == 0) {
        Lba_Count = BlkIo->Media->LastBlock-Start_Lba+1;
    }
    for (j=0; j<Lba_Count && (Start_Lba+j)<=BlkIo->Media->LastBlock; j++) {
        EFI_STATUS Status;

        Print(L"  LBA %ld\n", Start_Lba+j);
        Status = BlkIo->ReadBlocks (
                          BlkIo,
                          BlkIo->Media->MediaId,
                          j+Start_Lba,
                          BlkIo->Media->BlockSize,
                          Buffer
                          );
        if (EFI_ERROR (Status)) {
          Print(L"    ReadBlocks error %r at LBA %ld\n", Status, j+Start_Lba);
          break;
        }
        //DumpHex(2, (j+Start_Lba)*BlkIo->Media->BlockSize, BlkIo->Media->BlockSize, Buffer);
        hexdump(Buffer, BlkIo->Media->BlockSize);
    }
    SafeFreePool((VOID**)&Buffer);

  }
  if (!NVME_Count) {
    Print(L"  Not found.\n");
  }
  return NVME_Count;
}


//DumpHex (
//  IN UINTN        Indent,
//  IN UINTN        Offset,
//  IN UINTN        DataSize,
//  IN VOID         *UserData
//  );