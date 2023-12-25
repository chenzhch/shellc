/*
 * This code is released under
 * GNU GENERAL PUBLIC LICENSE Version 3
 * Function: Convert shell scripts into C code
 * Author: ChenZhongChao
 * E-Mail: nbczc@163.com
 * Date: 2023-12-25
 * Version: 0.3
 * Github: https://github.com/chenzhch/shellc.git
 * Gitee: https://gitee.com/chenzhch/shellc.git
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/utsname.h>
#include <sys/stat.h>

#define F1(a, b, c, d)  ((~c|~a)>(~d|~b)?b-c:a-d)
#define F2(a, b, c, d)  ((~a|~c)>(~b|~d)?a<<c:b<<d)
#define F3(a, b, c, d)  ((a|b)>(c|d)?a|c:b<<d)
#define F4(a, b, c, d)  ((a|b)>(c|d)?a|b:c&d)
#define F5(a, b, c, d)  ((a&c)>(b&d)?a|c:b-d)
#define F6(a, b, c, d)  ((a&c)>(b&d)?a|c:b&d)
#define F7(a, b, c, d)  ((~c|~a)>(~b|~d)?a&c:b&d)
#define F8(a, b, c, d)  ((a|b)<(c|d)?a<<c:b&d)
#define F9(a, b, c, d)  ((a|b)<(c|d)?a|b:c<<d)
#define F10(a, b, c, d) ((~a|~c)<(~b|~d)?a+b:c-d)
#define F11(a, b, c, d) ((a&c)<(b&d)?a&c:b&d)
#define F12(a, b, c, d) ((a|~c)>(b|~d)?b<<c:a&d)
#define F13(a, b, c, d) ((c|~a)>(b|~d)?c&a:b<<d)
#define F14(a, b, c, d) ((b&c)<(b&d)?a&c:b|d)
#define F15(a, b, c, d) ((a&c)<(b&d)?a|c:b&d)
#define F16(a, b, c, d) ((a&c)>(b&d)?a&c:b|d)
#define F17(a, b, c, d) ((c|b)>(d|b)?a|c:b&d)
#define F18(a, b, c, d) ((a|~c)<(b|~d)?a<<c:b&d)
#define F19(a, b, c, d) ((b|c)<(a|d)?a<<c:b&d)
#define F20(a, b, c, d) ((b|c)<(a|d)?b&c:a<<d)
#define F21(a, b, c, d) ((a|c)<(a|d)?b|c:b&d)
#define F22(a, b, c, d) ((a|c)>(b|d)?a<<c:b|d)
#define F23(a, b, c, d) ((~a|~c)<(~b|~d)?a<<c:b+d)
#define F24(a, b, c, d) ((~a|~c)<(~b|~d)?a-c:b+d)
#define F25(a, b, c, d) ((b|c)>(a|d)?a|c:b<<d)
#define F26(a, b, c, d) ((a|~c)<(b|~d)?a&c:b<<d)
#define F27(a, b, c, d) ((~a|~c)<(b|~d)?a|c:b&d)
#define F28(a, b, c, d) ((~a|~c)>(b|~d)?a&c:b|d)
#define F29(a, b, c, d) ((a|b)<(c|d)?a&b:c&d)
#define F30(a, b, c, d) ((~b|~a)>(c|d)?a<<c:b|d)
#define F31(a, b, c, d) ((a|~b)>(c|d)?a&c:d&b)
#define F32(a, b, c, d) ((a|b)>(c|~d)?a|c:d|b)

typedef long(*Fn)(long, long);

long ff(Fn fn, long a, long b)
{
    return fn(a, b);
}

static const char *head[] = {
    "#include <stdio.h>",
    "#include <string.h>",
    "#include <stdlib.h>",
    "#include <time.h>",
    "#include <signal.h>",
    "#include <unistd.h>",
    "#include <sys/types.h>",
    "#include <sys/wait.h>",
    0
};

static const char *tail[] = {
    "typedef long(*Fn)(long, long);",
    "",
    "long ff(Fn fn, long a, long b)",
    "{",
    "    return fn(a, b);",
    "}",
    "",
    "typedef struct {",
    "    int argc;",
    "    char **argv;",
    "} Param;",
    "",
    "int handler(int argc, char **argv)",
    "{",
    "    int i, j, k;",
    "    FILE *pipe;",
    "    long mask;",
    "    char chr[3], *str;",
    "    Fn fn[] = {",
    "        f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16,",
    "        f17, f18, f19, f20, f21, f22, f23, f24, f25, f26, f27, f28, f29, f30, f31, f32,",
    "        f33, f34, f35, f36, f37, f38, f39, f40, f41, f42, f43, f44, f45, f46, f47, f48,",
    "        f49, f50, f51, f52, f53, f54, f55, f56, f57, f58, f59, f60, f61, f62, f63, f64",
    "    };",
    "    if(getenv(\"SHELL\") == NULL) {",
    "        printf(\"Failed to get shell environment variable\\n\");",
    "        return(1);",
    "    }",
    "    pipe = popen(getenv(\"SHELL\"), \"w\");",
    "    if(pipe == NULL) {",
    "        printf(\"Failed to popen shell\\n\");",
    "        return(1);",
    "    }",
    "    srand(time(0));",
    "    i = 0;",
    "    memset(chr, 0, (size_t) sizeof(chr));",
    "    while(data[i]) {",
    "        mask = (long) 1 << (atol(data[i + 1]) - 1);",
    "        k = rand() % 8 + 2;",
    "        str = (char *) malloc(k);",
    "        memset(str, 0, (size_t) k);",
    "        for(j = 0; j < atol(data[i + 1]); j++) {",        
    "           memcpy(chr, &data[i + 5][j * 2], 2);",            
    "           sprintf(",
    "               str + strlen(str), \"%c\",",
    "               (atol(data[i + 2]) + ff(fn[atol(data[i]) -1], atol(data[i + 3]), atol(data[i + 4]))) & mask",
    "               ? (int) strtol(chr, 0, 16)",
    "               ^ (int) (atol(data[i + 2]) + ff(fn[atol(data[i]) -1], atol(data[i + 3]), atol(data[i + 4]))) % 256",
    "               ^ (int) ff(fn[j % 32 + 32], atol(data[i + 3]), atol(data[i + 4])) % 256",
    "               : 0",
    "           );",  
    "           if(strlen(str) == k - 1) {",
    "               fputs(str, pipe);",
    "               memset(str, 0, (size_t) k);",
    "           }",            
    "           mask >>= 1;",
    "       }",
    "       if(strlen(str) != 0) {",
    "           fputs(str, pipe);",
    "       }",
    "       free(str);",
    "       i += 6;",
    "    }",
    "    for(i = 1; i < argc; i++) {",
    "        fputs(\" \\\"\", pipe);",
    "        fputs(argv[i], pipe);",
    "        fputc('\"', pipe);",
    "    }",
    "    if(pclose(pipe)) {",
    "        return(1);",
    "    }",
    "    return(0);",
    "}",  
    "",
    "int process(void *arg)", 
    "{",
    "    Param *param = (Param *) arg;",
    "    return(handler(param->argc, param->argv));",
    "}",
    "",
    0
}; 

static const char *main_func[] = {
    "int main(int argc, char **argv)",
    "{",   
    0,
};

static const char *ptrace_sco[] = {
    "    if(ptrace(0, getppid(), 0, 0)) {",
    "        return(1);",
    "    } else {",
    "        kill(getppid(), SIGCONT);",
    "    }",
    "    return(handler(argc, argv));",
    "}",
    0
};

static const char *ptrace_aix[] = {
    "    pid_t pid = 0;",
    "    if(fork() == 0) {",
    "        pid = getpid();",
    "        if(__linux_ptrace(PT_WRITE_GPR, getppid(), 0, 0)) {",
    "            kill(getppid(), SIGKILL);",
    "        } else {",
    "            __linux_ptrace(PT_WRITE_FPR, getppid(), 0, 0);",
    "        }",
    "        return(0);",
    "    }",
    "    waitpid(pid, 0, 0);",
    "    return(handler(argc, argv));",
    "}",
    0
};

static const char *ptrace_linux[] = {
    "    Param param = {argc, argv};",
    "    pthread_t id = 0;",
    "    void *result;",
    "    if(ptrace(0, 0, 0, 0)) {",
    "        return(1);",        
    "    }",
    "    if(pthread_create(&id, 0, (void *) process, &param)) {",
    "        perror(\"Faild to pthread_create\");",
    "        return(1);",
    "    }",
    "    if(pthread_join(id, &result)) {",
    "        perror(\"Faild to pthread_join\");",
    "        return(1);",
    "    }",
    "    return((long) result);",
    "}",
    0 
};

static const char *traced[] = {
    "    return(handler(argc, argv));",
    "}",
    0        
};

/*Binary characters to decimal numbers*/
long binary_to_int(char *str, int len)
{
    int i;
    long val = 0;
    long mask = 1;
    for(i = len -1; i >= 0;i--) {
        val += (str[i] - '0') * mask;
        mask <<= 1;
    }
    return(val);
}

/*Generate macro definitions*/
void macro(int order, int x, int y, char *str)
{
    switch(order) {
        case 1:  sprintf(str, "((~%d|~a)>(~%d|~b)?b-%d:a-%d)", x, y, x, y); break;
        case 2:  sprintf(str, "((~a|~%d)>(~b|~%d)?a<<%d:b<<%d)", x, y, x, y); break;
        case 3:  sprintf(str, "((a|b)>(%d|%d)?a|%d:b<<%d)", x, y, x, y); break;
        case 4:  sprintf(str, "((a|b)>(%d|%d)?a|b:%d&%d)", x, y, x, y); break;
        case 5:  sprintf(str, "((a&%d)>(b&%d)?a|%d:b-%d)", x, y, x, y); break;
        case 6:  sprintf(str, "((a&%d)>(b&%d)?a|%d:b&%d)", x, y, x, y); break;
        case 7:  sprintf(str, "((~%d|~a)>(~b|~%d)?a&%d:b&%d)", x, y, x, y); break;
        case 8:  sprintf(str, "((a|b)<(%d|%d)?a<<%d:b&%d)", x, y, x, y); break;
        case 9:  sprintf(str, "((a|b)<(%d|%d)?a|b:%d<<%d)", x, y, x, y); break;
        case 10: sprintf(str, "((~a|~%d)<(~b|~%d)?a+b:%d-%d)", x, y, x, y); break;
        case 11: sprintf(str, "((a&%d)<(b&%d)?a&%d:b&%d)", x, y, x, y); break;
        case 12: sprintf(str, "((a|~%d)>(b|~%d)?b<<%d:a&%d)", x, y, x, y); break;
        case 13: sprintf(str, "((%d|~a)>(b|~%d)?%d&a:b<<%d)", x, y, x, y); break;
        case 14: sprintf(str, "((b&%d)<(b&%d)?a&%d:b|%d)", x, y, x, y); break;
        case 15: sprintf(str, "((a&%d)<(b&%d)?a|%d:b&%d)", x, y, x, y); break;
        case 16: sprintf(str, "((a&%d)>(b&%d)?a&%d:b|%d)", x, y, x, y); break;
        case 17: sprintf(str, "((%d|b)>(%d|b)?a|%d:b&%d)", x, y, x, y); break;
        case 18: sprintf(str, "((a|~%d)<(b|~%d)?a<<%d:b&%d)", x, y, x, y); break;
        case 19: sprintf(str, "((b|%d)<(a|%d)?a<<%d:b&%d)", x, y, x, y); break;
        case 20: sprintf(str, "((b|%d)<(a|%d)?b&%d:a<<%d)", x, y, x, y); break;
        case 21: sprintf(str, "((a|%d)<(a|%d)?b|%d:b&%d)", x, y, x, y); break;
        case 22: sprintf(str, "((a|%d)>(b|%d)?a<<%d:b|%d)", x, y, x, y); break;
        case 23: sprintf(str, "((~a|~%d)<(~b|~%d)?a<<%d:b+%d)", x, y, x, y); break;
        case 24: sprintf(str, "((~a|~%d)<(~b|~%d)?a-%d:b+%d)", x, y, x, y); break;
        case 25: sprintf(str, "((b|%d)>(a|%d)?a|%d:b<<%d)", x, y, x, y); break;
        case 26: sprintf(str, "((a|~%d)<(b|~%d)?a&%d:b<<%d)", x, y, x, y); break;
        case 27: sprintf(str, "((~a|~%d)<(b|~%d)?a|%d:b&%d)", x, y, x, y); break;
        case 28: sprintf(str, "((~a|~%d)>(b|~%d)?a&%d:b|%d)", x, y, x, y); break;
        case 29: sprintf(str, "((a|b)<(%d|%d)?a&b:%d&%d)", x, y, x, y); break;
        case 30: sprintf(str, "((~b|~a)>(%d|%d)?a<<%d:b|%d)", x, y, x, y); break;
        case 31: sprintf(str, "((a|~b)>(%d|%d)?a&%d:%d&b)", x, y, x, y); break;
        case 32: sprintf(str, "((a|b)>(%d|~%d)?a|%d:%d|b)", x, y, x, y); break;
    }
}

int main(int argc, char **argv)
{
    FILE *in, *out;
    int code_length, obscure_length, length, pos;
    char *code_text, *obscure_text, *text;
    char *bitmap, *inname, *outname;
    char str[64];
    long result, offset1, offset2;
    long salt1, salt2;
    int size, digit;
    int route[32][17], x[32][16], y[32][16];
    char *shellc = "____sHeLl___C__MaiN_fUNcTioN___";
    char *operators = "+-*/%&|^";
    char number[32][32][8];
    char algorithm[32][17];
    int i, j, k, loop, mode;
    int flag = 0;    
    struct utsname sysinfo;
    struct stat status;  
    
    if(argc != 2 && argc != 3) {
        printf("Usage: %s [-t] infile\n", argv[0]);
        return(0);
    }
    if(argc == 2 && !strcmp("-h", argv[1])) {
        printf("%s: Convert shell scripts into C code\n", argv[0]);
        printf("Usage: %s [-t] infile\n", argv[0]);
        printf("Option: \n");
        printf("    -t    Make traceable binary program\n");
        printf("    -h    Display help and return\n");
        return(0);
    }else if(argc == 2 && !strcmp("-t", argv[1])) {
        printf("Usage: %s [-t] infile\n", argv[0]);
        return(0);
    }else if(argc == 3) {
        if(strcmp("-t", argv[1]) && strcmp("-t", argv[2])) {
            printf("Usage: %s [-t] infile\n", argv[0]);
            return(0);    
        }
        if(!strcmp("-t", argv[1])) {
            inname = strdup(argv[2]); 
        }else {
            inname = strdup(argv[1]);    
        }
        flag = 1;                
    }else {
        inname =  strdup(argv[1]);
    }
    
    in = fopen(inname, "r");
    if (in == NULL) {
        printf("Failed to open the file: %s\n", inname);
        return(1);
    }
    stat(inname, &status);
    code_length = status.st_size + strlen(shellc) * 2 + 32;
    code_text = (char *) malloc(code_length);
    memset(code_text, 0, (size_t) code_length);
    sprintf(code_text, "%s(){ ", shellc);
    i = strlen(code_text);
    for(j=0; j < status.st_size; j++) {
        code_text[i++] = fgetc(in);
    }
    sprintf(code_text + strlen(code_text), "%c}%c%s", '\n', '\n', shellc);
    srand(time(0));
    obscure_length = code_length / 5 * 6  + rand() % 1024 + 4096; 
    bitmap = (char *) malloc(obscure_length * 2);
    obscure_text = (char *) malloc(obscure_length * 2);
    
    /*Randomly generated visible obfuscated characters*/
    loop = 1;
    while(loop) {
        j = 0;
        for(i = 0; i < code_length; j++) {
            digit = rand() % obscure_length;
            if(digit < code_length) {
                bitmap[j] = '1';
                obscure_text[j] = code_text[i++];
            } else {
                bitmap[j] = '0';
                digit = rand() % 3;
                obscure_text[j] = digit == 0 ? code_text[i] : rand() % 95 + 32;
            }
            if(j == obscure_length * 2) break;
        }
        if(i == code_length){
            loop = 0;
        }
    }
    /*Total length after confusion*/
    obscure_length = j;
 
    /*Effective location calculation factors*/
    for(i= 0; i < 32; i++) {
        for(j = 0; j < 16; j++){
            x[i][j] = rand() % 10+1;
            y[i][j] = rand() % 10+1;
        }
    }

    /*Effective location calculation path*/
    for(i = 0; i < 32; i++) {
        k = rand() % 9 + 8;
        for(j = 0; j < k; j++) {
            route[i][j] = rand() % 32 + 1;
        }
        route[i][j]=0;
    }
    
    /*Character mask calculation path*/
    for(i= 0; i < 32; i++) {
        k = rand() % 9 + 8; 
        for(j = 0; j < k; j++){
            algorithm[i][j] = operators[rand() % 8];
            sprintf(number[i][j * 2], "%c", 'a' + rand() % 2);
            sprintf(number[i][j * 2 + 1], "%d", rand() % 1000 + 10);
        }
        algorithm[i][j] = 0;
    }
    
    outname = (char *) malloc(strlen(inname) + 3);
    sprintf(outname, "%s.c", inname);
    out = fopen(outname, "w");
    if (out == NULL) {
        printf("Failed to write to file %s\n", outname);
        return(1);
    }
    
    /*Write to the file header*/
    i = 0;
    while(head[i]) fprintf(out, "%s\n", head[i++]);
        
    if(!flag) {
        if (uname(&sysinfo)) {
            perror("Failed to uname");
            return(1);
        }  
        if(strcmp(sysinfo.sysname, "SCO_SV")) {
            fprintf(out, "#include <pthread.h>\n");
            fprintf(out, "#include <sys/ptrace.h>\n");
        }    
    }

    /*Macro definitions are generated based on the calculation path*/
    for(i = 0; i < 32; i++) {
        j = 0;
        while(route[i][j] != 0) {
            if(j == 0){
                fprintf(out, "#define F%d(a, b) (\\\n", i + 1);
            }
            memset(str, 0, (size_t) sizeof(str));
            macro(route[i][j], x[i][j], y[i][j], str);
            fprintf(out, "   %s%s\\\n", j == 0 ? " ":" +", str);
            j++;
        }
        fprintf(out, ")\n\n");
    }
    
    /*Character mask calculation function*/
    for(i = 0; i < 32; i++) {
        j = 0;
        while(algorithm[i][j] != 0) {
            if(j == 0){
                fprintf(out, "#define F%d(a, b) labs(\\\n", 32 + i + 1);
            }
            fprintf(out, "   %s(%s%c%s)\\\n", j == 0 ? " ":" +", number[i][j * 2], algorithm[i][j], number[i][j * 2 + 1]);
            j++;
        }
        fprintf(out, ")\n\n");
    }

    /*Generate a call function*/
    for(i = 0; i < 64; i++) {
        fprintf(out, "long f%d(long a, long b)\n", i + 1);
        fprintf(out, "{\n");
        fprintf(out, "    return(F%d(a, b));\n", i + 1);
        fprintf(out, "}\n\n");
    }        
    
    /*Write to the data section*/        
    fprintf(out, "static const char *data[] = {\n");
   
    size = sizeof(long) * 8;
    text =(char *) malloc(size);
    j = 0;
    while(obscure_length) {
        mode = rand() % 32;
        length = rand() % 4 + size - 5;

        if(obscure_length < length) {
            length = obscure_length;
        }
        obscure_length -= length;
        memset(text, 0, (size_t) size);
        result = binary_to_int(bitmap + j, length);
        salt1 = result % 256;
        memcpy(text, obscure_text + j, length);
        j += length;
        offset1 = rand() % 1000 + 10;
        offset2 = rand() % 1000 + 10;
        i = 0;
        while(route[mode][i] != 0) {
            switch(route[mode][i]) {
               case 1:  result -= F1(offset1,  offset2, x[mode][i], y[mode][i]); break;
               case 2:  result -= F2(offset1,  offset2, x[mode][i], y[mode][i]); break;
               case 3:  result -= F3(offset1,  offset2, x[mode][i], y[mode][i]); break;
               case 4:  result -= F4(offset1,  offset2, x[mode][i], y[mode][i]); break;
               case 5:  result -= F5(offset1,  offset2, x[mode][i], y[mode][i]); break;
               case 6:  result -= F6(offset1,  offset2, x[mode][i], y[mode][i]); break;
               case 7:  result -= F7(offset1,  offset2, x[mode][i], y[mode][i]); break;
               case 8:  result -= F8(offset1,  offset2, x[mode][i], y[mode][i]); break;
               case 9:  result -= F9(offset1,  offset2, x[mode][i], y[mode][i]); break;
               case 10: result -= F10(offset1, offset2, x[mode][i], y[mode][i]); break;
               case 11: result -= F11(offset1, offset2, x[mode][i], y[mode][i]); break;
               case 12: result -= F12(offset1, offset2, x[mode][i], y[mode][i]); break;
               case 13: result -= F13(offset1, offset2, x[mode][i], y[mode][i]); break;
               case 14: result -= F14(offset1, offset2, x[mode][i], y[mode][i]); break;
               case 15: result -= F15(offset1, offset2, x[mode][i], y[mode][i]); break;
               case 16: result -= F16(offset1, offset2, x[mode][i], y[mode][i]); break;
               case 17: result -= F17(offset1, offset2, x[mode][i], y[mode][i]); break;
               case 18: result -= F18(offset1, offset2, x[mode][i], y[mode][i]); break;
               case 19: result -= F19(offset1, offset2, x[mode][i], y[mode][i]); break;
               case 20: result -= F20(offset1, offset2, x[mode][i], y[mode][i]); break;
               case 21: result -= F21(offset1, offset2, x[mode][i], y[mode][i]); break;
               case 22: result -= F22(offset1, offset2, x[mode][i], y[mode][i]); break;
               case 23: result -= F23(offset1, offset2, x[mode][i], y[mode][i]); break;
               case 24: result -= F24(offset1, offset2, x[mode][i], y[mode][i]); break;
               case 25: result -= F25(offset1, offset2, x[mode][i], y[mode][i]); break;
               case 26: result -= F26(offset1, offset2, x[mode][i], y[mode][i]); break;
               case 27: result -= F27(offset1, offset2, x[mode][i], y[mode][i]); break;
               case 28: result -= F28(offset1, offset2, x[mode][i], y[mode][i]); break;
               case 29: result -= F29(offset1, offset2, x[mode][i], y[mode][i]); break;
               case 30: result -= F30(offset1, offset2, x[mode][i], y[mode][i]); break;
               case 31: result -= F31(offset1, offset2, x[mode][i], y[mode][i]); break;
               case 32: result -= F32(offset1, offset2, x[mode][i], y[mode][i]); break;
            }
            i++;
        }
        fprintf(out, "    \"%d\", \"%d\", \"%ld\", \"%ld\", \"%ld\", \n    \"", mode + 1, length, result, offset1, offset2);
        
        for(i = 0; i < length; i++) {
            salt2 = 0;
            k = 0;
            pos = i % 32;
            while(algorithm[pos][k]){
                switch(algorithm[pos][k]){   
                    case '+': salt2 += (number[pos][k * 2][0] == 'a' ? offset1 : offset2) + atol(number[pos][k * 2 + 1]); break;
                    case '-': salt2 += (number[pos][k * 2][0] == 'a' ? offset1 : offset2) - atol(number[pos][k * 2 + 1]); break; 
                    case '*': salt2 += (number[pos][k * 2][0] == 'a' ? offset1 : offset2) * atol(number[pos][k * 2 + 1]); break;
                    case '/': salt2 += (number[pos][k * 2][0] == 'a' ? offset1 : offset2) / atol(number[pos][k * 2 + 1]); break;
                    case '%': salt2 += (number[pos][k * 2][0] == 'a' ? offset1 : offset2) % atol(number[pos][k * 2 + 1]); break;       
                    case '&': salt2 += (number[pos][k * 2][0] == 'a' ? offset1 : offset2) & atol(number[pos][k * 2 + 1]); break;          
                    case '|': salt2 += (number[pos][k * 2][0] == 'a' ? offset1 : offset2) | atol(number[pos][k * 2 + 1]); break;    
                    case '^': salt2 += (number[pos][k * 2][0] == 'a' ? offset1 : offset2) ^ atol(number[pos][k * 2 + 1]); break; 
                } 
                k++;
            }
            salt2 = labs(salt2) % 256;
            fprintf(out, "%02X", ((unsigned char) text[i]) ^ ((int) salt1) ^ ((int) salt2));
        }
        fprintf(out, "\",\n");
    }
    fprintf(out, "    0\n");
    fprintf(out, "};\n\n");   
   
    i = 0;
    while(tail[i]) fprintf(out, "%s\n", tail[i++]);
   
    /*Write to the main function section*/
    i = 0;
    while(main_func[i]) fprintf(out, "%s\n", main_func[i++]);    

    i = 0;
    if(flag) {      
        while(traced[i]) fprintf(out, "%s\n", traced[i++]);        
    } else if (!strcmp(sysinfo.sysname, "SCO_SV")){
        while(ptrace_sco[i]) fprintf(out, "%s\n", ptrace_sco[i++]);          
    } else if (!strcmp(sysinfo.sysname, "AIX")){
        while(ptrace_aix[i]) fprintf(out, "%s\n", ptrace_aix[i++]);        
    } else {
        while(ptrace_linux[i]) fprintf(out, "%s\n", ptrace_linux[i++]); 
        flag = 2;            
    }
   
    fflush(out);
    fclose(out);
    printf("Ok! You can make binary program by \"cc %s -s -O2 -o %s.x%s\"\n", outname, inname, flag == 2 ? " -lpthread" : "");
    free(bitmap);
    free(code_text);
    free(obscure_text);
    free(text);
    free(inname);
    free(outname);
    return(0);
}
