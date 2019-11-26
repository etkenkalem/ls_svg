#include "stdio.h"
#include "stdint.h"
#include "malloc.h"
#include "assert.h"
#include "stdarg.h"

#define global_variable static
#define internal static
#define ArrayCount(Array) ((sizeof(Array)) / (sizeof(Array[0])))
#define Assert(Expression) if(!(Expression)) {*(int *)0=0;}

typedef char utf8;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef s32 b32;
typedef float r32;
typedef double r64;

#define LS_STRING_IMPLEMENTATION
#include "ls_string.h"
#include "ls_svg.h"

struct file {
    u8 *Data;
    u32 Size;
};

b32
ReadFile(char *Name, file *File_out)
{
    file File = {};

    FILE *F = fopen(Name, "rb");

    if (!F) {
        return false;
    }

    fseek(F, 0, SEEK_END);
    File.Size = ftell(F);
    fseek(F, 0, SEEK_SET);

    File.Data = (u8 *)malloc(File.Size);
    fread(File.Data, 1, File.Size, F);

    *File_out = File;

    return true;
}

void
Svg()
{
    file SvgFile = {};
    if (!ReadFile("electronjs.svg", &SvgFile)) {
        printf("AAAAAAAAAAAA!\n");
        return;
    }

    // svg Svg = ParseSvg("filename")
    svg Svg = SvgParse(SvgFile.Data, SvgFile.Size);

    u32 a =4;
}

// void
// Test()
// {
//     file File = {};
//     if (!ReadFile("test.txt", &File)) {
//         printf("AAAAAAAAAAAA!\n");
//         return;
//     }

//     r32 a = 1.0f;
//     a = 2.0f;
//     a = 3.0f;
//     a = 4.0f;
//     a = 5.0f;
//     a = 6.0f;
//     a = 7.0f;
//     a = -2.0f;

//     r32 t = -3.9;

//     u32 b = (u32)a;

//     ls_parser P((char *)File.Data, File.Size);
//     token T = P.GetToken();
// }

void
main()
{
    // Test();
    Svg();

    u32 a = 4;
}
