ZapNvme
===
Wipe out an NVME-SSD's GPT tables


## Prerequisites:
1. Python 3.6+,
2. git 2.17.0+
3. UDK/EDK2 code tree in following tags: edk2-stable{201911, 202002}


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
2. git-clone this package to the root folder of the edk2-stable202002 codetree.
3. Build this package as building a general EDK'2 package.


## Usage of ZapNvme
1. To get the NVME info including the serial number<br>
    `ZapNvme info`
2. To clean the GPT tables<br>
    `ZapNvme zap Nvme_SSD_serial_number`<br>
    **WARNING: The specified NVME SSD's GPT tables would be wiped out !**


## Known limitations:
1. Only Linux build is tested. Windows and Xcode are not covered.


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
