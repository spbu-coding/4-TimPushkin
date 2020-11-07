#ifndef BMP_HANDLER_H
#define BMP_HANDLER_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include "return_codes.h"
#include "message_handler.h"

#define BITMAP_SIZE_FORMULA(width, height, bpp) ((width * bpp + sizeof(DWORD) * BITS_IN_BYTE_NUM - 1) / (sizeof(DWORD) * BITS_IN_BYTE_NUM)) * sizeof(DWORD)  * abs(height)

#define BITS_IN_BYTE_NUM 8u
#define BMP_SIGN 0x4d42
#define FULL_HEADER_SIZE 54
#define DIB_HEADER_SIZE 40
#define PLANES_NUM 1
#define BPP_8BIT 8
#define BPP_24BIT 24
#define MAX_8BIT_PALETTE_SIZE 256

#ifndef SEEK_SET
#define SEEK_SET 0
#endif
#ifndef SEEK_END
#define SEEK_END 2
#endif

typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef int32_t LONG;

struct bmp_header_t {
    WORD file_type;
    DWORD file_size;
    WORD reserved1;
    WORD reserved2;
    DWORD bitmap_offset;
    DWORD dib_header_size;
    LONG width;
    LONG height;
    WORD planes;
    WORD bpp;
    DWORD compression;
    DWORD bitmap_size;
    LONG horz_res;
    LONG vert_res;
    DWORD colors_num;
    DWORD colors_req;
};

struct bmp_file_t {
    struct bmp_header_t header;
    BYTE* palette;
    BYTE* bitmap;
};

enum return_codes_t read_bmp_file(struct bmp_file_t* bmp_container, const char filename[]);

enum return_codes_t write_bmp_file(struct bmp_file_t* bmp_container, const char filename[]);

void dealloc_bmp_container(struct bmp_file_t* bmp_container);

#endif
