# shellc
**Tool for converting shell scripts into c code**
### Install
```bash
cc -s shellc.c -O2 -o shellc
```
### Compile shell scripts and generate executable programs
```bash
shellc example.sh
cc -s example.sh.c -O2 -o example -lpthread
```
The generated C code is ```example.sh.c```, which is the original file name followed by ```.c```.
For programs used in production environments, it is recommended to add the parameter ```-s``` when compiling to remove the symbol table in the executable program, increasing the difficulty of disassembly and reverse engineering.

I have tried the tools ```shc``` and ```shellcrypt```, which can compile shell scripts into executable programs.  The main problems are:
- The ```shc``` command actually calls the ```sh -c``` command.  The source code can be seen through the ```ps -ef``` command. The length of the shell script cannot exceed ARG_ MAX .
- ```Shellcrypt``` does not support external parameter input, and the encryption algorithm is relatively simple.  The generated executable program can be decrypted by XORing with ```0x000C``` to obtain the source code.

In addition to solving the problems of the above two tools, ```shellc```  also adds code obfuscation, randomly generating effective character position calculation functions, random character encryption, and debugging to increase the complexity of disassembly and reverse engineering.

This program is suitable for shells that can define function syntax, such as ```ksh```, ```bsh```, ```zsh```, etc., and does not support ```csh``` (```csh``` does not support function definition).Currently, the program has been tested and has practical applications in ```AIX```, ```UNIX```, and ```Linux``` environments.It is recommended to conduct comprehensive testing on the compiled program to prevent production failures.

**If the shell script involves human-computer interaction, some modifications need to be made to the original shell script, and ```read input``` needs to be changed to ```read input </dev/tty```.**

Version 3 has added untraceable and tested the following tools for untraceable

OS| TOOL|SUPPORTED
------|------|------
AIX 7.1|dbx sdb|YES
SCO UNIX 5.0.6|dbx sdb adb|YES
Red Hat Linux7.8|gdb|YES
CentOS Linux6.4|gdb|YES
debian 12.4.0|gdb|YES
Fedora 39|gdb lldb|YES
FreeBSD 14.0|gdb|YES
openSUSE Leap 15.5|gdb|YES
OracleLinux R9|gdb|YES
ubuntu 22.04.3|gdb lldb|YES
NetBSD 10.0|gdb|NO
DragonFly 6.4|gdb|NO
macOS 13|lldb|NO

**For systems that do not support anti debugging functionality, the generated code needs to use the - t option. BSD and LINUX variant systems require thread support to enable anti debugging functionality**

### History
- v0.3 2023-12-25

  Add untraceable

  Fix compilation warnings in some systems

- v0.21 2023-12-13

  Fix program bugs in the ```zsh``` environment

- v0.2  2023-12-12

  Changing the data structure from a structured structure to a string array reduces compilation time for large files

  Add code character encryption function

- v0.1  2023-12-01

  Initial version 
 

