/*
 * This code is released under
 * GNU GENERAL PUBLIC LICENSE Version 3
 * Function: Convert script into C code
 * Author: ChenZhongChao
 * Date: 2023-12-25
 * Version: 1.7
 * Github: https://github.com/chenzhch/shellc.git
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/utsname.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct
{
    unsigned int hash[8];
    unsigned char block[64];
    unsigned int total;
    unsigned int remain;
} CTX;

#define FF(x, y, z) ((x) ^ (y) ^ (z))
#define GG(x, y, z) (((x) & (y)) | ((x) & (z)) | ((y) & (z)))
#define HH(x, y, z) (((x) & (y)) | ((~(x)) & (z)))
#define P0(x) ((x) ^ (LR((x), 9)) ^ (LR((x), 17)))
#define P1(x) ((x) ^ (LR((x), 15)) ^ (LR((x), 23)))
#define SWAP(x) (((x) >> 24) | (((x) & 0x00FF0000) >> 8) | (((x) & 0x0000FF00) << 8) | ((x) << 24))

unsigned int LR(unsigned int x, unsigned n) 
{
    n = n % 32;
    return(x << n | x >> (32 - n));
}

void init(CTX *ctx)
{
    ctx->hash[0] = 0x7380166F;
    ctx->hash[1] = 0x4914B2B9;
    ctx->hash[2] = 0x172442D7;
    ctx->hash[3] = 0xDA8A0600;
    ctx->hash[4] = 0xA96F30BC;
    ctx->hash[5] = 0x163138AA;
    ctx->hash[6] = 0xE38DEE4D;
    ctx->hash[7] = 0xB0FB0E4E;
    ctx->total = 0;
    ctx->remain = 0;
}

void CF(CTX *ctx)
{
    int i = 0;
    unsigned int w0[68];
    unsigned int w1[64];
    unsigned int a, b, c, d, e, f, g, h, ss1, ss2, tt1, tt2;
    unsigned int t[64] = {0};

    for (i = 0; i < 16; i++) {
        t[i] = 0x79CC4519;
    }

    for (i = 16; i < 64; i++) {
        t[i] = 0x7A879D8A;
    }

    for (i = 0; i < 16; i++) {
        w0[i] = SWAP(*(unsigned int *)(ctx->block + i * 4));
    }

    for (i = 16; i < 68; i++) {
        w0[i] = P1(w0[i - 16] ^ w0[i - 9] ^ LR(w0[i - 3], 15))
                ^ LR(w0[i - 13], 7)
                ^ w0[i - 6];
    }
    for (i = 0; i < 64; i++) {
        w1[i] = w0[i] ^ w0[i + 4]; 
    }
    a = ctx->hash[0];
    b = ctx->hash[1];
    c = ctx->hash[2];
    d = ctx->hash[3];
    e = ctx->hash[4];
    f = ctx->hash[5];
    g = ctx->hash[6];
    h = ctx->hash[7];
    for (i = 0; i < 64; i++) {
        ss1 = LR((LR(a, 12) + e + LR(t[i], i)), 7);
        ss2 = ss1 ^ LR(a, 12);
        if (i < 16) {
            tt1 = FF(a, b, c) + d + ss2 + w1[i];
            tt2 = FF(e, f, g) + h + ss1 + w0[i];
        } else {
            tt1 = GG(a, b, c) + d + ss2 + w1[i];
            tt2 = HH(e, f, g) + h + ss1 + w0[i];
        }
        d = c;
        c = LR(b, 9);
        b = a;
        a = tt1;
        h = g;
        g = LR(f, 19);
        f = e;
        e = P0(tt2);
    }
    ctx->hash[0] ^= a;
    ctx->hash[1] ^= b; 
    ctx->hash[2] ^= c;
    ctx->hash[3] ^= d; 
    ctx->hash[4] ^= e;
    ctx->hash[5] ^= f;
    ctx->hash[6] ^= g;
    ctx->hash[7] ^= h;
}

void update(CTX *ctx, unsigned char *buf, unsigned int len) 
{
    unsigned int i, pos;
    ctx->total += len;
    if (ctx->remain + len < 64) {
        memcpy(ctx->block + ctx->remain, buf, len);
        ctx->remain += len;
    } else {
        pos = 64 - ctx->remain;
        memcpy(ctx->block + ctx->remain, buf, pos);
        CF(ctx);
        for (i = 0; i < (len - pos) / 64; i++) {
            memcpy(ctx->block, buf + pos + i * 64, 64);
            CF(ctx);    
        }         
        ctx->remain = (len - pos) % 64;
        memcpy(ctx->block, buf + pos + i * 64, ctx->remain);
    }    
}

void final(CTX *ctx, unsigned char *digest)
{
    unsigned int i, bits;
    bits = SWAP(ctx->total * 8);
    ctx->block[ctx->remain] = 0x80;
    if (ctx->remain <= 55) {        
        memset(ctx->block + ctx->remain + 1, 0, 64 - ctx->remain - 1 - 8 + 4);
        memcpy(ctx->block + 64 - 4, &bits, 4);
        CF(ctx);
    } else {
        memset(ctx->block + ctx->remain + 1, 0, 64 - ctx->remain - 1);
        CF(ctx);
        memset(ctx->block, 0, 64 - 4);
        memcpy(ctx->block + 64 - 4, &bits, 4);
        CF(ctx);
    }     
    for (i = 0; i < 8; i++) {
        ctx->hash[i]=SWAP(ctx->hash[i]);
        memcpy(digest, ctx->hash, 32);
    }
}

int text_sign(unsigned char *buf, unsigned int len, unsigned char *sign)
{
    unsigned char digest[32];
    int i = 0;
    CTX ctx;
    init(&ctx);
    update(&ctx, buf, len);
    final(&ctx, digest);    
    for (i = 0; i < 32; i++) {
        sprintf((char *) sign + i * 2, "%02x", digest[i]);
    }
    return(0);
}

int file_sign(unsigned char *name, unsigned char *sign)
{
    FILE *in;
    unsigned char buf[256],  digest[32];
    int i = 0, len = 0;
    CTX ctx;
    in = fopen((char *) name, "rb");
    if (in == NULL) {
        fprintf(stderr, "Failed to open the file: %s\n", name);
        return(1);
    }
    init(&ctx);
    while ((len = (int) fread(buf, 1, sizeof(buf), in))) {
        update(&ctx, buf, len);    
    }
    fclose(in);
    final(&ctx, digest);
    for (i = 0; i < 32; i++) {
        sprintf((char *) sign + i * 2, "%02x", digest[i]);
    }
    return(0);
}

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
#define F14(a, b, c, d) ((b&c)<(a&d)?a&c:b|d)
#define F15(a, b, c, d) ((a&c)<(b&d)?a|c:b&d)
#define F16(a, b, c, d) ((a&c)>(b&d)?a&c:b|d)
#define F17(a, b, c, d) ((c|b)>(d|a)?a|c:b&d)
#define F18(a, b, c, d) ((a|~c)<(b|~d)?a<<c:b&d)
#define F19(a, b, c, d) ((b|c)<(a|d)?a<<c:b&d)
#define F20(a, b, c, d) ((b|c)<(a|d)?b&c:a<<d)
#define F21(a, b, c, d) ((a|c)<(b|d)?b|c:b&d)
#define F22(a, b, c, d) ((a|c)>(b|d)?a<<c:b|d)
#define F23(a, b, c, d) ((~a|~c)<(~b|~d)?a<<c:b+d)
#define F24(a, b, c, d) ((~a|~c)<(~b|~d)?a-c:b+d)
#define F25(a, b, c, d) ((b|c)>(a|d)?a|c:b<<d)
#define F26(a, b, c, d) ((a|~c)<(b|~d)?a&c:b<<d)
#define F27(a, b, c, d) ((~a|~c)<(b|~d)?a|c:b&d)
#define F28(a, b, c, d) ((~a|~c)>(b|~d)?a&c:b|d)
#define F29(a, b, c, d) ((a|c)<(b|d)?a&b:c&d)
#define F30(a, b, c, d) ((~b|~a)>(~c|d)?a<<c:b|d)
#define F31(a, b, c, d) ((a|~b)>(c|d)?a&c:d&b)
#define F32(a, b, c, d) ((a|b)>(c|~d)?a|c:d|b)

typedef long(*Fn)(long, long);

long ff(Fn fn, long a, long b)
{
    return fn(a, b);
}

static const char *format[] = {
    "BASH",
    "FISH",
    "ZSH",
    "PERL",
    "PYTHON",
    "JAVASCRIPT",
    "LUA",
    "RUBY",
    "CSH",
    "TCLSH",
    "PHP",
    "RC",
    0
};

static const char *fix_code[][2] = { 
    {"BASH_ARGV0='?'; ", 0}, /*bash*/
    {"set 0 '?'; ", 0}, /*fish*/
    {"0='?'; ", 0}, /*zsh*/
    {"$0='?'; ", 0}, /*perl*/
    {"import sys; sys.argv[0] = '''?'''; ", 0}, /*python*/
    {"__filename = `?`; process.argv[1] = `?`; ", 0}, /*javascript*/
    {"arg[0] = '?'; ", 0}, /*lua*/
    {"$0='?'; ", 0}, /*ruby*/
    {0, 0}, /*csh*/
    {0, 0}, /*tclsh*/
    {"$argv[0] = '?'; ", 0}, /*php*/
    {0, 0}, /*rc*/
    {0, 0}
};

static const char *arg_code[][2] = { 
    {0, 0}, /*bash*/
    {"sprintf(name, \"set -a argv '%s'; \", argv[i]);", 0}, /*fish*/
    {0, 0}, /*zsh*/
    {"sprintf(name, \"push @ARGV, '%s'; \", argv[i]);", 0}, /*perl*/
    {"sprintf(name, \"sys.argv.append('''%s'''); \", argv[i]);", 0}, /*python*/
    {"sprintf(name, \"process.argv.push(`%s`); \", argv[i]);", 0}, /*javascript*/
    {"sprintf(name, \"table.insert(arg, '%s'); \", argv[i]);", 0}, /*lua*/
    {"sprintf(name, \"ARGV.push('%s'); \", argv[i]);", 0}, /*ruby*/
    {"sprintf(name, \"set argv = ($argv %s); \", argv[i]);", 0}, /*csh*/
    {"sprintf(name, \"set argv [concat $argv %s]; \", argv[i]);", 0}, /*tclsh*/
    {"sprintf(name, \"array_push($argv, '%s'); \", argv[i]);", 0}, /*php*/
    {"sprintf(name, \"* = ($* %s); \", argv[i]);", 0}, /*rc*/
    {0, 0}
};

static const char *sysname[] = {
    "NetBSD",
    "Darwin",
    "DragonFly",
    0
};

static const char *head[] = {
    "#include <stdio.h>",
    "#include <string.h>",
    "#include <stdlib.h>",
    "#include <signal.h>",
    "#include <unistd.h>",
    "#include <sys/types.h>",
    "#include <sys/wait.h>", 
    "#include <sys/stat.h>",
    "#include <sys/resource.h>",
    0
};

static const char *func_def[] = {
    "typedef long(*Fn)(long, long);",
    "",
    "long ff(Fn fn, long a, long b)",
    "{",
    "    return fn(a, b);",
    "}",
    "",
    "typedef struct",
    "{",
    "    unsigned int hash[8];",
    "    unsigned char block[64];",
    "    unsigned int total;",
    "    unsigned int remain;",
    "} CTX;",
    "",
    "#define FF(x, y, z) ((x) ^ (y) ^ (z))",
    "#define GG(x, y, z) (((x) & (y)) | ((x) & (z)) | ((y) & (z)))",
    "#define HH(x, y, z) (((x) & (y)) | ((~(x)) & (z)))",
    "#define P0(x) ((x) ^ (LR((x), 9)) ^ (LR((x), 17)))",
    "#define P1(x) ((x) ^ (LR((x), 15)) ^ (LR((x), 23)))",
    "#define SWAP(x) (((x) >> 24) | (((x) & 0x00FF0000) >> 8) | (((x) & 0x0000FF00) << 8) | ((x) << 24))",
    "",
    "unsigned int LR(unsigned int x, unsigned n) ",
    "{",
    "     n = n % 32;",
    "     return(x << n | x >> (32 - n));",
    "}",
    "",
    "void init(CTX *ctx)",
    "{",
    "    ctx->hash[0] = 0x7380166F;",
    "    ctx->hash[1] = 0x4914B2B9;",
    "    ctx->hash[2] = 0x172442D7;",
    "    ctx->hash[3] = 0xDA8A0600;",
    "    ctx->hash[4] = 0xA96F30BC;",
    "    ctx->hash[5] = 0x163138AA;",
    "    ctx->hash[6] = 0xE38DEE4D;",
    "    ctx->hash[7] = 0xB0FB0E4E;",
    "    ctx->total = 0;",
    "    ctx->remain = 0;",
    "}",
    "",
    "void CF(CTX *ctx)",
    "{",
    "    int i = 0;",
    "    unsigned int w0[68];",
    "    unsigned int w1[64];",
    "    unsigned int a, b, c, d, e, f, g, h, ss1, ss2, tt1, tt2;",
    "    unsigned int t[64] = {0};",
    "",
    "    for (i = 0; i < 16; i++) {",
    "        t[i] = 0x79CC4519;",
    "    }",
    "",
    "    for (i = 16; i < 64; i++) {",
    "        t[i] = 0x7A879D8A;",
    "    }",
    "",
    "    for (i = 0; i < 16; i++) {",
    "        w0[i] = SWAP(*(unsigned int *)(ctx->block + i * 4));",
    "    }",
    "",
    "    for (i = 16; i < 68; i++) {",
    "        w0[i] = P1(w0[i - 16] ^ w0[i - 9] ^ LR(w0[i - 3], 15))",
    "                ^ LR(w0[i - 13], 7)",
    "                ^ w0[i - 6];",
    "    }",
    "    for (i = 0; i < 64; i++) {",
    "        w1[i] = w0[i] ^ w0[i + 4]; ",
    "    }",
    "    a = ctx->hash[0];",
    "    b = ctx->hash[1];",
    "    c = ctx->hash[2];",
    "    d = ctx->hash[3];",
    "    e = ctx->hash[4];",
    "    f = ctx->hash[5];",
    "    g = ctx->hash[6];",
    "    h = ctx->hash[7];",
    "    for (i = 0; i < 64; i++) {",
    "        ss1 = LR((LR(a, 12) + e + LR(t[i], i)), 7);",
    "        ss2 = ss1 ^ LR(a, 12);",
    "        if (i < 16) {",
    "            tt1 = FF(a, b, c) + d + ss2 + w1[i];",
    "            tt2 = FF(e, f, g) + h + ss1 + w0[i];",
    "        } else {",
    "            tt1 = GG(a, b, c) + d + ss2 + w1[i];",
    "            tt2 = HH(e, f, g) + h + ss1 + w0[i];",
    "        }",
    "        d = c;",
    "        c = LR(b, 9);",
    "        b = a;",
    "        a = tt1;",
    "        h = g;",
    "        g = LR(f, 19);",
    "        f = e;",
    "        e = P0(tt2);",
    "    }",
    "    ctx->hash[0] ^= a;",
    "    ctx->hash[1] ^= b; ",
    "    ctx->hash[2] ^= c;",
    "    ctx->hash[3] ^= d; ",
    "    ctx->hash[4] ^= e;",
    "    ctx->hash[5] ^= f;",
    "    ctx->hash[6] ^= g;",
    "    ctx->hash[7] ^= h;",
    "}",
    "",
    "void update(CTX *ctx, unsigned char *buf, unsigned int len) ",
    "{",
    "    unsigned int i, pos;",
    "    ctx->total += len;",
    "    if (ctx->remain + len < 64) {",
    "        memcpy(ctx->block + ctx->remain, buf, len);",
    "        ctx->remain += len;",
    "    } else {",
    "        pos = 64 - ctx->remain;",
    "        memcpy(ctx->block + ctx->remain, buf, pos);",
    "        CF(ctx);",
    "        for (i = 0; i < (len - pos) / 64; i++) {",
    "            memcpy(ctx->block, buf + pos + i * 64, 64);",
    "            CF(ctx);",
    "        }",
    "        ctx->remain = (len - pos) % 64;",
    "        memcpy(ctx->block, buf + pos + i * 64, ctx->remain);",
    "    }",
    "}",
    "",
    "void final(CTX *ctx, unsigned char *digest)",
    "{",
    "    unsigned int i, bits;",
    "    bits = SWAP(ctx->total * 8);",
    "    ctx->block[ctx->remain] = 0x80;",
    "    if (ctx->remain <= 55) {        ",
    "        memset(ctx->block + ctx->remain + 1, 0, 64 - ctx->remain - 1 - 8 + 4);",
    "        memcpy(ctx->block + 64 - 4, &bits, 4);",
    "        CF(ctx);",
    "    } else {",
    "        memset(ctx->block + ctx->remain + 1, 0, 64 - ctx->remain - 1);",
    "        CF(ctx);",
    "        memset(ctx->block, 0, 64 - 4);",
    "        memcpy(ctx->block + 64 - 4, &bits, 4);",
    "        CF(ctx);",
    "    }     ",
    "    for (i = 0; i < 8; i++) {",
    "        ctx->hash[i]=SWAP(ctx->hash[i]);",
    "        memcpy(digest, ctx->hash, 32);",
    "    }",
    "}",
    "",
    "int text_sign(unsigned char *buf, unsigned int len, unsigned char *sign)",
    "{",
    "    unsigned char digest[32];",
    "    int i = 0;",
    "    CTX ctx;",
    "    init(&ctx);",
    "    update(&ctx, buf, len);",
    "    final(&ctx, digest);    ",
    "    for (i = 0; i < 32; i++) {",
    "        sprintf((char *) sign + i * 2, \"%02x\", digest[i]);",
    "    }",
    "    return(0);",
    "}",
    "",
    "int file_sign(unsigned char *name, unsigned char *sign)",
    "{",
    "    FILE *in;",
    "    unsigned char buf[256],  digest[32];",
    "    int i = 0, len = 0;",
    "    CTX ctx;",
    "    in = fopen((char *) name, \"rb\");",
    "    if (in == NULL) {",
    "        fprintf(stderr, \"Failed to open the file: %s\\n\", name);",
    "        return(1);",
    "    }",
    "    init(&ctx);",
    "    while ((len = (int) fread(buf, 1, sizeof(buf), in))) {",
    "        update(&ctx, buf, len);",
    "    }",
    "    fclose(in);",
    "    final(&ctx, digest);",
    "    for (i = 0; i < 32; i++) {",
    "        sprintf((char *) sign + i * 2, \"%02x\", digest[i]);",
    "    }",
    "    return(0);",
    "}",
    "",
    "int which(const char *command, char *fullname)",
    "{",
    "    char *name;",
    "    int i, j, len;",
    "    char *path = getenv(\"PATH\");",
    "    if (command[0] == '/' || (command[0] == '.')) {",
    "        if (access(command, X_OK) == 0) {",
    "            memset(fullname, 0, (size_t) strlen(fullname));",
    "            strcpy(fullname, command);",
    "            return(0);",
    "        }",
    "        fprintf(stderr, \"Error: command %s not found\\n\", command);",
    "        return(1);",
    "    }",
    "    if (!path) {",
    "        fprintf(stderr, \"PATH environment variable not set\\n\");",
    "        return(1);",
    "    } ",   
    "    len = strlen(path) + strlen(command) + 8;",
    "    name = (char *) malloc(len);", 
    "    memset(name, 0,  (size_t) len);", 
    "    j = 0;",
    "    len = strlen(path);",
    "    for (i = 0; i <= len; i++) {",
    "        if(i == len || path[i] == ':') {",
    "            strcat(name, \"/\");",\
    "            strcat(name, command);",
    "            if (access(name, X_OK) == 0) {",
    "                memset(fullname, 0, (size_t) strlen(fullname));",
    "                strcpy(fullname, name);",
    "                free(name);",
    "                return(0);",
    "            }",
    "            memset(name, 0,  (size_t) len);", 
    "            j = 0;",   
    "        } else {",
    "            name[j++] = path[i];",
    "        }",
    "    }", 
    "    free(name);",
    "    fprintf(stderr, \"Error: command %s not found\\n\", command);",
    "    return(1);",
    "}",
    "",
    0
};

static const char *first[] = {
    "{",
    "    int i, j, k;",
    "    long mask, salt1, salt2, salt, length, script_length = 0;",
    "    char chr[3], *str, *seed, summary[65];",
    "    char *fullname;",
    "    FILE *script = NULL;",
    "    struct stat status;", 
    "    unsigned char byte = 0x0;",
    "    Fn fn[] = {",
    "        f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14, f15, f16,",
    "        f17, f18, f19, f20, f21, f22, f23, f24, f25, f26, f27, f28, f29, f30, f31, f32,",
    "        f33, f34, f35, f36, f37, f38, f39, f40, f41, f42, f43, f44, f45, f46, f47, f48,",
    "        f49, f50, f51, f52, f53, f54, f55, f56, f57, f58, f59, f60, f61, f62, f63, f64",
    "    };",
    "    struct rlimit rl;",
    "    if (getrlimit(RLIMIT_CORE, &rl) == -1) {",
    "        perror(\"Failed to getrlimit\");",
    "        return(1);",
    "    }",
    "    memset(summary, 0, sizeof(summary));",
    "    srand(getpid());",
    "    i = 0;",
    "    salt = strlen(command) + strlen(tracer) + strlen(digest) + 2;",
    "    seed = (char *) malloc((size_t) (salt + 16));",
    "    memset(seed, 0, salt + 16);",
    "    strcat(seed, command);",
    "    strcat(seed, tracer);",
    "    if (inner || anti) {",
    "        length = strlen(getenv(\"PATH\")) + strlen(command) + 1;",
    "        fullname = malloc((size_t) length);",
    "        if (which(command, fullname)) {",
    "            free(seed);",
    "            free(fullname);",
    "            return(1);",
    "        }",
    "        if(file_sign((unsigned char *) fullname, (unsigned char *) summary)) {",
    "            free(seed);",
    "            free(fullname); ",
    "            return(1);",
    "        }",
    "        stat(fullname, &status);", 
    "        script_length = status.st_size;",  
    "        script = fopen(fullname, \"rb\");",
    "        if (script == NULL) {",
    "             fprintf(stderr, \"Failed to open the file: %s\\n\", fullname);",
    "             free(seed);",
    "             free(fullname);",
    "             return(1);",
    "        }",  
    "        free(fullname);",   
    "    } else {",
    "        strcpy(summary, digest);",
    "    }",
    "    strcat(seed, summary);",
    "    sprintf(seed + strlen(seed), \"%d%d\", inner, anti);",
    "    memset(chr, 0, (size_t) sizeof(chr));",
    "    while (data[i]) {",
    "        mask = (long) 1 << (atol(data[i + 1]) - 1 + rl.rlim_cur);",
    "        k = rand() % 8 + 2;",
    "        str = (char *) malloc(k);",
    "        memset(str, 0, (size_t) k);",
    "        for (j = 0; j < atol(data[i + 1]); j++) {",        
    "            memcpy(chr, &data[i + 5][j * 2], 2);",   
    "            salt1 = (atol(data[i + 2]) + ff(fn[atol(data[i]) -1], atol(data[i + 3]), atol(data[i + 4]))) % 256;",
    "            salt2 = ff(fn[j % 32 + 32], atol(data[i + 3]), atol(data[i + 4])) % 256;", 
    "            if (inner || anti) {",
    "                length = (script_length / (salt2 + 64) + script_length - salt1 + script_length / salt ) % script_length + rl.rlim_cur;",
    "                if (fseek(script, length, SEEK_SET) != 0) {",
    "                    perror(\"Failed to seeking interpreter file\");",
    "                    fclose(script);",
    "                    free(seed);",
    "                    return(1);",
    "                }",                              
    "                if (fread(&byte, sizeof(byte), 1, script) != 1) {",
    "                    perror(\"Failed to reading interpreter file\");",
    "                    fclose(script);",
    "                    free(seed);",    
    "                    return(1);",
    "                }", 
    "            }",        
    "            sprintf(",
    "                str + strlen(str), \"%c\",",
    "                (atol(data[i + 2]) + ff(fn[atol(data[i]) -1], atol(data[i + 3]), atol(data[i + 4])) + rl.rlim_cur) & mask",
    "                ? (int) strtol(chr, 0, 16)",
    "                ^ (int) salt1",
    "                ^ (int) salt2",
    "                ^ ((unsigned char) seed[(salt1 + salt2) % salt])",
    "                ^ byte",
    "                : 0",
    "            );",  
    "            if (strlen(str) == k - 1) {",
    0
};

static const char *sencod[] = {
    "                    perror(\"Failed to write script\");",
    "                    free(str);",
    "                    free(seed);",
    "                    return(1);",
    "                }",
    "                memset(str, 0, (size_t) k);",
    "            }",            
    "            mask >>= 1;",
    "        }",
    "        if (strlen(str) != 0) {",
    0
};

static const char *third[] = {
    "                perror(\"Failed to write script\");",
    "                free(seed);",
    "                free(str);",
    "                return(1);",
    "            }",
    "        }",
    "        free(str);",
    "        i += 6;",
    "    }",    
    "    free(seed);",
    "    if (inner || anti) fclose(script);",
    "    return(0);",
    "}",  
    "",    
    0
};

static const char *fourth[] = {
    "int handler(int argc, char **argv)",
    "{",
    "    char dev[64], *str;",                                 
    "    int file[2], i;",
    "    pid_t pid;",
    "    int j, status;",
    "    char **args;",
    "    str = strdup(\"Failed to pipe\");",
    "    if (pipe(file) == -1) {",
    "        perror(str);",
    "        free(str);",
    "        return 1;",
    "    }",
    "    free(str);",
    "    memset(dev, 0, sizeof(dev));",
    "    sprintf(dev, \"/proc/self/fd/%d\", file[0]);",
    "    if (access(dev, R_OK)) {",
    "       sprintf(dev, \"/dev/fd/%d\", file[0]);",
    "    }",
    "    args = (char **) malloc((argc + 8) * sizeof(char *));",
    0
};

static const char *fourth_safe[] = {
    "int handler(int argc, char **argv)",
    "{",
    "    FILE *pipe;",
    "    char *name, *str;",
    "    int length, i, status;",
    "    str = strdup(getenv(\"PATH\"));",
    "    length = strlen(str) + strlen(command) + 1;",
    "    free(str);",
    "    name = malloc((size_t) length);",
    "    if (which(command, name)) {",   
    "        return(1);",
    "    }",  
    0
};

static const char *fourth_end[] = {
    "    free(name);",
    "    pipe = popen(str, \"w\");",
    "    if (pipe == NULL) {",
    "        perror(\"Failed to popen\");",
    "        return(1);",
    "    }",
    "    free(str);",
    "",
    0
};

static const char *sh_start[] = {
    "    srand(getpid());",
    "    length = rand() % 9 + 16;",
    "    name = malloc((size_t) length);",
    "    memset(name, 0, length);",
    "    for (i = 0; i < length - 1; i++) {",     
    "        switch (rand() % 3) {",
    "            case 0:",
    "                sprintf(name + i, \"%c\", rand() % 26 + 65);",
    "                break;",
    "            case 1:",    
    "                sprintf(name + i, \"%c\", rand() % 26 + 97);",
    "                break;", 
    "            default:",
    "                sprintf(name + i, \"%c\", 95);",
    "                break;",           
    "        }",
    "    }",
    "    fputs(name, pipe);",
    "    fputs(\"() { \", pipe);",
    0
};

static const char *fifth[] = {    
    "    args[j++] = strdup(dev);",
    "    for (i = 1; i < argc; i++) {",
    "        args[j++] = strdup(argv[i]);",    
    "    }",
    "    args[j] = 0;",  
    "    if ((pid = fork()) == 0) {",  
    "       close(file[0]);",
    "       write_script(file[1]);", 
    "       return(0);",
    "    } else if (pid < 0) {",
    "        perror(\"Failed to fork\");",
    "        return(1);",
    "    }",
    "    wait(&status);",
    "    close(file[1]);",
    "    execvp(command, args);",
    "    perror(\"Failed to execvp\");",
    "    return(1);",  
    "}",
    "",
    0
};

static const char *sh_end[] = {
    "    fputs(\"\\n} ; \", pipe);",
    "    fputs(name, pipe);",
    "    free(name);",
    "    fputc(' ', pipe);",
    "    for (i = 1; i < argc; i++) {",
    "        fputs(\" \\\"\", pipe);",
    "        fwrite(argv[i], sizeof(char), strlen(argv[i]), pipe);",
    "        fputc('\"', pipe);",
    "    }",
    "    fputs(\" </dev/tty\", pipe);",
    0
};

static const char *arg_start[] = {    
    "    for (i = 1; i < argc; i++) {",
    "        length = strlen(argv[i]) + 128;",
    "        name = malloc(length);",
    "        memset(name, 0, length);",
    0
};
 
static const char *arg_end[] = {
    "        fwrite(name, sizeof(char), strlen(name), pipe);",
    "        free(name);",
    "    }",      
    0
};

static const char *fifth_safe[] = {
    "    if ((status = pclose(pipe))) {",
    "        return(WEXITSTATUS(status));",
    "    }",
    "    return(0);",
    "}",  
    "",
    0
};

static const char *main_func[] = {
    "int main(int argc, char **argv)",
    "{", 
    0,
};

static const char *ptrace_self[] = {
    "    char str[1024];",
    "    char *name = \"/proc/self/status\";",
    "    int tracer_pid = 0;",
    0
};

static const char *ptrace_self_end[] = {
    "    in = fopen(name, \"r\");",
    "    if (in == NULL) {",
    "        fprintf(stderr, \"Failed to open the file: %s\\n\", name);",
    "        return(1);",
    "    }",
    "    while (fgets(str, sizeof(str), in)) {",
    "        if (strstr(str, tracer) != NULL) {",
    "            tracer_pid = atoi(strchr(str, tracer[strlen(tracer) -1]) + 1);",
    "            break;",
    "        }",
    "    }",
    "    fclose(in);",
    "    if (tracer_pid) {",
    "        return(1);",
    "    }",
    0
};

static const char *handler[] = {
    "    return(handler(argc, argv));",
    "}",
    0 
};

static const char *ptrace_aix[] = {
    "    pid_t pid = 0;",
    "    int status;",
    0
};


static const char *ptrace_aix_end[] = {   
    "    if ((pid = fork()) == 0) {",
    "        if (__linux_ptrace(PT_WRITE_GPR, getppid(), 0, 0)) {",
    "            kill(getppid(), SIGKILL);",
    "        } else {",
    "            __linux_ptrace(PT_WRITE_FPR, getppid(), 0, 0);",
    "        }",
    "        return(0);",
    "    } else if (pid < 0) {",
    "        perror(\"Failed to fork\");",
    "        return(1);",
    "    }",
    "    wait(0);",
    0
};

static const char *handler_fork[] = {
    "    if ((pid = fork()) == 0) {", 
    "        return(handler(argc, argv));",
    "    } else if (pid < 0) {",
    "        perror(\"Failed to fork\");",
    "        return(1);",
    "    }",
    "    wait(&status);",
    "    if (status) {",
    "        return(1);",
    "    } else {",
    "        return(0);",
    "    }",
    "}",
    0
};

static const char *ptrace_unix[] = {
    "    pid_t pid = 0;",    
    "    int status;",
    0
};

static const char *ptrace_unix_end[] = {
    "    if (ptrace(0, 0, 0, 0)) {",
    "        return(1);",        
    "    }",
    0 
};

static const char *traced[] = {
    "    int length;",
    "    unsigned char *fullname;",
    "    unsigned char summary[65];",
    "    FILE *in = NULL;",
    "    struct rlimit rl;",
    "    if (getrlimit(RLIMIT_CORE, &rl) == -1) {",
    "        perror(\"Failed to getrlimit\");",
    "        return(1);",
    "    }",
    "    rl.rlim_cur = 0;",
    "    if (setrlimit(RLIMIT_CORE, &rl) == -1) {",
    "        perror(\"Failed to setrlimit\");",
    "        return(1);",
    "    }",
    "    if (inner) {",
    "        if (access(command, F_OK)) {",
    "            in = fopen(command, \"wb\");",
    "            if (in == NULL) {",
    "                perror(\"Failed to write interpreter\");",
    "                return(1);",
    "            }",
    "            if (fwrite(interpreter, 1, sizeof(interpreter), in) != sizeof(interpreter)) {",
    "                perror(\"Failed to write interpreter\");",
    "                fclose(in);",
    "                return(1);",
    "            }",
    "            fclose(in);",
    "        }",
    "        if (access(command, X_OK)) {",
    "            if (chmod(command, S_IRUSR | S_IWUSR | S_IXUSR)) {",
    "                perror(\"Failed to chmod interpreter\");",
    "                return(1);",
    "            }",
    "        }",
    "    }",
    "    length = strlen(getenv(\"PATH\")) + strlen(command) + 1;",
    "    fullname = malloc((size_t) length);",
    "    if (which(command, (char *) fullname)) {",
    "        free(fullname);",
    "        return(1);",
    "    }",       
    "    if (inner || anti) {",
    "        memset(summary, 0, sizeof(summary));",
    "        if (file_sign((unsigned char *) fullname, (unsigned char *) summary)) {",
    "            free(fullname);",
    "            return(1);",
    "        }",
    "        if (strcmp((char *) summary, (char *) digest)) {",
    "            fprintf(stderr, \"Error: invalid interpreter %s\\n\", fullname);",
    "            free(fullname);",
    "            return(1);",
    "        }",
    "    }",    
    "    free(fullname);",
    0
};

/*Which command*/
int which(const char *command, char *fullname)
{
    char *name;
    int i, j, len;
    char *path = getenv("PATH");
    if (command[0] == '/' || (command[0] == '.')) {
        if (access(command, X_OK) == 0) {
            memset(fullname, 0, (size_t) strlen(fullname));
            strcpy(fullname, command);
            return(0);
        }
        fprintf(stderr, "Error: Command %s not found\n", command);
        return(1);
    }
    if (!path) {
        fprintf(stderr, "PATH environment variable not set\n");
        return(1);
    } 
    len = strlen(path) + strlen(command) + 8;
    name = (char *) malloc(len);
    memset(name, 0,  (size_t) len);
    j = 0;
    len = strlen(path);
    for (i = 0; i <= len; i++) {
        if (i == len || path[i] == ':') {
            strcat(name, "/");
            strcat(name, command);
            if (access(name, X_OK) == 0) {
                memset(fullname, 0, (size_t) strlen(fullname));
                strcpy(fullname, name);                free(name);
                return(0);
            }
            memset(name, 0,  (size_t) len);
            j = 0;
        } else {
            name[j++] = path[i];
        }
    }
    free(name);
    fprintf(stderr, "Error: Command %s not found\n", command);
    return(1);
}
    
/*Binary characters to decimal numbers*/
long binary_to_int(char *str, int len)
{
    int i;
    long val = 0;
    long mask = 1;
    for (i = len -1; i >= 0;i--) {
        val += (str[i] - '0') * mask;
        mask <<= 1;
    }
    return(val);
}

/*Generate function definition*/
void function(int order, int x, int y, char *str)
{
    switch (order) {
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
        case 14: sprintf(str, "((b&%d)<(a&%d)?a&%d:b|%d)", x, y, x, y); break;
        case 15: sprintf(str, "((a&%d)<(b&%d)?a|%d:b&%d)", x, y, x, y); break;
        case 16: sprintf(str, "((a&%d)>(b&%d)?a&%d:b|%d)", x, y, x, y); break;
        case 17: sprintf(str, "((%d|b)>(%d|a)?a|%d:b&%d)", x, y, x, y); break;
        case 18: sprintf(str, "((a|~%d)<(b|~%d)?a<<%d:b&%d)", x, y, x, y); break;
        case 19: sprintf(str, "((b|%d)<(a|%d)?a<<%d:b&%d)", x, y, x, y); break;
        case 20: sprintf(str, "((b|%d)<(a|%d)?b&%d:a<<%d)", x, y, x, y); break;
        case 21: sprintf(str, "((a|%d)<(b|%d)?b|%d:b&%d)", x, y, x, y); break;
        case 22: sprintf(str, "((a|%d)>(b|%d)?a<<%d:b|%d)", x, y, x, y); break;
        case 23: sprintf(str, "((~a|~%d)<(~b|~%d)?a<<%d:b+%d)", x, y, x, y); break;
        case 24: sprintf(str, "((~a|~%d)<(~b|~%d)?a-%d:b+%d)", x, y, x, y); break;
        case 25: sprintf(str, "((b|%d)>(a|%d)?a|%d:b<<%d)", x, y, x, y); break;
        case 26: sprintf(str, "((a|~%d)<(b|~%d)?a&%d:b<<%d)", x, y, x, y); break;
        case 27: sprintf(str, "((~a|~%d)<(b|~%d)?a|%d:b&%d)", x, y, x, y); break;
        case 28: sprintf(str, "((~a|~%d)>(b|~%d)?a&%d:b|%d)", x, y, x, y); break;
        case 29: sprintf(str, "((a|%d)<(b|%d)?a&b:%d&%d)", x, y, x, y); break;
        case 30: sprintf(str, "((~b|~a)>(~%d|%d)?a<<%d:b|%d)", x, y, x, y); break;
        case 31: sprintf(str, "((a|~b)>(%d|%d)?a&%d:%d&b)", x, y, x, y); break;
        case 32: sprintf(str, "((a|b)>(%d|~%d)?a|%d:%d|b)", x, y, x, y); break;
    }
}

int main(int argc, char **argv)
{
    FILE *in, *out, *fix_file, *self_file, *script = NULL;
    int code_length, obscure_length, length = 0, pos;
    int fix_pos = -1;
    char *code_text, *obscure_text, *text;
    char *bitmap, *inname = NULL, *outname, *command = NULL, *parameter = NULL;
    char *fix_format = NULL, *file_name = NULL, *bit = NULL, *fullname = NULL;
    char str[1024];
    long result, offset1, offset2, script_length = 0L;
    long salt1, salt2, salt;
    int size, digit, opt;   
    int route[32][17], x[32][16], y[32][16];
    char *operators = "+-*/%&|^";
    char number[32][32][8], digest[65], buffer[32];
    char algorithm[32][17];
    unsigned char byte = 0x0;
    int i, j, k, loop, mode;
    int trace_flag = 0, fix_flag = 0, input_flag = 0, command_flag = 0, file_flag = 0;   
    int self_flag = 0, safe_flag = 0, bit_flag = 0, para_flag = 0, anti_flag = 0, inner_flag = 0;    
    struct utsname sysinfo;
    struct stat status; 
    char *self_name = "/proc/self/status";
    char option[32];
    char **args = (char **) malloc((argc + 1) * sizeof(char *));
    char *seed;
    char *usage = "command inputfile [-t] [-s] [-a] [-f fix-format] [-e fix-file] [-p parameter] [-b 8|16|32|64] [-i interpreter]";
    char *tracer = "TracerPid:";
    char *interpreter = NULL;
    memset(option, 0, sizeof(option));
    strcat(option, "f:e:b:p:i:tsah");
    memset(digest, 0, sizeof(digest));
    sprintf(digest, "%064d", 0);

    j = 0;
    args[j++] = strdup(argv[0]);
    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (strlen(argv[i]) > 1) {
                for (k = 0; k < strlen(option); k++) {
                    if (option[k] == argv[i][1]) break;
                }
                args[j++] = strdup(argv[i]);
                argv[i] = NULL; 
                if (option[k + 1] == ':') {
                    i++;
                    if (i == argc) break;
                    args[j++] = strdup(argv[i]);
                    argv[i] = NULL;
                }
            } else {
                args[j++] = strdup(argv[i]);
                argv[i] = NULL;  
            }
        }
    }

    for (i = 1; i < argc; i++) {
        if (argv[i] != NULL) args[j++] = strdup(argv[i]);        
    }
  
    while ((opt = getopt(argc, args, option)) != -1) {
        switch (opt) {
            case 'h':
                printf("%s: Convert script into C code\n", argv[0]);
                printf("Usage: %s %s\n", argv[0], usage);
                printf("Option: \n");
                printf("    -t    Make traceable binary\n"); 
                printf("    -s    Using safe mode\n"); 
                printf("    -f    Fix arguments format\n");
                printf("    -e    Fix arguments 0 by external file\n");
                printf("    -p    Command parameter\n");
                printf("    -b    Operating system bits\n");
                printf("    -a    Anti modification interpreter\n");
                printf("    -i    Built in script interpreter\n");
                printf("    -h    Display help and return\n");
                return(0);
            case 't':
                trace_flag++;
                break;
            case 's':
                safe_flag++;
                break;    
            case 'f':
                fix_flag++;
                fix_format = strdup(optarg);
                break; 
            case 'e':
                file_flag++;
                file_name = strdup(optarg);
                break;
            case 'b':
                bit_flag++;
                bit = strdup(optarg);
                break;
            case 'p':
                para_flag++;
                parameter = strdup(optarg);
                break;
            case 'i':
                inner_flag++;
                interpreter = strdup(optarg);
                break;
            case 'a':
                anti_flag++;
                break;
            case '?':
                return(1);
            default:
                fprintf(stderr, "Usage: %s %s", argv[0], usage);
                return(1);   
        }
    }    

    for (i = optind; i < argc; i++) {
        if (!command_flag) {            
            command = strdup(args[i]);            
            command_flag = 1;    
        } else {           
            inname = strdup(args[i]);
            input_flag++; 
        }        
    }      

    if (input_flag != 1 || command_flag != 1 || fix_flag > 1 || trace_flag > 1 
        || safe_flag > 1 || file_flag > 1 || bit_flag > 1 || para_flag > 1
        || anti_flag > 1 || inner_flag > 1 
        || (bit_flag && strcmp(bit, "8") && strcmp(bit, "16") && strcmp(bit, "32") && strcmp(bit,"64"))) {
        fprintf(stderr, "Usage: %s %s\n", argv[0], usage);
        return(1);    
    } 
    
    if (inner_flag && anti_flag) {
        fprintf(stderr, "Usage: %s %s\n", argv[0], "Error: Only one of the options -a or -i can be selected");
        return(1); 
    }

    /*Running environment check*/
     
    if (uname(&sysinfo)) {
        perror("Failed to uname");
        return(1);
    }

    i = 0;
    if (!trace_flag) {
        while (sysname[i]) {
            if (!strcmp(sysinfo.sysname, sysname[i])) {
                fprintf(stderr, "Error: OS not support untraceable\n");
                return(1);
            }
              i++;
        }     
    }
    
    i = 0;
    if (fix_flag) {
        while (format[i]) {
            if (!strcmp(fix_format, format[i])) {
                fix_pos = i;
                break;
            } 
            i++;  
        } 
        if (fix_pos == -1) {
            fprintf(stderr, "Error: invalid fix format %s\n", fix_format);
            return(1);     
        }
    }  

    if (file_flag && access(file_name, R_OK)) {       
        fprintf(stderr, "Error: fix file %s is not exists\n", fix_format);
        return(1);          
    }

    length = strlen(getenv("PATH")) + strlen(command) + 1;
    fullname = malloc((size_t) length);
    if (inner_flag || anti_flag) {
        if(which(command, fullname)) {
            free(fullname);
            return(1); 
        } else if (file_sign((unsigned char *) fullname, (unsigned char *) digest)) {
            free(fullname);
            return(1); 
        }
    }

    /*Self file format*/
    if (access(self_name, R_OK) == 0) {
        self_file = fopen(self_name, "r");
        if (self_file == NULL) {
            fprintf(stderr, "Failed to open the file: %s\n", self_name);
            return(1);
        }
        while (fgets(str, sizeof(str), self_file)) {
            if (strstr(str, "TracerPid:") != NULL) {
                self_flag = 1;
                break;
            }
        }
        fclose(self_file);
    }
        
    in = fopen(inname, "r");
    if (in == NULL) {
        fprintf(stderr, "Failed to open the file: %s\n", inname);
        return(1);
    }
    stat(inname, &status);
    code_length = status.st_size + 32;

    code_text = (char *) malloc(code_length);
    memset(code_text, 0, (size_t) code_length);
    
    i = 0;
    for (j = 0; j < status.st_size; j++) {
        code_text[i++] = fgetc(in);
    }  
        
    srand(time(0));
    obscure_length = code_length / 5 * 6  + rand() % 1024; 
    bitmap = (char *) malloc(obscure_length * 2);
    obscure_text = (char *) malloc(obscure_length * 2);
    
    /*Randomly generated visible obfuscated characters*/
    loop = 1;
    while (loop) {
        j = 0;
        for (i = 0; i < code_length; j++) {
            digit = rand() % obscure_length;
            if (digit < code_length) {
                bitmap[j] = '1';
                obscure_text[j] = code_text[i++];
            } else {
                bitmap[j] = '0';
                digit = rand() % 3;
                obscure_text[j] = digit == 0 ? code_text[i] : rand() % 95 + 32;
            }
            if(j == obscure_length * 2) break;
        }
        if (i == code_length){
            loop = 0;
        }
    }
    /*Total length after confusion*/
    obscure_length = j;
 
    /*Effective location calculation factors*/
    for (i= 0; i < 32; i++) {
        for (j = 0; j < 16; j++) {
            x[i][j] = rand() % 10+1;
            y[i][j] = rand() % 10+1;
        }
    }

    /*Effective location calculation path*/
    for (i = 0; i < 32; i++) {
        k = rand() % 9 + 8;
        for (j = 0; j < k; j++) {
            route[i][j] = rand() % 32 + 1;
        }
        route[i][j]=0;
    }
    
    /*Character mask calculation path*/
    for (i = 0; i < 32; i++) {
        k = rand() % 9 + 8; 
        for (j = 0; j < k; j++){
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
        fprintf(stderr, "Failed to write to file %s\n", outname);
        return(1);
    }
    
    /*Write to the file header*/
    i = 0;
    while (head[i]) fprintf(out, "%s\n", head[i++]); 
    
    if(strcmp(sysinfo.sysname, "SCO_SV")) {
        fprintf(out, "#include <sys/ptrace.h>\n");
    }       
     
    /*Calculation path function*/
    for (i = 0; i < 32; i++) {
        j = 0;
        while (route[i][j] != 0) {
            if(j == 0){
                fprintf(out, "long f%d(long a, long b) \n{\n    return (\n", i + 1);
            }
            memset(str, 0, (size_t) sizeof(str));
            function(route[i][j], x[i][j], y[i][j], str);
            fprintf(out, "       %s%s\n", j == 0 ? " ":" +", str);
            j++;
        }
        fprintf(out, "    );\n}\n\n");
    }
    
    /*Character mask calculation function*/
    for (i = 0; i < 32; i++) {
        j = 0;
        while (algorithm[i][j] != 0) {
            if(j == 0){
                fprintf(out, "long f%d(long a, long b) \n{\n    return labs(\n", 32 + i + 1);
            }
            fprintf(out, "       %s(%s%c%s)\n", j == 0 ? " ":" +", number[i][j * 2], algorithm[i][j], number[i][j * 2 + 1]);
            j++;
        }
        fprintf(out, "    );\n}\n\n");
    }

    if (inner_flag || anti_flag) {        
        stat(fullname, &status);
        script_length = status.st_size;
        script = fopen(fullname, "rb");
        if (script == NULL) {
            fprintf(stderr, "Failed to open the file: %s\n", fullname);
            return(1);
        }   
    }

    fprintf(out, "static const unsigned char interpreter[] = {\n");    
    if (inner_flag) {         
        j = 0;
        while ((length = (int) fread(buffer, 1, sizeof(buffer), script))) {
            if(j != 0) {
                fprintf(out, ",\n");     
            }
            fprintf(out, "    ");
            for (i = 0; i < length; i++) {
                if (i != 0) {
                    fprintf(out, ", ");
                }
                fprintf(out, "0x%02X", (unsigned char) buffer[i]); 
                j++;               
            }  
        }
        fprintf(out, "\n");        
    } else {
        fprintf(out, "    0x0\n");     
    }
    fprintf(out, "};\n");

    if (inner_flag) {     
        fprintf(out, "static const char *command = \"%s\";\n", interpreter);  
    } else {
        fprintf(out, "static const char *command = \"%s\";\n", command);     
    }
    fprintf(out, "static const char *tracer = \"%s\";\n", tracer);    
    fprintf(out, "static const char *digest = \"%s\";\n", digest); 
    fprintf(out, "static const short inner = %d;\n", inner_flag); 
    fprintf(out, "static const short anti = %d;\n", anti_flag); 

    /*Write to the data section*/         
    fprintf(out, "static const char *data[] = {\n");
    if (bit_flag) {
        size = atoi(bit);
    } else {
        size = sizeof(long) * 8;
    }
    text = (char *) malloc(size);
    if (inner_flag) {
        salt = strlen(interpreter) + strlen(tracer) + strlen(digest) + 2;
        seed = (char *) malloc(salt + 16);
        memset(seed, 0, (size_t) (salt + 16));
        strcat(seed, interpreter);
    } else {
        salt = strlen(command) + strlen(tracer) + strlen(digest) + 2;
        seed = (char *) malloc(salt + 16);
        memset(seed, 0, (size_t) (salt + 16));
        strcat(seed, command);
    }
    strcat(seed, tracer);
    strcat(seed, digest);
    sprintf(seed + strlen(seed), "%d%d", inner_flag, anti_flag);
    j = 0;
    while (obscure_length) {
        mode = rand() % 32;
        length = rand() % 4 + size - 5;

        if (obscure_length < length) {
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
        while (route[mode][i] != 0) {
            switch (route[mode][i]) {
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
        
        for (i = 0; i < length; i++) {
            salt2 = 0;
            k = 0;
            pos = i % 32;
            while (algorithm[pos][k]) {
                switch (algorithm[pos][k]) {   
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
            if (inner_flag || anti_flag) {
                result = (script_length / (salt2 + 64) + script_length - salt1 + script_length / salt ) % script_length;
                if (fseek(script, result, SEEK_SET) != 0) {
                    perror("Failed to seeking interpreter file");
                    fclose(script);
                    return 1;
                }                              
                if (fread(&byte, sizeof(byte), 1, script) != 1) {
                    perror("Failed to reading interpreter file");
                    fclose(script);
                    return 1;
                }    
            } 
            fprintf(out, "%02X", ((unsigned char) text[i]) ^ ((int) salt1) ^ ((int) salt2) ^ ((unsigned char) seed[(salt1 + salt2) % salt]) ^ byte);
        }
        fprintf(out, "\",\n");
    }
    fprintf(out, "    0\n");
    fprintf(out, "};\n\n"); 
    if (inner_flag || anti_flag) fclose(script);
    i = 0;   
    while (func_def[i]) fprintf(out, "%s\n", func_def[i++]);

    if (safe_flag) {
        fprintf(out, "int write_script(FILE *pipe)\n");
    } else {
        fprintf(out, "int write_script(int file)\n");
    }

    i = 0;
    while (first[i]) fprintf(out, "%s\n", first[i++]);
    if (safe_flag) {
        fprintf(out, "                if (fwrite(str, sizeof(char), strlen(str), pipe) != strlen(str)) {\n");
        
    } else {
        fprintf(out, "                if (write(file, str, (size_t) strlen(str)) < 0) {\n");
    }

    i = 0;
    while (sencod[i]) fprintf(out, "%s\n", sencod[i++]);
    if (safe_flag) {
        fprintf(out, "            if (fwrite(str, sizeof(char), strlen(str), pipe) != strlen(str)) {\n");
    }else {
        fprintf(out, "            if (write(file, str, (size_t) strlen(str)) < 0) {\n");   
    }

    i  = 0;
    while (third[i]) fprintf(out, "%s\n", third[i++]); 

    i = 0;
    if (safe_flag) { 
        while (fourth_safe[i]) fprintf(out, "%s\n", fourth_safe[i++]);
        if (para_flag) {
            fprintf(out, "    length = strlen(name) + strlen(\"%s\") + 2;\n", parameter);
            fprintf(out, "    str = malloc(length);\n");
            fprintf(out, "    memset(str, 0, length);\n");
            fprintf(out, "    strcat(str, name);\n");
            fprintf(out, "    strcat(str, \" \");\n");
            fprintf(out, "    strcat(str, \"%s\");\n", parameter);           
        } else {
            fprintf(out, "    str = strdup(name);\n"); 
        }
        i = 0;
        while (fourth_end[i]) fprintf(out, "%s\n", fourth_end[i++]);            
    } else {        
        while (fourth[i]) fprintf(out, "%s\n", fourth[i++]);               
    }

    /*Write to the fix code section*/    
    if (fix_flag) {   
        i = 0;
        k = 0; 
        length = 0; 
        if (safe_flag) {
            if (fix_format != NULL && !strcmp(fix_format, "PHP")) {
                fprintf(out,  "    fwrite(\"<?php \", sizeof(char), 6, pipe);\n");    
            } else  if (fix_format != NULL && !strcmp(fix_format, "LUA")) {
                fprintf(out,  "    fwrite(\"arg = {}; \", sizeof(char), 10, pipe);\n");    
            }
        } else {
            if (fix_format != NULL && !strcmp(fix_format, "PHP")) {
                fprintf(out,  "    write(file[1], \"<?php \", 6);\n");    
            }
        }     
        while (fix_code[fix_pos][i]) {
            length += strlen(fix_code[fix_pos][i]);
            for (j = 0; j < strlen(fix_code[fix_pos][i]); j++) {
                if (fix_code[fix_pos][i][j] == '?') {
                    k++;        
                }
            }
            i++; 
        }
        fprintf(out, "    str = (char *) malloc(%d + strlen(argv[0]) * %d);\n", length + 8, k);            
        i = 0;
        while (fix_code[fix_pos][i]) {
            fprintf(out, "    sprintf(str, \"");  
            k = 0;             
            for (j = 0; j < strlen(fix_code[fix_pos][i]); j++) {
                if (fix_code[fix_pos][i][j] == '?') {
                    fputc('%', out);
                    fputc('s', out);
                    k++; 
                } else {
                    fputc(fix_code[fix_pos][i][j], out);
                }
            }
            fputc('"', out);
            for (j = 0; j < k; j++) {
                fprintf(out, ", argv[0]");
            }           
            fprintf(out, ");\n");
            if (safe_flag) {                
                fprintf(out,  "    fwrite(str, sizeof(char), strlen(str), pipe);\n");
            } else {                
                fprintf(out,  "    write(file[1], str, strlen(str));\n");
            }
            i++;                   
        }
        fprintf(out, "    free(str);\n");  
    }

    if (file_flag) {
        fix_file = fopen(file_name, "r");              
        if (fix_file == NULL) {
            fprintf(stderr, "Failed to open the fix file: %s\n", file_name);
            return(1);
        }
        stat(file_name, &status);
        length = status.st_size;
        k = 0;
        for (j = 0; j < status.st_size; j++) {
            if (fgetc(fix_file) == '?') {
                k++;        
            }
        }
        fprintf(out, "    str = (char *) malloc(%d + strlen(argv[0]) * %d);\n", length + 8, k);
        rewind(fix_file);
        while (fgets(str, sizeof(str), fix_file)) {
            fprintf(out, "    sprintf(str, \"");
            k = 0;
            for (j = 0; j < strlen(str) - 1; j++) {
                if (str[j] == '?') {
                    fputc('%', out);
                    fputc('s', out);  
                    k++;      
                } else {
                    fputc(str[j], out);
                }
            }
            fputc('"', out);
            for (j = 0; j < k; j++) {
                fprintf(out, ", argv[0]");
            }
            fprintf(out, ");\n");
            if (safe_flag) {
                fprintf(out,  "    fwrite(str, sizeof(char), strlen(str), pipe);\n");
            } else {
                fprintf(out,  "    write(file[1], str, strlen(str));\n");
            }
        }
        fclose(fix_file);
        fprintf(out, "    free(str);\n"); 
    }

    i = 0;
    if (safe_flag) { 
        if (!fix_flag || !arg_code[fix_pos][0]) {
            i = 0;
            while(sh_start[i]) fprintf(out, "%s\n", sh_start[i++]); 
            fprintf(out, "    write_script(pipe);\n");
            i = 0;
            while (sh_end[i]) fprintf(out, "%s\n", sh_end[i++]); 
        } else {              
            while (arg_start[i]) fprintf(out, "%s\n", arg_start[i++]); 
            i = 0;
            while (arg_code[fix_pos][i]) fprintf(out, "        %s\n", arg_code[fix_pos][i++]); 
            i = 0;
            while (arg_end[i]) fprintf(out, "%s\n", arg_end[i++]); 
            if (fix_format != NULL && !strcmp(fix_format, "PHP")) {
                fprintf(out,  "    fwrite(\"//\", sizeof(char), 2, pipe);\n");    
            }
            fprintf(out, "    write_script(pipe);\n");             
        }      
        i = 0;        
        while (fifth_safe[i]) fprintf(out, "%s\n", fifth_safe[i++]); 
    } else {     
        if (fix_format != NULL && !strcmp(fix_format, "PHP")) {
            fprintf(out,  "    write(file[1], \"//\", 2);\n");    
        }
        fprintf(out, "%s\n", "    j = 0;");
        if (para_flag) {
            fprintf(out, "%s\n", "    args[j++] = strdup(command);"); 
            fprintf(out, "    args[j++] = strdup(\"%s\");\n", parameter);     
        } else if (!strcmp(command, "node")) {
            fprintf(out, "%s\n", "    args[j++] = strdup(command);"); 
            fprintf(out, "%s\n", "    args[j++] = strdup(\"--preserve-symlinks-main\");"); 
        } else {
            fprintf(out, "%s\n", "    args[j++] = strdup(argv[0]);");
        }
        i = 0 ;   
        while (fifth[i]) fprintf(out, "%s\n", fifth[i++]);         
    }

    /*Write to the main function section*/
    i = 0;
    while (main_func[i]) fprintf(out, "%s\n", main_func[i++]);   

    i = 0;
    if (trace_flag) {      
        while (traced[i]) fprintf(out, "%s\n", traced[i++]);        
    } else if (self_flag) {        
        while (ptrace_self[i]) fprintf(out, "%s\n", ptrace_self[i++]);
        i = 0;
        while (traced[i]) fprintf(out, "%s\n", traced[i++]);  
        i = 0;
        while (ptrace_self_end[i]) fprintf(out, "%s\n", ptrace_self_end[i++]);  
    } else if (!strcmp(sysinfo.sysname, "AIX")) {
        while (ptrace_aix[i]) fprintf(out, "%s\n", ptrace_aix[i++]);  
        i = 0;
        while (traced[i]) fprintf(out, "%s\n", traced[i++]);  
        i = 0;   
        while (ptrace_aix_end[i]) fprintf(out, "%s\n", ptrace_aix_end[i++]);     
    } else {
        while (ptrace_unix[i]) fprintf(out, "%s\n", ptrace_unix[i++]);
        i = 0;
        while (traced[i]) fprintf(out, "%s\n", traced[i++]);  
        i = 0; 
        while (ptrace_unix_end[i]) fprintf(out, "%s\n", ptrace_unix_end[i++]);
    }
    
    i = 0;
    if (safe_flag || trace_flag || self_flag) {
        while (handler[i]) fprintf(out, "%s\n", handler[i++]);     
    } else {
        while (handler_fork[i]) fprintf(out, "%s\n", handler_fork[i++]);     
    }
   
    fflush(out);
    fclose(out);
    printf("Ok! Make binary by \"cc %s -O2 -o %s.x\"\n", outname, inname);
    free(seed);
    free(bitmap);
    free(code_text);
    free(obscure_text);
    free(text);
    free(outname);
    return(0);
}
