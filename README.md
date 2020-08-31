ZapNvme
===
Zap an NVME-SSD's GPT tables


## Prerequisites:
1. Python 3.6+,
2. git 2.17.0+
3. UDK/EDK2 code tree in following tags: edk2-stable{201911, 202005}


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
- (Optional) When using iPug:
    - `pip3 install ipug --user`


## Code tree setup and build:
1. git-clone edk2-stable202005
2. git-clone this package to the root folder of the edk2-stable202005 codetree.
3. Build this package as building a general EDK'2 package.

## Usage of ZapNvme
1. To clean the GPT tables<br>
    `ZapNvme zap Nvme_SSD_serial_number`<br>
    **BEWARE: The specified NVME SSD's GPT tables would be wiped out !**
2. To get the NVME info including the serial number<br>
    `ZapNvme info`

## Known limitations:
1. Only Linux build is tested. Windows and Xcode are not covered.


## (Optional) Build using iPug:
1. `git-clone https://github.com/timotheuslin/ZapNvme`
2. Change-directory to folder **ZapNvme**.
3. (Optional) Edit `CODETREE` in `project.py` to specify where to place the downloaded source files of the UDK git repo or any other additional repos.
4. To setup the EDK2 code base and build the BaseTools executables, run `ipug setup`.
5. To build the code, run `ipug build` (iPug will then handle all the rest of remaining tedious works with the UDK code tree setup and the build process.)
6. Browse to folder **Build/ZapNvme** for the build results.
7. Browse to folder **Build/Conf** for CONF_PATH setting files.
8. Run `ipug {clean, cleanall}` to clean (all) the intermediate files.
