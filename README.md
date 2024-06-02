# shellc
**Tool for converting scripts into C code**
### Install
```bash
cc shellc.c -O2 -o shellc
```
### Compile shell scripts and generate executable programs
```bash
shellc sh example.sh
cc example.sh.c -O2 -o example
```
The generated C code is ```example.sh.c```, which is the original file name followed by ```.c```.
For programs used in production environments, it is recommended to add the parameter ```-s``` or use the ```strip``` command after compilation to remove the symbol table in the executable program, increasing the difficulty of disassembly and reverse engineering.

I have tried the tools ```shc``` and ```shellcrypt```, which can compile shell scripts into executable programs.  The main problems are:
- The ```shc``` command actually calls the ```sh -c``` command.  The source code can be seen through the ```ps -ef``` command. The length of the shell script cannot exceed ARG_ MAX .
- ```Shellcrypt``` does not support external parameter input, and the encryption algorithm is relatively simple.  The generated executable program can be decrypted by XORing with ```0x000C``` to obtain the source code.

In addition to solving the problems of the above two tools, ```shellc```  also adds code obfuscation, randomly generating effective character position calculation functions, random character encryption, and debugging to increase the complexity of disassembly and reverse engineering.

Version 1.0 or above not only support shell, but also other scripting languages, which can completely replace  ```shc```.
At present, the program has practical applications in ```AIX```, ```UNIX```, ```Linux``` environments. It is recommended to conduct comprehensive testing on the compiled program to prevent production failures.
### User manual
- Generate C code command
 
   ``` shellc command inputfile [-t] [-s] [-f fix-argv0] ```  

    command：    Execute script commands, such ```sh```,```perl```,```python```,```node```,```ruby```,```Rscript```,```php```, etc.

    inputfile： Script file name

    -t option：    For systems that do not support anti debugging functionality, this option is required for generated code.

    -s option：    Generate C code mode, the - s option uses safe mode, and defaults to normal mode.

    -f option：    Fix parameter 0 value.

- Code pattern differences

    The normal mode shell script has some code visibility issues in most Linux operating systems, and the code after the ```read``` command can be obtained through ```/proc/[pid]/fd/[pipe]```. The safe mode does not have this issue.

    The normal mode supports semaphore processing in scripts, while the safe mode does not support it.

    The normal mode supports external parameter calls and human-machine interaction for all scripting languages, while the safe mode only supports shells that can define function syntax.

    The safe mode human-machine interaction ```read input``` needs to be changed to ```read input</dev/tty```, and the general mode does not require code modification.

    The safe mode and normal mode support different script language types, which can be checked by running ```test/run.sh```.

- Fix arguments 0
    
    Fix arguments 0  supports ```BASH```、```FISH```、```ZSH```、```PERL```、```PYTHON```、```JAVASCRIPT```、```LUA```、```RUBY```. 

    In addition to built-in fix types, custom file fix can also be used. Custom file format：```？```Indicates parameter 0 value, line breaks must be explicitly used```\n```, Double quotes require an escape character ```\```.Reference ```test/fix.txt```.

### Untraceable
Version 0.3 or above has added untraceable and tested the following tools for untraceable

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
openEuler|gdb|YES
Debian|gdb|YES
NetBSD 10.0|gdb|NO
DragonFly 6.4|gdb|NO
macOS 13|lldb|NO

### History

- v1.01 2024-06-02

  Remove special handling of FreeBSD

- v1.0 2024-06-01

  Add support for non SHELL scripting languages

  Add fix parameter 0

  Remove BSD and LINUX variant system thread requirements
  
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
 

