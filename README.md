# skynet v0.01

## Skynet is a c/asm based operating system. The primary goal is windows compatibility. The first versions will be strictly 32-bit command line with no GUI. All build processes will build under window 10
---
## Tools
1. Window 10 Home (I am using v21H2 x64)
2. NASM v2.15.05
	1. for 32 bit windows https://www.nasm.us/pub/nasm/releasebuilds/2.15.05/win32/nasm-2.15.05-installer-x86.exe
	2. for 64 bit windows https://www.nasm.us/pub/nasm/releasebuilds/2.15.05/win64/nasm-2.15.05-installer-x64.exe
3. 32/64 bit MinGW 8.1.0 compiler for windows (https://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win32/Personal%20Builds/mingw-builds/installer/mingw-w64-install.exe)
4. Custom written drive image file maker (in progress)

## Main File System
1) FAT 12/16/32

## Initial build mechanism
1) windows scripts

## Objectives
1. &#9745; decide on tooling
2. &#9745; decide on build mechanism
3. &#9745; write drive image file maker
4. &#9745; decide on main file system
4. &#9744; write boot sector and master boot sector for main file system
5. &#9744; incorporate boot sector and image build into a bootable imga by QEmu


