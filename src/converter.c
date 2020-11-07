#include <string.h>

#include "return_codes.h"
#include "message_handler.h"
#include "str_ending_comparer.h"
#include "bmp_handler.h"
#include "qdbmp.h"

#define CHECK_ERROR(return_code) if (return_code != success) return return_code;

#define MAX_FILENAME_LEN 50
#define FILE_FORMAT ".bmp"
#define ESTIMATED_CL_ARGS_NUM 4

struct conv_options_t {
    enum author {empty, mine, theirs} author;
    _Bool inp_file_given;
    _Bool otp_file_given;
    char inp_filename[MAX_FILENAME_LEN];
    char otp_filename[MAX_FILENAME_LEN];
};

enum return_codes_t parse_conv_options(struct conv_options_t* const options_container, const int* const argc, const char* const* const argv) {
    if (*argc != ESTIMATED_CL_ARGS_NUM) {
        message_handle("Unexpected number of command-line arguments\n", error);
        return runtime_error;
    }
    options_container->author = empty;
    options_container->inp_file_given = 0;
    options_container->otp_file_given = 0;
    for (unsigned int i = 1; i < ESTIMATED_CL_ARGS_NUM; i++) {
        if (strcmp(argv[i], "--mine") == 0 && options_container->author == empty) {
            options_container->author = mine;
        } else if (strcmp(argv[i], "--theirs") == 0 && options_container->author == empty) {
            options_container->author = theirs;
        } else if (is_str_ending(argv[i], FILE_FORMAT) && !options_container->inp_file_given) {
            options_container->inp_file_given = 1;
            strcpy(options_container->inp_filename, argv[i]);
        } else if (is_str_ending(argv[i], FILE_FORMAT) && !options_container->otp_file_given) {
            options_container->otp_file_given = 1;
            strcpy(options_container->otp_filename, argv[i]);
        } else {
            message_handle("Unknown command-line argument found\n", error);
            return runtime_error;
        }
    }
    return success;
}

void invert_bmp_8bpp(BYTE* const palette_container, const DWORD* const colors_num) {
    for (unsigned int i = 0; i < *colors_num; i++) {
        palette_container[i * sizeof(DWORD)] = ~palette_container[i * sizeof(DWORD)];
        palette_container[i * sizeof(DWORD) + 1] = ~palette_container[i * sizeof(DWORD) + 1];
        palette_container[i * sizeof(DWORD) + 2] = ~palette_container[i * sizeof(DWORD) + 2];
    }
}

void invert_bmp_24bpp(BYTE* const pixels_container, const struct bmp_header_t* const header_container) {
    const unsigned int bytes_per_row = header_container->bitmap_size / abs(header_container->height), bytes_per_pixel = BPP_24BIT / BITS_IN_BYTE_NUM;
    for (unsigned int i = 0; i < (unsigned) abs(header_container->height); i++) {
        for (unsigned int j = 0; j < (unsigned long) header_container->width; j++) {
            BYTE* const pxl = pixels_container + i * bytes_per_row + j * bytes_per_pixel;
            pxl[0] = ~pxl[0];
            pxl[1] = ~pxl[1];
            pxl[2] = ~pxl[2];
        }
    }
}

int main(const int argc, const char* const* const argv) {
    enum return_codes_t return_code;
    struct conv_options_t conv_options;
    return_code = parse_conv_options(&conv_options, &argc, argv);
    CHECK_ERROR(return_code)
    if (conv_options.author == mine) {
        message_handle("Conversion will be performed using self-written methods\n", report);
        struct bmp_file_t bmp_input;
        return_code = read_bmp_file(&bmp_input, conv_options.inp_filename);
        CHECK_ERROR(return_code)
        if (bmp_input.header.bpp == BPP_8BIT) {
            invert_bmp_8bpp(bmp_input.palette, &bmp_input.header.colors_num);
        } else {
            invert_bmp_24bpp(bmp_input.bitmap, &bmp_input.header);
        }
        return_code = write_bmp_file(&bmp_input, conv_options.otp_filename);
        dealloc_bmp_container(&bmp_input);
        CHECK_ERROR(return_code)
    } else {
        message_handle("Conversion will be performed using QDBMP\n", report);
        BMP* const bmp_input = BMP_ReadFile(conv_options.inp_filename);
        BMP_CHECK_ERROR(stderr, qdbmp_runtime_error)
        if (bmp_input->Header.BitsPerPixel == BPP_8BIT) {
            UCHAR red, green, blue;
            for (unsigned int i = 0; i < bmp_input->Header.ColorsUsed; i++) {
                BMP_GetPaletteColor(bmp_input, i, &red, &green, &blue);
                BMP_CHECK_ERROR(stderr, qdbmp_runtime_error)
                BMP_SetPaletteColor(bmp_input, i, ~red, ~green, ~blue);
                BMP_CHECK_ERROR(stderr, qdbmp_runtime_error)
            }
        } else if (bmp_input->Header.BitsPerPixel == BPP_24BIT) {
            UCHAR red, green, blue;
            for (unsigned int i = 0; i < bmp_input->Header.Height; i++) {
                for (unsigned int j = 0; j < bmp_input->Header.Width; j++) {
                    BMP_GetPixelRGB(bmp_input, j, i, &red, &green, &blue);
                    BMP_CHECK_ERROR(stderr, qdbmp_runtime_error)
                    BMP_SetPixelRGB(bmp_input, j, i, ~red, ~green, ~blue);
                    BMP_CHECK_ERROR(stderr, qdbmp_runtime_error)
                }
            }
        } else {
            message_handle("Unsupported file type (unsupported bpp, only 8 and 24 are supported)\n", error);
            return runtime_error;
        }
        BMP_WriteFile(bmp_input, conv_options.otp_filename);
        BMP_CHECK_ERROR(stderr, qdbmp_runtime_error)
        BMP_Free(bmp_input);
    }
    message_handle("Done!\n", report);
    return success;
}
