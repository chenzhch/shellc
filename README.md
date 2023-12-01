# shellc
### Tool for converting shell scripts into C code
## Install
```bash
cc -s shellc.c -O2 -o shellc
```
### Compile shell scripts and generate executable programs
```bash
shellc example.sh
cc -s example.sh.c -O2 -o example
```
The generated C code is ```example.sh.c```, which is the original file name followed by ```.c```.
For programs used in production environments, it is recommended to add the parameter ```-s``` when compiling to remove the symbol table in the executable program, increasing the difficulty of disassembly and reverse engineering.

I have tried the tools ```shc``` and ```shellcrypt```, which can compile shell scripts into executable programs.  The main problems are:
- The ```shc``` command actually calls the ```sh -c``` command.  The source code can be seen through the ```ps -ef``` command.
- ```Shellcrypt``` does not support external parameter input, and the encryption algorithm is relatively simple.  The generated executable program can be decrypted by XORing with ```0x000C``` to obtain the source code.

In addition to solving the problems of the above two tools, ```shellc``` also adds code obfuscation and randomly generates effective character position calculation functions to increase the complexity of disassembly and reverse engineering.If further difficulty is required, professional tools such as ```obfuscator-llvm``` can be used to further obfuscate the generated C code.

This program is suitable for shells that can define function syntax, such as ```ksh```, ```bsh```, ```zsh```, etc., and does not support ```csh``` (```csh``` does not support function definition).Currently, the program has been tested and has practical applications in ```AIX7.1```, ```SCO UNIX5.0.6```, ```Red Hat Linux7.8```, and ```CentOS Linux6.4``` environments.It is recommended to conduct comprehensive testing on the compiled program to prevent production failures.
#### If the shell script involves human-computer interaction, some modifications need to be made to the original shell script, and ```read input``` needs to be changed to ```read input </dev/tty```.
 

