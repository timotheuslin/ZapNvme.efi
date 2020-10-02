ZapNvme.efi
===
A UEFI shell utility that zaps an NVME SSD's GUID Partition Tables (GPT)<br>
**WARNING: when an SSD's GPT is wiped out, it would become non-bootable if it was an OS disk.**


## Prerequisites:
1. Python 3.6+,
2. git 2.17.0+
3. [UDK/EDK2 code tree](https://github.com/tianocore/edk2) in following tags: edk2-stable{201911, 202002}


## Generic prerequisites for the UDK porting:
1. nasm (2.0 or above)
2. iasl (version 2018xxxx or later, maybe optional)
3. MSVC(Windows) or Xcode(Mac) or GCC(Open-source Posix)
4. build-essential uuid-dev (Posix)
5. motc (Xcode)
6. Reference:
    - [Getting Started with EDK II](https://github.com/tianocore/tianocore.github.io/wiki/Getting%20Started%20with%20EDK%20II) 
    - [Xcode](https://github.com/tianocore/tianocore.github.io/wiki/Xcode)


## Tools installation for any Debian-Based Linux:
- `sudo apt update; sudo apt install nasm iasl build-essential uuid-dev`


## Code tree setup and build:
1. git-clone edk2-stable202002
2. git-clone this package to the root folder of the above git-cloned EDK2 codetree.
3. Build this package as the way of building a general EDK2's package.


## Usage of ZapNvme
0. Boot into the EFI Shell.<br>
1. To get the NVME info including the serial number<br>
    `ZapNvme info`
2. To wipe out the GPT. The serial number can be the first 6 characters.<br>
    `ZapNvme zap Nvme_SSD_serial_number`<br>
    **WARNING: The specified NVME SSD's GUID Partition Tables would be wiped out !**


## (Optional) Build using iPug:
1. `pip3 install ipug --user`
2. `git-clone https://github.com/timotheuslin/ZapNvme.efi`
3. Change-directory to folder **ZapNvme.efi**.
4. (Optional) Edit `CODETREE` in `project.py` to specify where to place the downloaded source files of the UDK git repo or any other additional repos.
5. To setup the EDK2 code base and build the BaseTools executables, run `ipug setup`.
6. To build the code, run `ipug build` (iPug will then handle all the rest of remaining tedious works with the UDK code tree setup and the build process.)
7. Browse to folder **Build/ZapNvme** for the build results.
8. Browse to folder **Build/Conf** for CONF_PATH setting files.
9. Run `ipug {clean, cleanall}` to clean (all) the intermediate files.


## Known limitations:
1. Only Linux build is tested. Windows and Xcode are not covered.
