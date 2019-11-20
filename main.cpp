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
main()
{
    file SvgFile = {};
    if (!ReadFile("electronjs.svg", &SvgFile)) {
        printf("AAAAAAAAAAAA!\n");
        return;
    }

    // svg Svg = ParseSvg("filename")
    svg Svg = SvgParse(SvgFile.Data, SvgFile.Size);
}
