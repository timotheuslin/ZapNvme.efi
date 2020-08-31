#include <ZapNvme.h>

// Boldly copied from edk2-stable202002 :: MdeModulePkg/Library/UefiBootManagerLib/BmBootDescription.c :: BmGetNvmeDescription()

/**
  Return the boot description for NVME boot device.

  @param Handle                Controller handle.

  @return  The description string.
**/
CHAR16 *
BmGetNvmeDescription (
  IN EFI_HANDLE                      Handle
  )
{
  EFI_STATUS                               Status;
  EFI_NVM_EXPRESS_PASS_THRU_PROTOCOL       *NvmePassthru;
  EFI_DEV_PATH_PTR                         DevicePath;
  EFI_NVM_EXPRESS_PASS_THRU_COMMAND_PACKET CommandPacket;
  EFI_NVM_EXPRESS_COMMAND                  Command;
  EFI_NVM_EXPRESS_COMPLETION               Completion;
  NVME_ADMIN_CONTROLLER_DATA               ControllerData;
  CHAR16                                   *Description;
  CHAR16                                   *Char;
  UINTN                                    Index;

  Status = gBS->HandleProtocol (Handle, &gEfiDevicePathProtocolGuid, (VOID **) &DevicePath.DevPath);
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  Status = gBS->LocateDevicePath (&gEfiNvmExpressPassThruProtocolGuid, &DevicePath.DevPath, &Handle);
  if (EFI_ERROR (Status) ||
      (DevicePathType (DevicePath.DevPath) != MESSAGING_DEVICE_PATH) ||
      (DevicePathSubType (DevicePath.DevPath) != MSG_NVME_NAMESPACE_DP)) {
    //
    // Do not return description when the Handle is not a child of NVME controller.
    //
    return NULL;
  }

  //
  // Send ADMIN_IDENTIFY command to NVME controller to get the model and serial number.
  //
  Status = gBS->HandleProtocol (Handle, &gEfiNvmExpressPassThruProtocolGuid, (VOID **) &NvmePassthru);
  ASSERT_EFI_ERROR (Status);

  ZeroMem (&CommandPacket, sizeof(EFI_NVM_EXPRESS_PASS_THRU_COMMAND_PACKET));
  ZeroMem (&Command, sizeof(EFI_NVM_EXPRESS_COMMAND));
  ZeroMem (&Completion, sizeof(EFI_NVM_EXPRESS_COMPLETION));

  Command.Cdw0.Opcode = NVME_ADMIN_IDENTIFY_CMD;
  //
  // According to Nvm Express 1.1 spec Figure 38, When not used, the field shall be cleared to 0h.
  // For the Identify command, the Namespace Identifier is only used for the Namespace data structure.
  //
  Command.Nsid        = 0;
  CommandPacket.NvmeCmd        = &Command;
  CommandPacket.NvmeCompletion = &Completion;
  CommandPacket.TransferBuffer = &ControllerData;
  CommandPacket.TransferLength = sizeof (ControllerData);
  CommandPacket.CommandTimeout = EFI_TIMER_PERIOD_SECONDS (5);
  CommandPacket.QueueType      = NVME_ADMIN_QUEUE;
  //
  // Set bit 0 (Cns bit) to 1 to identify a controller
  //
  Command.Cdw10                = 1;
  Command.Flags                = CDW10_VALID;

  Status = NvmePassthru->PassThru (
                               NvmePassthru,
                               0,
                               &CommandPacket,
                               NULL
                               );
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  Description = AllocateZeroPool (
                  (ARRAY_SIZE (ControllerData.Mn) + 1
                   + ARRAY_SIZE (ControllerData.Sn) + 1
                   + MAXIMUM_VALUE_CHARACTERS + 1
                   ) * sizeof (CHAR16));
  if (Description != NULL) {
    Char = Description;
    for (Index = 0; Index < ARRAY_SIZE (ControllerData.Mn); Index++) {
      *(Char++) = (CHAR16) ControllerData.Mn[Index];
    }
    *(Char++) = L' ';
    for (Index = 0; Index < ARRAY_SIZE (ControllerData.Sn); Index++) {
      *(Char++) = (CHAR16) ControllerData.Sn[Index];
    }
    *(Char++) = L' ';
    UnicodeValueToStringS (
      Char, sizeof (CHAR16) * (MAXIMUM_VALUE_CHARACTERS + 1),
      0, DevicePath.NvmeNamespace->NamespaceId, 0
      );
    //BUGBUG- BmEliminateExtraSpaces (Description);
  }

  return Description;
}