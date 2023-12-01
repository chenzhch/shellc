/*
 * This code is released under
 * GNU GENERAL PUBLIC LICENSE Version 3
 * Function: Convert shell scripts into C code
 * Author: ChenZhongChao
 * E-Mail: nbczc@163.com
 * Date: 2023-12-01
 * Version: 0.1
 * Github: https://github.com/chenzhch/shellc.git
 */

#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

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
    "",
    0
};

static const char *middle[] = {
    "typedef long(*Fn)(long, long);",
    "",
    "long ff(Fn fn, long a, long b)",
    "{",
    "    return fn(a, b);",
    "}",
    "",
    "void main(int argc, char **argv)",
    "{",
    "    int i, j;",
    "    FILE *pipe;",
    "    long mask;",
    "    char str[3];",
    "    Fn fn[] = {",
    "        f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16,",
    "        f17, f18, f19, f20, f21, f22, f23, f24, f25, f26, f27, f28, f29, f30, f31, f32",
    "    };",
    "    char *main = \"____shellC___MaiN__fUNcTioN_20231201_v01_\";",
    "    struct Data {",
    "        int mode;",
    "        int length;",
    "        long bitmap;",
    "        long x;",
    "        long y;",
    0
};

static const char *tail[] = {
    "    if(getenv(\"SHELL\") == NULL) {",
    "        printf(\"Failed to get shell environment variable\\n\");",
    "        exit(1);",
    "    }",
    "    pipe = popen(getenv(\"SHELL\"), \"w\");",
    "    if(pipe == NULL) {",
    "        printf(\"Failed to popen shell\\n\");",
    "        exit(1);",
    "    }",
    "    fputs(main, pipe);",
    "    fputs(\"(){ \", pipe);",
    "    i = 0;",
    "    memset(str, 0, sizeof(str));",
    "    while(data[i].mode) {",
    "        mask = (long) 1 << (data[i].length - 1);",
    "        for(j = 0; j < data[i].length; j++) {",
    "            memcpy(str, &data[i].text[j * 2], 2);",
    "            fputc((data[i].bitmap + ff(fn[data[i].mode -1], data[i].x, data[i].y)) & mask ? strtol(str, 0, 16) : 0, pipe);",
    "            mask >>= 1;",
    "        }",
    "        i++;",
    "    }",
    "    fputs(\"\\n}\\n\", pipe);",
    "    fputs(main, pipe);",
    "    for(i = 1; i < argc; i++) {",
    "        fputs(\" \\\"\", pipe);",
    "        fputs(argv[i], pipe);",
    "        fputc('\"', pipe);",
    "    }",
    "    i = pclose(pipe);",
    "    if(i != 0) {",
    "        exit(1);",
    "    }",
    "    exit(0);",
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

void main(int argc, char **argv) 
{
    struct stat status;
    FILE *in, *out, *pipe;
    int code_length, obscure_length, length;
    unsigned char *code_text, *obscure_text;
    unsigned char *bitmap, *text;
    unsigned char str[64];
    long result, offset1, offset2;
    long mask;
    int size, digit;
    int route[32][17], x[32][16], y[32][16];
    int i, j, k, ch, loop, mode;
    if(argc != 2) {
        printf("Usage: %s file\n", argv[0]);
        exit(1);
    }
    in = fopen(argv[1], "r");
    if (in == NULL) {
        printf("Failed to open the file %s\n", argv[1]);
        exit(1);
    }
    stat(argv[1], &status);
    code_length = status.st_size;
    code_text = (unsigned char *) malloc(code_length);
    for(i = 0; i < code_length; i++) {
        code_text[i] = fgetc(in);
    }

    if(code_length < 2048) {
        obscure_length = 4096;
    } else {
        obscure_length = code_length * 2;
    }
    bitmap = (unsigned char *) malloc(obscure_length * 2);
    obscure_text = (unsigned char *) malloc(obscure_length * 2);
    srand(time(0));
    loop = 1;

    /*Randomly generated visible obfuscated characters*/
    while(loop) {
        j = 0;
        for(i = 0; i < code_length; j++) {
            digit = rand() % obscure_length;
            if(digit < code_length){
                bitmap[j] = '1';
                obscure_text[j] = code_text[i++];
            }else{
                bitmap[j] = '0';
                digit = rand() % 3;
                obscure_text[j] = digit == 0 ? code_text[i] : rand() % 95 + 32;
            }
            if(j == obscure_length * 2) break;
        }
        if( i == code_length){
            loop = 0;
        }
    }

    /*Total length after confusion*/
    obscure_length = j;

    /*Generate calculation factors*/
    for(i= 0; i < 32; i++) {
        for(j = 0; j < 16; j++){
            x[i][j] = rand() % 10+1;
            y[i][j] = rand() % 10+1;
        }
    }

    /*Generate a computation path*/
    for(i = 0; i < 32; i++) {
        k = rand() % 9 + 8;
        for(j = 0; j < k; j++) {
            route[i][j] = rand() % 32 + 1;
        }
        route[i][j]=0;
    }

    text = (char *) malloc(strlen(argv[1]) + 3);
    memset(text, 0, sizeof(text));
    sprintf(text, "%s.c", argv[1]);
    out = fopen(text, "w");
    if (out == NULL) {
        printf("Failed to write to file %s\n", text);
        exit(1);
    }

    /*Write to the file header*/
    for(i=0; head[i]; i++) {
        fprintf(out, "%s\n", head[i]);
    }

    /*Macro definitions are generated based on the calculation path*/
    for(i = 0; i < 32; i++) {
        j = 0;
        while(route[i][j] != 0) {
            if(j == 0){
                fprintf(out, "#define F%d(a, b) (\\\n", i + 1);
            }
            memset(str, 0, sizeof(str));
            macro(route[i][j], x[i][j], y[i][j], str);
            fprintf(out, "   %s%s\\\n", j == 0 ? " ":" +", str);
            j++;
        }
        fprintf(out, ")\n\n");
    }

    /*Generate a call function*/
    for(i = 0; i < 32; i++) {
        fprintf(out, "long f%d(long a, long b)\n", i + 1);
        fprintf(out, "{\n");
        fprintf(out, "    return(F%d(a, b));\n", i + 1);
        fprintf(out, "}\n\n");
    }

    /*Write to the main function section*/
    for(i=0; middle[i]; i++) {
        fprintf(out, "%s\n", middle[i]);
    }

    size = sizeof(long) * 8;
    fprintf(out, "        unsigned char text[%d];\n", (size - 2) * 2);
    fprintf(out, "    };\n");
    fprintf(out, "    struct Data data[] = {\n");
    free(text);
    text =(char *) malloc(size);
    j = 0;
    while(obscure_length) {
        mode = rand() % 32;
        length = rand() % 4 + size - 5;

        if(obscure_length < length) {
            length = obscure_length;
        }
        obscure_length -= length;
        memset(text, 0, sizeof(text));
        result = binary_to_int(bitmap + j, length);
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
        fprintf(out, "        {%d, %d, %ld, %ld, %ld, \"", mode + 1, length, result, offset1, offset2);
        for(i = 0; i < length; i++) {
            fprintf(out, "%02X", (unsigned char) text[i]);
        }
        fprintf(out, "\"},\n");
    }
    fprintf(out, "        {0, 0, 0, 0, 0, 0}\n");
    fprintf(out, "    };\n");

    /*Write to the end of the file*/
    for(i=0; tail[i]; i++) {
        fprintf(out, "%s\n", tail[i]);
    }

    fflush(out);
    fclose(out);
    free(bitmap);
    free(code_text);
    free(obscure_text);
    free(text);
}
