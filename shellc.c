/*
 * This code is released under
 * GNU GENERAL PUBLIC LICENSE Version 3
 * Function: Convert script into C code
 * Author: ChenZhongChao
 * Birthdate: 2023-12-25
 * Version: 1.8
 * Github: https://github.com/chenzhch/shellc.git
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <sys/utsname.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

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
#define SWAP(x) ((((x) & 0xFF000000) >> 24) | (((x) & 0x00FF0000) >> 8) | (((x) & 0x0000FF00) << 8) | (((x) & 0x000000FF) << 24))

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
    "#include <fcntl.h>",
    "#include <limits.h>",
    "#include <time.h>",
    0
};

static const char *first[] = {
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
    "#ifndef PATH_MAX",
    "#define PATH_MAX 4096",
    "#endif",
    ""
    "#define FF(x, y, z) ((x) ^ (y) ^ (z))",
    "#define GG(x, y, z) (((x) & (y)) | ((x) & (z)) | ((y) & (z)))",
    "#define HH(x, y, z) (((x) & (y)) | ((~(x)) & (z)))",
    "#define P0(x) ((x) ^ (LR((x), 9)) ^ (LR((x), 17)))",
    "#define P1(x) ((x) ^ (LR((x), 15)) ^ (LR((x), 23)))",
    "#define SWAP(x) ((((x) & 0xFF000000) >> 24) | (((x) & 0x00FF0000) >> 8) | (((x) & 0x0000FF00) << 8) | (((x) & 0x000000FF) << 24))",
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
    "void xfree(void *buf)",
    "{",
    "    if (buf != NULL) {",
    "        free(buf);", 
    "        buf = NULL;",
    "    }",   
    "}",
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
    "    char *name = NULL;",
    "    int i, j, len;",
    "    char *path = getenv(\"PATH\");",
    "    if (command[0] == '/' || (command[0] == '.')) {",
    "        if (access(command, X_OK) == 0) {",
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
    "                strcpy(fullname, name);",
    "                xfree(name);",
    "                return(0);",
    "            }",
    "            memset(name, 0,  (size_t) len);", 
    "            j = 0;",   
    "        } else {",
    "            name[j++] = path[i];",
    "        }",
    "    }", 
    "    xfree(name);",
    "    fprintf(stderr, \"Error: command %s not found\\n\", command);",
    "    return(1);",
    "}",
    "",
    "int write_script(int file)",
    "{",
    "    int i, j;",
    "    long mask, salt1, salt2, salt, length, script_length = 0;",
    "    char chr[3], *seed = NULL, summary[65];",
    "    char fullname[PATH_MAX];",
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
    "    memset(fullname, 0, sizeof(fullname));",
    "    i = 0;",
    "    salt = strlen(command) + strlen(date_str) + strlen(message) + strlen(digest) + 2;",
    "    seed = (char *) malloc((size_t) (salt + 16));",
    "    memset(seed, 0, salt + 16);",
    "    strcat(seed, command);",
    "    strcat(seed, date_str);",
    "    strcat(seed, message);",
    "    if (inner || anti) {",
    "        if (which(command, fullname)) {",
    "            xfree(seed);",
    "            return(1);",
    "        }",
    "        if(file_sign((unsigned char *) fullname, (unsigned char *) summary)) {",
    "            xfree(seed);",
    "            return(1);",
    "        }",
    "        stat(fullname, &status);", 
    "        script_length = status.st_size;",  
    "        script = fopen(fullname, \"rb\");",
    "        if (script == NULL) {",
    "             fprintf(stderr, \"Failed to open the file: %s\\n\", fullname);",
    "             xfree(seed);",
    "             return(1);",
    "        }",  
    "    } else {",
    "        strcpy(summary, digest);",
    "    }",
    "    strcat(seed, summary);",
    "    sprintf(seed + strlen(seed), \"%d%d\", inner, anti);",
    "    memset(chr, 0, (size_t) sizeof(chr));",
    "    while (data[i]) {",
    "        mask = (long) 1 << (atol(data[i + 1]) - 1 + rl.rlim_cur);",    
    "        for (j = 0; j < atol(data[i + 1]); j++) {",
    "            byte = 0x0;",            
    "            memcpy(chr, &data[i + 5][j * 2], 2);",   
    "            salt1 = (atol(data[i + 2]) + ff(fn[atol(data[i]) -1], atol(data[i + 3]), atol(data[i + 4]))) % 256;",
    "            salt2 = ff(fn[j % 32 + 32], atol(data[i + 3]), atol(data[i + 4])) % 256;", 
    "            if (inner || anti) {",
    "                length = (script_length / (salt2 + 64) + script_length - salt1 + script_length / salt ) % script_length + rl.rlim_cur;",
    "                if (fseek(script, length, SEEK_SET) != 0) {",
    "                    perror(\"Failed to seeking interpreter file\");",
    "                    fclose(script);",
    "                    xfree(seed);",
    "                    return(1);",
    "                }",                              
    "                if (fread(&byte, sizeof(byte), 1, script) != 1) {",
    "                    perror(\"Failed to reading interpreter file\");",
    "                    fclose(script);",
    "                    xfree(seed);",    
    "                    return(1);",
    "                }", 
    "            }",
    "            if ((atol(data[i + 2]) + ff(fn[atol(data[i]) -1], atol(data[i + 3]), atol(data[i + 4])) + rl.rlim_cur) & mask) {",
    "                byte ^= (int) strtol(chr, 0, 16) ^ (int) salt1 ^ (int) salt2 ^ ((unsigned char) seed[(salt1 + salt2) % salt]);",
    "                byte &= 0xFF;",
    "                if(xbash || byte) {",
    "                    if (write(file, &byte, 1) == -1) {",
    "                        perror(\"Failed to write file\");",
    "                        fclose(script);",
    "                        xfree(seed);",    
    "                        return(1);",
    "                    }",
    "                }",
    "            }",
    "            mask >>= 1;",
    "       }",
    "       i += 6;",
    "    }",
    "    xfree(seed);",
    "    if (inner || anti) fclose(script);",
    "    return(0);",
    "}",
    "",
    "int handler(int argc, char **argv)",
    "{",
    "    char *dev = NULL, *name = NULL, *cwd = NULL, real[PATH_MAX], fullname[PATH_MAX];",
    "    int file, i, length;",
    "    pid_t pid;",
    "    int j, status;",
    "    char **args = NULL;",
    "    unsigned char summary[65];",
    "    FILE *in = NULL;",
    "    struct tm *timeinfo;",
    "    struct rlimit rl;",
    "    time_t rawtime;", 
    "    char nowdate[9];",
    "    time(&rawtime);",
    "    timeinfo = localtime(&rawtime);",
    "    memset(nowdate, 0, sizeof(nowdate));",
    "    strftime(nowdate, sizeof(nowdate), \"%Y%m%d\", timeinfo);", 
    "    if (strcmp(nowdate, date_str) > 0) {",
    "        printf(\"%s\\n\", message);",
    "        return(1);",
    "    }",
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
    "    if (which(command, fullname)) {",
    "        return(1);",
    "    }",       
    "    if (inner || anti) {",
    "        memset(summary, 0, sizeof(summary));",
    "        if (file_sign((unsigned char *) fullname, (unsigned char *) summary)) {",
    "            return(1);",
    "        }",
    "        if (strcmp((char *) summary, (char *) digest)) {",
    "            fprintf(stderr, \"Error: invalid interpreter %s\\n\", fullname);",
    "            return(1);",
    "        }",
    "    }",    
    "    srand(time(0));",
    "    length = rand() % 8 + 16;",
    "    name = malloc((size_t) length);",
    "    memset(name, 0, length);",
    "    for (i = 0; i < length - 8; i++) {",
    "        switch (rand() % 2) {",
    "            case 0:",
    "                sprintf(name + i, \"%c\", rand() % 26 + 65);",
    "                break;",
    "            case 1:",
    "                sprintf(name + i, \"%c\", rand() % 26 + 97);",
    "                break;",
    "        }",
    "    } ",
    "    sprintf(name + i, \"%d\", rand()%100);",
    "    if (xbash) strcat(name, \"X\");",
    "    if (access(\"/tmp\", R_OK)) {",
    "        if(realpath(argv[0], real) == NULL) {",
    "            printf(\"Failed to realpath\\n\");",
    "            return(1);",
    "	     }",
    "        length = strlen(real);",
    "        dev = (char *) malloc((size_t) (length + 64));",
    "        cwd = (char *) malloc((size_t) (length));",
    "        memset(dev, 0, length + 64);",
    "        memset(cwd, 0, length);",
    "        memcpy(cwd, real, strlen(real) - strlen(strrchr(real, '/')));",
    "        sprintf(dev, \"%s/%s\", cwd, name);",
    "        xfree(cwd);",
    "    } else {",
    "        dev = (char *) malloc((size_t) 64);",
    "        memset(dev, 0, 64);",
    "        sprintf(dev, \"/tmp/%s\", name);",
    "    }",
    "    xfree(name);",
    "    if(mkfifo(dev, S_IRUSR | S_IWUSR)) {",
    "        perror(\"Failed to mkfifo\");",
    "        return(1);    ",
    "    }",
    "    if ((pid = fork()) == 0) { ",
    "        file = open(dev, O_WRONLY);",
    "        if(unlink(dev))  {",
    "            perror(\"Failed to unlink\");",
    "            return(1);    ",
    "        }",
    0
};

static const char *second[] = {
    "        close(file);",
    "        _exit(0);",
    "    } else if (pid < 0) {",
    "        perror(\"Failed to fork\");",
    "        return(1);",
    "    }",
    "    args = (char **) malloc((argc + 8) * sizeof(char *));",
    "    j = 0;",
    0
};

static const char *third[] = {    
    "    args[j++] = strdup(dev);",
    "    xfree(dev);",
    "    for (i = 1; i < argc; i++) {",
    "        args[j++] = strdup(argv[i]);",
    "    }",
    "    args[j] = 0;",
    "    execvp(command, args);",
    "    perror(\"Failed to execvp\");",
    "    wait(&status);",
    "    return(1);",
    "}",
    "",
    "typedef struct {",
    "    int argc;",
    "    char **argv;",
    "} Param;",
    "",
    "int process(void *arg)", 
    "{",
    "    Param *param = (Param *) arg;",
    "    return(handler(param->argc, param->argv));",
    "}",
    "",
    0
};


static const char *ptrace_sco[] = {
    "int main(int argc, char **argv)",
    "{",
    "    pid_t pid = 0;",
    "    int status;",
    "    if(ptrace(0, 0, 0, 0)) {",
    "        return(1);",        
    "    }",
    "    if ((pid = fork()) == 0) {", 
    "        return(handler(argc, argv));",
    "    } else if (pid < 0) {",
    "        perror(\"Failed to fork\");",
    "        return(1);",
    "    }",
    "    waitpid(pid, &status, 0);",
    "    return(WEXITSTATUS(status));", 
    "}",
    0
};

static const char *ptrace_aix[] = {
    "int main(int argc, char **argv)",
    "{",
    "    pid_t pid = 0;",
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
    "    return(handler(argc, argv));",
    "}",
    0
};

static const char *ptrace_linux[] = {
    "int main(int argc, char **argv)",
    "{",
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
    "int main(int argc, char **argv)",
    "{",
    "    return(handler(argc, argv));",
    "}",
    0 
};

void xfree(void *buf)
{
    if (buf != NULL) {
        free(buf); 
        buf = NULL;
    }   
}

/*Which command*/
int which(const char *command, char *fullname)
{
    char *name = NULL;
    int i, j, len;
    char *path = getenv("PATH");
    if (command[0] == '/' || (command[0] == '.')) {
        if (access(command, X_OK) == 0) {
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
                strcpy(fullname, name);                
                xfree(name);
                return(0);
            }
            memset(name, 0,  (size_t) len);
            j = 0;
        } else {
            name[j++] = path[i];
        }
    }
    xfree(name);
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

void free_args(char **args, int len)
{
    int i;
    if(len == 0) return;
    for(i = 0; i < len; i++) {
        xfree(args[i]);
    }
    xfree(args);
}

int main(int argc, char **argv)
{
    FILE *in, *out, *fix_file, *script = NULL;
    int code_length, obscure_length, length = 0, pos;
    int fix_pos = -1;
    char *code_text = NULL, *obscure_text = NULL, *text = NULL;
    char *bitmap = NULL, *inname = NULL, *outname = NULL, *command = NULL, *parameter = NULL;
    char *fix_format = NULL, *file_name = NULL, *bit = NULL, fullname[PATH_MAX];
    char *date_str = NULL, *message = NULL;
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
    int trace_flag = 0, fix_flag = 0, input_flag = 0, command_flag = 0, file_flag = 0, xbash_flag = 0;   
    int bit_flag = 0, date_flag = 0, message_flag = 0, para_flag = 0, anti_flag = 0, inner_flag = 0;   
    struct utsname sysinfo;
    struct stat status; 
    char option[32];
    char *seed = NULL;
    char **args = NULL;
    char *usage = "command inputfile [-t] [-a] [-x] [-f fix-format] [-e fix-file] [-p parameter] [-i interpreter] [-b 8|16|32|64] [-d YYYYMMDD] [-m message]";
    char *interpreter = NULL;
    
    memset(option, 0, sizeof(option));
    memset(fullname, 0, sizeof(fullname));
    strcat(option, "f:e:p:i:b:d:m:taxh");
    memset(digest, 0, sizeof(digest));
    sprintf(digest, "%064d", 0);     
    
    if(argc > strlen(option)) {
        fprintf(stderr, "Usage: %s %s\n", argv[0], usage);
        return(1);        
    }
    args = (char **) malloc((argc) * sizeof(char *));
    if(args == NULL) {
        fprintf(stderr, "malloc error\n");
        return(1); 
    }
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
                printf("    -x    Script compiled by xbash\n");
                printf("    -f    Fix arguments format\n");
                printf("    -e    Fix arguments 0 by external file\n");
                printf("    -p    Command parameter\n");
                printf("    -a    Anti modification interpreter\n");
                printf("    -b    Operating system bits\n");
                printf("    -i    Built in script interpreter\n");
                printf("    -d    Expiration date in YYYYMMDD format\n");
                printf("    -m    Expiration date message, default 'The program has expired'\n");
                printf("    -h    Display help and return\n");
                goto finish;
            case 't':
                trace_flag++;
                break;
            case 'x':
                xbash_flag++;
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
            case 'd':
                date_flag++;
                date_str = strdup(optarg);
                break;
            case 'm':
                message_flag++;
                message = strdup(optarg);
                break;
            case '?':
                goto finish;
            default:
                fprintf(stderr, "Usage: %s %s", argv[0], usage);
                goto finish;   
        }
    }    

    for (i = optind; i < argc; i++) {
        if (!command_flag) {            
            command = strdup(args[i]);            
            command_flag = 1;    
        } else {
            if(!input_flag) inname = strdup(args[i]);
            input_flag++; 
        }        
    }      
    if (input_flag != 1 || command_flag != 1 || fix_flag > 1 || trace_flag > 1 
        || file_flag > 1 || bit_flag > 1 || para_flag > 1 || anti_flag > 1 
        || inner_flag > 1 || date_flag > 1 || message_flag > 1 || xbash_flag > 1 
        || (bit_flag && strcmp(bit, "8") && strcmp(bit, "16") && strcmp(bit, "32") && strcmp(bit,"64"))) {
        fprintf(stderr, "Usage: %s %s\n", argv[0], usage);
        goto finish;    
    } 
    
    if(date_flag) {
        if (strlen(date_str) != 8) {
            fprintf(stderr, "Error: invalid date format\n");
            goto finish;
        }
        for (i = 0; i < strlen(date_str); i++) {
            if (date_str[i] < '0' || date_str[i] > '9') {
                fprintf(stderr, "Error: invalid date format\n");
                goto finish;   
            }
        }
    }
    
    if (message_flag && !date_flag) {
        fprintf(stderr, "Error: option -m must have -d\n");
        goto finish;      
    }
    
    if (inner_flag && anti_flag) {
        fprintf(stderr, "Error: only one of the options -a or -i can be selected\n");
        goto finish; 
    }
    
    /*Running environment check*/
     
    if (uname(&sysinfo)) {
        perror("Failed to uname");
        goto finish;
    }

    i = 0;
    if (!trace_flag) {
        while (sysname[i]) {
            if (!strcmp(sysinfo.sysname, sysname[i])) {
                fprintf(stderr, "Error: OS not support untraceable\n");
                goto finish;
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
            goto finish;     
        }
    }  

    if (file_flag && access(file_name, R_OK)) {       
        fprintf(stderr, "Error: fix file %s is not exists\n", fix_format); 
        goto finish;          
    }

    length = strlen(getenv("PATH")) + strlen(command) + 1;
    if (inner_flag || anti_flag) {
        if(which(command, fullname)) {
            goto finish; 
        } else if (file_sign((unsigned char *) fullname, (unsigned char *) digest)) {
            goto finish; 
        }
    }
        
    in = fopen(inname, "r");
    if (in == NULL) { 
        fprintf(stderr, "Failed to open the file: %s\n", inname);
        goto finish;;
    }
    stat(inname, &status);
    code_length = status.st_size + 32;

    code_text = (char *) malloc(code_length);
    memset(code_text, 0, (size_t) code_length);
    i = 0;
    for (j = 0; j < status.st_size; j++) {
        code_text[i++] = fgetc(in);
    }  
    
    if (xbash_flag 
        && (status.st_size < 8 || (code_text[0] & 0xFF) != 0xEF || (code_text[1] & 0xFF) != 0x97 
            || (code_text[2] & 0xFF) != 0xFB || (code_text[3] & 0xFF) != 0x03 || (code_text[4] & 0xFF) != 0xC7 
            || (code_text[5] & 0xFF) != 0x65 || (code_text[6] & 0xFF) != 0xA8 || (code_text[7] & 0xFF) != 0x03)) {
        fclose(in);    
        fprintf(stderr, "Error: %s is not in xbash format\n", inname);
        goto finish;
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
        goto finish;
    }
    
    /*Write to the file header*/
    i = 0;
    while (head[i]) fprintf(out, "%s\n", head[i++]); 
    
    if (!trace_flag && strcmp(sysinfo.sysname, "SCO_SV")) {
        fprintf(out, "#include <pthread.h>\n");
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
            goto finish;
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
    fprintf(out, "static const short xbash = %d;\n", xbash_flag);    
    fprintf(out, "static const char *digest = \"%s\";\n", digest); 
    fprintf(out, "static const short inner = %d;\n", inner_flag); 
    fprintf(out, "static const short anti = %d;\n", anti_flag);
    if (date_flag) {
        fprintf(out, "static const char *date_str = \"%s\";\n", date_str);    
    } else {
        date_str = strdup("99991231");
        fprintf(out, "static const char *date_str = \"%s\";\n", "99991231");     
    }
    if (message_flag) {
        fprintf(out, "static const char *message = \"%s\";\n", message);    
    } else {
        message = strdup("The program has expired");
        fprintf(out, "static const char *message = \"%s\";\n", "The program has expired");     
    }

    /*Write to the data section*/         
    fprintf(out, "static const char *data[] = {\n");
    if (bit_flag) {
        size = atoi(bit);
    } else {
        size = 32;
    }
    text = (char *) malloc(size);
    if (inner_flag) {
        salt = strlen(interpreter) + strlen(date_str) + strlen(message) + strlen(digest) + 2;
        seed = (char *) malloc(salt + 16);
        memset(seed, 0, (size_t) (salt + 16));
        strcat(seed, interpreter);
    } else {
        salt = strlen(command) + strlen(date_str) + strlen(message) + strlen(digest) + 2;
        seed = (char *) malloc(salt + 16);
        memset(seed, 0, (size_t) (salt + 16));
        strcat(seed, command);
    }
    strcat(seed, date_str);
    strcat(seed, message);
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
                    goto finish;
                }                              
                if (fread(&byte, sizeof(byte), 1, script) != 1) {
                    perror("Failed to reading interpreter file");
                    fclose(script);
                    goto finish;
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
    while (first[i]) fprintf(out, "%s\n", first[i++]);

    /*Write to the fix code section*/    
    if (fix_flag) {   
        i = 0;
        k = 0; 
        length = 0; 
        
        if (fix_format != NULL && !strcmp(fix_format, "PHP")) {
            fprintf(out,  "        write(file, \"<?php \", 6);\n");    
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
        fprintf(out, "        name = (char *) malloc(%d + strlen(argv[0]) * %d);\n", length + 8, k);            
        i = 0;
        while (fix_code[fix_pos][i]) {
            fprintf(out, "        sprintf(name, \"");  
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
                           
            fprintf(out,  "        write(file, name, strlen(name));\n");
            i++;                   
        }
        fprintf(out, "        xfree(name);\n");  
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
        fprintf(out, "    name = (char *) malloc(%d + strlen(argv[0]) * %d);\n", length + 8, k);
        rewind(fix_file);
        while (fgets(str, sizeof(str), fix_file)) {
            fprintf(out, "    sprintf(name, \"");
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
            
             fprintf(out,  "    write(file, name, strlen(name));\n");
            
        }
        fclose(fix_file);
        fprintf(out, "    xfree(name);\n"); 
    }
    
    if (fix_format != NULL && !strcmp(fix_format, "PHP")) {
        fprintf(out, "        write(file, \"//\", 2);\n");    
    }  
     
    fprintf(out, "        write_script(file);\n");  
    
    i = 0;
    while (second[i]) fprintf(out, "%s\n", second[i++]); 
    
    if (para_flag) {
        fprintf(out, "%s\n", "    args[j++] = strdup(command);"); 
        fprintf(out, "    args[j++] = strdup(\"%s\");\n", parameter);     
    } else if (!strcmp(command, "node")) {
        fprintf(out, "%s\n", "    args[j++] = strdup(command);"); 
        fprintf(out, "%s\n", "    args[j++] = strdup(\"--preserve-symlinks-main\");"); 
    } else {
        fprintf(out, "%s\n", "    args[j++] = strdup(argv[0]);");
    }
    
    i = 0;
    while (third[i]) fprintf(out, "%s\n", third[i++]);         

    /*Write to the main function section*/
    i = 0;
    if (trace_flag) {      
        while (traced[i]) fprintf(out, "%s\n", traced[i++]);        
    } else if (!strcmp(sysinfo.sysname, "SCO_SV")){
        while (ptrace_sco[i]) fprintf(out, "%s\n", ptrace_sco[i++]);          
    } else if (!strcmp(sysinfo.sysname, "AIX")) {
        while(ptrace_aix[i]) fprintf(out, "%s\n", ptrace_aix[i++]);        
    } else {
        while (ptrace_linux[i]) fprintf(out, "%s\n", ptrace_linux[i++]); 
        trace_flag = 2;            
    }
    fflush(out);
    fclose(out);
    
    printf("Ok! You can make binary by \"cc %s -s -O2 -o %s.x%s\"\n", outname, inname, trace_flag == 2 ? " -lpthread" : "");   
         
finish:
    free_args(args, argc);
    xfree(code_text);
    xfree(obscure_text);
    xfree(fix_format);
    xfree(file_name);
    xfree(bit);
    xfree(parameter);
    xfree(interpreter);
    xfree(date_str);  
    xfree(message); 
    xfree(bitmap);
    xfree(text);
    xfree(inname);
    xfree(outname);
    xfree(seed);
    xfree(command);
    return(0);
}
