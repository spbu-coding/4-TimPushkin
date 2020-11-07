#include <stdio.h>

#include "return_codes.h"
#include "message_handler.h"
#include "str_ending_comparer.h"
#include "bmp_handler.h"

#define CMP_8BPP_PXLS(plt1, plt2, pxl1, pxl2) (plt1[*pxl1 * sizeof(DWORD)] == plt2[*pxl2 * sizeof(DWORD)] && plt1[*pxl1 * sizeof(DWORD) + 1] == plt2[*pxl2 * sizeof(DWORD) + 1] && plt1[*pxl1 * sizeof(DWORD) + 2] == plt2[*pxl2 * sizeof(DWORD) + 2])
#define CMP_24BPP_PXLS(pxl1, pxl2) (pxl1[0] == pxl2[0] && pxl1[1] == pxl2[1] && pxl1[2] == pxl2[2])
#define CMP_MIXED_PXLS(plt1, pxl1, pxl2) (plt1[*pxl1 * sizeof(DWORD)] == pxl2[0] && plt1[*pxl1 * sizeof(DWORD) + 1] == pxl2[1] && plt1[*pxl1 * sizeof(DWORD) + 2] == pxl2[2])

#define MAX_FILENAME_LEN 50
#define FILE_FORMAT ".bmp"
#define ESTIMATED_CL_ARGS_NUM 3

struct comp_options_t {
    _Bool file1_given;
    _Bool file2_given;
    char filename1[MAX_FILENAME_LEN];
    char filename2[MAX_FILENAME_LEN];
};

enum return_codes_t parse_comp_options(struct comp_options_t* const options_container, const int* const argc, const char* const* const argv) {
    if (*argc != ESTIMATED_CL_ARGS_NUM) {
        message_handle("Unexpected number of command-line arguments\n", error);
        return runtime_error;
    }
    options_container->file1_given = 0;
    options_container->file2_given = 0;
    for (unsigned int i = 1; i < ESTIMATED_CL_ARGS_NUM; i++) {
        if (is_str_ending(argv[i], FILE_FORMAT) && !options_container->file1_given) {
            options_container->file1_given = 1;
            strcpy(options_container->filename1, argv[i]);
        } else if (is_str_ending(argv[i], FILE_FORMAT) && !options_container->file2_given) {
            options_container->file2_given = 1;
            strcpy(options_container->filename2, argv[i]);
        } else {
            message_handle("Unknown command-line argument found\n", error);
            return runtime_error;
        }
    }
    return success;
}

void compare_bmp_pixels(const struct bmp_file_t* bmp1, const struct bmp_file_t* bmp2) {
    const unsigned int bytes_per_row_1 = bmp1->header.bitmap_size / abs(bmp1->header.height), bytes_per_row_2 = bmp2->header.bitmap_size / abs(bmp2->header.height),
        bytes_per_pxl_1 = bmp1->header.bpp / BITS_IN_BYTE_NUM, bytes_per_pxl_2 = bmp2->header.bpp / BITS_IN_BYTE_NUM;
    unsigned int diff_count = 0;
    const _Bool equal_height = (bmp1->header.height == bmp2->header.height);
    int vert_loop_start = 0, vert_loop_finish = abs(bmp1->header.height);
    if (equal_height && bmp1->header.height < 0) {
        vert_loop_start = bmp1->header.height + 1;
        vert_loop_finish = 1;
    }
    else if (!equal_height && bmp1->header.height < 0) {
        const struct bmp_file_t* buf = bmp1;
        bmp1 = bmp2;
        bmp2 = buf;
    }
    for (int i = vert_loop_start; i < vert_loop_finish && diff_count < 100; i++) {
        for (unsigned int j = 0; j < (unsigned long) bmp1->header.width && diff_count < 100; j++) {
            const BYTE* const pxl1 = bmp1->bitmap + abs(i) * bytes_per_row_1 + j * bytes_per_pxl_1;
            const BYTE* const pxl2 = bmp2->bitmap + ((!equal_height) * (bmp1->header.height - 1 - 2 * abs(i)) + abs(i)) * bytes_per_row_2 + j * bytes_per_pxl_2;
            if ((bmp1->header.bpp == BPP_8BIT && bmp2->header.bpp == BPP_8BIT && !CMP_8BPP_PXLS(bmp1->palette, bmp2->palette, pxl1, pxl2)) ||
                (bmp1->header.bpp == BPP_8BIT && bmp2->header.bpp == BPP_24BIT && !CMP_MIXED_PXLS(bmp1->palette, pxl1, pxl2)) ||
                (bmp1->header.bpp == BPP_24BIT && bmp2->header.bpp == BPP_8BIT && !CMP_MIXED_PXLS(bmp2->palette, pxl2, pxl1)) ||
                (bmp1->header.bpp == BPP_24BIT && bmp2->header.bpp == BPP_24BIT && !CMP_24BPP_PXLS(pxl1, pxl2))) {
                fprintf(stderr, "(%u, %u)\n", j, abs(vert_loop_start - i));
                diff_count++;
            }
        }
    }
    if (diff_count == 0) {
        message_handle("No difference detected\n", report);
    }
}

int main(const int argc, const char* const* const argv) {
    struct comp_options_t comp_options;
    if (parse_comp_options(&comp_options, &argc, argv) != success) {
        return runtime_error;
    }
    struct bmp_file_t bmp1, bmp2;
    message_handle("Opening file 1...\n", report);
    if (read_bmp_file(&bmp1, comp_options.filename1) != success) {
        return runtime_error;
    }
    message_handle("Opening file 2...\n", report);
    if (read_bmp_file(&bmp2, comp_options.filename2) != success) {
        dealloc_bmp_container(&bmp1);
        return runtime_error;
    }
    if (bmp1.header.width != bmp2.header.width || abs(bmp1.header.height) != abs(bmp2.header.height)) {
        message_handle("Given BMP files cannot be compared (equal image resolutions required)\n", error);
        dealloc_bmp_container(&bmp1);
        dealloc_bmp_container(&bmp2);
        return runtime_error;
    }
    message_handle("Comparing given BMPs by pixels (printing first 100 different ones to stderr, if any)...\n", report);
    compare_bmp_pixels(&bmp1, &bmp2);
    dealloc_bmp_container(&bmp1);
    dealloc_bmp_container(&bmp2);
    message_handle("Done!\n", report);
    return success;
}
