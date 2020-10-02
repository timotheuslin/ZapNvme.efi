/** @file

Clean the starting sectors of an NVME disk.
Ref. https://en.wikipedia.org/wiki/GUID_Partition_Table


NvmeGpt command [data]

command/data
  1. zap $Sn
  2. info
  3. TODO: backup
  4. TODO: restore


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


VOID Usage(VOID)
{
  Print (L"Usage: \n"
         L"NvmeGpt [arg1 [arg2 ...]] \n"
         L"   arg: \n"  
         L"       ZAP Serial_Number - Wipe out the NVME SSD's GPT\n"
//TODO         L"       STORE\  n"
//TODO         L"       RESTORE\n"
         L"       INFO - The basic infomation from every detected NVME SSD\n"
         L"       HELP - This help message\n"
         L"\n"
         L"Open Source:\n"
         L"   https://github.com/timotheuslin/ZapNvme.efi\n"
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
//TODO    else if (StriCmp(argv[1], L"DEBUG") == 0) {
//TODO      static UINTN HandleCount=0;
//TODO      static EFI_HANDLE *HandleBuffer = NULL;
//TODO      EFI_STATUS Status;
//TODO      INT16 i;
//TODO
//TODO      ShellSetPageBreakMode(TRUE);
//TODO      Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiBlockIoProtocolGuid, NULL, &HandleCount, &HandleBuffer);
//TODO      Print(L"Block IO, Instance Count: %d\n", HandleCount);
//TODO      for (i=0; i<HandleCount; i++) {
//TODO        CHAR16 *s;
//TODO        Print(L"i: %d\n", i);
//TODO        s = GetProtocolInformationDump(HandleBuffer[i], &gEfiBlockIoProtocolGuid, TRUE);
//TODO        Print (L"Handle %p: %s\n", HandleBuffer[i], s);
//TODO        SafeFreePool((void**)&s);
//TODO      }
//TODO      SafeFreePool((void**)&HandleBuffer);
//TODO    }
    else {
      Usage();
      return 0;
    }
  }
  else {
    Usage();
    return 0;
  }
  return 0;
}
